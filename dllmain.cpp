#pragma region Imports

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include <mutex>
#include <string>
#include "MemoryInt.h"
#include "Draw.h"
#include "Offset.h"

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif

// Detours imports
#include "detours.h"
#pragma comment(lib, "detours.lib")

// DX11 imports
#include <d3d11.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

// Custom import 
#include "Detour.h"
#include "geom.h"
#include "Scan.h"

//ImGUI imports
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <TlHelp32.h>
#pragma endregion

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#pragma region Defines 

typedef HRESULT(__fastcall* IDXGISwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma endregion

#pragma region Main D3D11 Objects

ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
ID3D11Device* pDevice = NULL;
static WNDPROC OriginalWndProcHandler = nullptr;
HWND window = nullptr;
IDXGISwapChainPresent fnIDXGISwapChainPresent;
static IDXGISwapChain* pSwapChain = NULL;
DWORD_PTR* pDeviceContextVTable = NULL;

#pragma endregion

#pragma region Booleans

BOOL g_bInitialised = false;
bool g_PresentHooked = false;

#pragma endregion

#pragma region ZBuffering

ID3D11DepthStencilState* m_DepthStencilState;

#pragma endregion


#pragma region Entity

char* jmpbackAddy;

int ix;
EntityBase* entlist[255];
EntityBase* entityObjStart = 0x0;
bool alreadyThere = false;

__declspec() void setObjStart()
{

	__asm {
		movss xmm0, [rbx + 0x54]
		movss xmm1, [rbx + 0x58]
		movss xmm2, [rbx + 0x5C]
		mov entityObjStart, rbx
	}

	if (entityObjStart == nullptr)
	{
		goto GIVE_UP;
	}

	alreadyThere = false;

	for (ix = 0; ix < 254; ix++)
	{
		if (entlist[ix] == entityObjStart)
		{
			alreadyThere = true;
			break;
		}
	}

	if (alreadyThere)
	{
		goto GIVE_UP;
	}
	else
	{
		for (ix = 0; ix < 254; ix++)
		{
			if (entlist[ix] == 0)
			{
				entlist[ix] = entityObjStart;
				break;
			}
		}
	}

GIVE_UP:
	__asm
	{
		jmp[jmpbackAddy]
	}
}

#pragma endregion


std::string name = "Vieuxnorris";
std::string verificationBone = "NPC COM [COM ]";
EntityBase* Player;
bool alreadyherePlayer = false;
bool g_ShowESP = false;
bool g_ShowAIMBOT = false;
bool g_ShowLINE = false;

uintptr_t modulebase;

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(window, &mPos);
	ImGui::GetIO().MousePos.x = mPos.x;
	ImGui::GetIO().MousePos.y = mPos.y;
	
	

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

HRESULT GetDeviceAndCtxFromSwapchain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
	HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)ppDevice);

	if (SUCCEEDED(ret))
		(*ppDevice)->GetImmediateContext(ppContext);

	return ret;
}

EntityBase* GetBestTarget(EntityBase* localPlayer)
{
	float oldDistance = FLT_MAX;
	float newDistance_angle = 0;
	float newDistance = 0;
	float coefficient = 0;
	EntityBase* target = nullptr;

	for (auto ents : entlist)
	{
		if (ents != NULL && ents->EntityPtr->Name != localPlayer->EntityPtr->Name && ents->Bone1ptr != 0)
		{
			if (verificationBone.compare(ents->Bone1ptr->Bone2ptr->Bone->ValidationBone) == 0)
			{
				vec_3 angleTo = CalcAngle(localPlayer->Bone1ptr->Bone2ptr->Bone->HeadHumanoid, ents->Bone1ptr->Bone2ptr->Bone->HeadHumanoid);
				newDistance_angle = Distance(vec_3(RadianToDegree(localPlayer->Yaw), RadianToDegree(localPlayer->Pitch), 0), angleTo);
				newDistance = std::sqrtf(std::pow(localPlayer->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.x - ents->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.x, 2) + std::pow(localPlayer->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.y - ents->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.y, 2) + std::pow(localPlayer->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.z - ents->Bone1ptr->Bone2ptr->Bone->HeadHumanoid.z, 2));
				coefficient = newDistance * 0.3 + newDistance_angle * 0.7;
				if (coefficient < oldDistance)
				{
					oldDistance = coefficient;
					target = ents;
				}
			}
		}
	}
	return target;
}

HRESULT __fastcall Present(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
{
	if (!g_bInitialised) {
		g_PresentHooked = true;
		std::cout << "\t[+] Present Hook called by first time" << std::endl;
		if (FAILED(GetDeviceAndCtxFromSwapchain(pChain, &pDevice, &pContext)))
			return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
		pSwapChain = pChain;
		DXGI_SWAP_CHAIN_DESC sd;
		pChain->GetDesc(&sd);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		window = sd.OutputWindow;

		//Set OriginalWndProcHandler to the Address of the Original WndProc function
		OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);

		// Disabling Z-Buffering
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


		pDevice->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(pDevice, pContext);
		ImGui::GetIO().ImeWindowHandle = window;

		ID3D11Texture2D* pBackBuffer;

		pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
		pBackBuffer->Release();

		g_bInitialised = true;
	}

	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_NUMPAD1) & 1)
	{
		g_ShowESP = !g_ShowESP;
		if (g_ShowESP)
		{
			std::cout << "[+] ESP ENABLE" << std::endl;
		}
		else
		{
			std::cout << "[+] ESP DISABLE" << std::endl;
		}
	}

	if (GetAsyncKeyState(VK_NUMPAD2) & 1)
	{
		g_ShowAIMBOT = !g_ShowAIMBOT;
		if (g_ShowAIMBOT)
		{
			std::cout << "[+] AIMBOT ENABLE" << std::endl;
		}
		else
		{
			std::cout << "[+] AIMBOT DISABLE" << std::endl;
		}
	}

	if (GetAsyncKeyState(VK_NUMPAD3) & 1)
	{
		g_ShowLINE = !g_ShowLINE;
		if (g_ShowLINE)
		{
			std::cout << "[+] ESP DRAW_line ENABLE" << std::endl;
		}
		else
		{
			std::cout << "[+] ESP DRAW_line DISABLE" << std::endl;
		}
	}

	if (g_ShowAIMBOT)
	{
		for (auto ents : entlist)
		{
			if (ents != 0)
			{
				if (name.compare(ents->EntityPtr->Name) == 0)
				{
					Player = ents;
				}
				else if (Player != 0)
				{
					EntityBase* Target = GetBestTarget(Player);
					if (Target != NULL && verificationBone.compare(Target->Bone1ptr->Bone2ptr->Bone->ValidationBone) == 0)
					{
						vec_3 angles = CalcAngle(Player->Bone1ptr->Bone2ptr->Bone->HeadHumanoid, Target->Bone1ptr->Bone2ptr->Bone->HeadHumanoid);

						Player->Pitch = DegreeToRadian(angles.y);
						Player->Yaw = DegreeToRadian(angles.x);
					}
				}
			}
		}
	}

	if (g_ShowESP)
	{
		for (int i = 0; i < 254; i++)
		{
			if (entlist[i] != NULL)
			{
				std::string IsValid;
				vec_2 Screen;
				vec_2 Screen_player;
				float Matrix[16];
				if (name.compare(entlist[i]->EntityPtr->Name) == 0)
				{
					Player = entlist[i];
				}
				else if (Player != 0 && entlist[i]->Verification1ptr != NULL && entlist[i]->Verification2ptr != NULL && entlist[i]->Verification3ptr != NULL)
				{
					float distance;
					memcpy(&Matrix, (PBYTE*)(modulebase + 0x2FE8840), sizeof(Matrix));
					if (WorldToScreenDX(entlist[i]->Coordonnee, Screen, Matrix, 1920, 1080))
					{
						distance = std::sqrtf(std::pow(Player->Coordonnee.x - entlist[i]->Coordonnee.x, 2) + std::pow(Player->Coordonnee.y - entlist[i]->Coordonnee.y, 2) + std::pow(Player->Coordonnee.z - entlist[i]->Coordonnee.z, 2));
						if (distance >= 0.f && distance <= 3000.f)
						{
							if (!g_ShowLINE)
							{
								std::string distance_string = std::string(entlist[i]->EntityPtr->Name);
								distance_string.append(" [" + std::to_string(int(distance) / 100) + "m]");
								DrawNewText(Screen.x, Screen.y, 255.0, 0.0, 0.0, 255.0, distance_string.c_str());
							}
							else
							{
								std::string distance_string = std::string(entlist[i]->EntityPtr->Name);
								distance_string.append(" [" + std::to_string(int(distance) / 100) + "m]");
								DrawNewText(Screen.x, Screen.y, 255.0, 0.0, 0.0, 255.0, distance_string.c_str());
								if (!WorldToScreenDX(Player->Coordonnee, Screen_player, Matrix, 1920, 1080)) continue;
								DrawLine(Screen_player.x, Screen_player.y, Screen.x, Screen.y, 255.0, 0.0, 0.0, 255.0, 1);
							}
						}
						else
						{
							entlist[i] = nullptr;
						}
					}
				}
				else
				{
					entlist[i] = nullptr;
				}
			}
			else
			{
				entlist[i] = nullptr;
			}
		}
	}
	ImGui::EndFrame();

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
}

void detourDirectXPresent()
{
	std::cout << "[+] Calling fnIDXGISwapChainPresent Detour" << std::endl;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	// Detours the original fnIDXGISwapChainPresent with our Present
	DetourAttach(&(LPVOID&)fnIDXGISwapChainPresent, (PBYTE)Present);
	DetourTransactionCommit();
}

void retrieveValues()
{
	DWORD_PTR hDxgi = (DWORD_PTR)GetModuleHandle(L"dxgi.dll");
#if defined(ENV64BIT)
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)((DWORD_PTR)hDxgi + 0x5070);
#elif defined (ENV32BIT)
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)((DWORD_PTR)hDxgi + 0x10230);
#endif
	std::cout << "[+] Present Addr: " << std::hex << fnIDXGISwapChainPresent << std::endl;
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }

void GetPresent()
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 2;
	sd.BufferDesc.Height = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT numFeatureLevelsRequested = 1;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	HRESULT hr;
	IDXGISwapChain* swapchain = 0;
	ID3D11Device* dev = 0;
	ID3D11DeviceContext* devcon = 0;
	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&FeatureLevelsRequested,
		numFeatureLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&swapchain,
		&dev,
		&FeatureLevelsSupported,
		&devcon)))
	{
		std::cout << "[-] Failed to hook Present with VT method." << std::endl;
		return;
	}
	DWORD_PTR* pSwapChainVtable = NULL;
	pSwapChainVtable = (DWORD_PTR*)swapchain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)(DWORD_PTR)pSwapChainVtable[8];
	g_PresentHooked = true;
	std::cout << "[+] Present Addr:" << fnIDXGISwapChainPresent << std::endl;
	Sleep(2000);
}

void ConsoleSetup()
{
	if (!AllocConsole())
		MessageBox(NULL, L"The console window was not created", NULL, MB_ICONEXCLAMATION);
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
	SetConsoleTitle(L"[+] Hooking DirectX 11");
}

void printValues()
{
	std::cout << "[+] ID3D11DeviceContext Addr: " << std::hex << pContext << std::endl;
	std::cout << "[+] ID3D11Device Addr: " << std::hex << pDevice << std::endl;
	std::cout << "[+] ID3D11RenderTargetView Addr: " << std::hex << mainRenderTargetView << std::endl;
	std::cout << "[+] IDXGISwapChain Addr: " << std::hex << pSwapChain << std::endl;
}

int WINAPI main()
{
	ConsoleSetup();

	GetPresent();

	//If GetPresent failed we have this backup method to get Present Address
	if (!g_PresentHooked) {
		retrieveValues();
	}

	 //After this call, Present should be hooked and controlled by me.
	detourDirectXPresent();
	while (!g_bInitialised) {
		Sleep(1000);
	}

	printValues();
	std::cout << "[+] pDeviceContextVTable0 Addr: " << std::hex << pContext << std::endl;
	pDeviceContextVTable = (DWORD_PTR*)pContext;
	std::cout << "[+] pDeviceContextVTable1 Addr: " << std::hex << pDeviceContextVTable << std::endl;
	pDeviceContextVTable = (DWORD_PTR*)pDeviceContextVTable[0];
	std::cout << "[+] pDeviceContextVTable2 Addr: " << std::hex << pDeviceContextVTable << std::endl;

	std::cout << "[+] pDeviceContextVTable Addr: " << std::hex << pDeviceContextVTable << std::endl;
	Sleep(4000);

	std::cout << "Internal SkyrimSE x64" << std::endl;

	modulebase = (uintptr_t)GetModuleHandle(L"SkyrimSE.exe");

	std::cout << "[+] Scan en cours ..." << std::endl;
	auto szmask = "xxxxxxxxxxxxxxxxxxxxxxx";
	char* address = PatternScan(0, 0x7FFFFFFFFFFFFFFF, "\xF3\x0F\x10\x43\x54\xF3\x0F\x10\x4B\x58\xF3\x0F\x10\x53\x5C\xC7\x44\x24\x3C\x00\x00\x00\x00", szmask, strlen(szmask));
	std::cout << "[+] PatternScan Addr: " << std::hex << address << std::endl;
	std::cout << "[+] Scan terminé !" << std::endl;
	Detour hook;

	jmpbackAddy = address + 15;
	hook.CreateDetour((void*)address, (void*)setObjStart, 15, true);

	while (true)
	{
	}
	return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: {
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main, NULL, NULL, NULL);
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}