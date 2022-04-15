#include "Scan.h"

bool __fastcall INT_ComparePattern2(char* szSource, const char* szPattern, const char* szMask)
{
	while (*szMask)
	{
		if (*szMask <= '9')
		{
			szPattern += ((*szMask) ^ 0x30);
			szSource += ((*szMask) ^ 0x30);
		}
		else
		{
			for (int i = ((*szMask) ^ 0x60); i; --i)
				if (*(szSource++) != *(szPattern++))
					return false;
		}
		++szMask;
	}

	return true;
}

bool INT_ComparePattern(char* szSource, const char* szPattern, const char* szMask)
{
	for (; *szMask; ++szSource, ++szPattern, ++szMask)
		if (*szMask == 'x' && *szSource != *szPattern)
			return false;

	return true;
}

char* INT_PatternScan(char* pData, UINT_PTR RegionSize, const char* szPattern, const char* szMask, int Len)
{
	for (UINT i = 0; i != RegionSize - Len; ++i, ++pData)
		if (INT_ComparePattern(pData, szPattern, szMask))
			return pData;

	return nullptr;
}

char* PatternScan(char* pStart, UINT_PTR RegionSize, const char* szPattern, const char* szMask, int Len)
{
	char* pCurrent = pStart;

	while (pCurrent <= pStart + RegionSize - Len)
	{
		MEMORY_BASIC_INFORMATION MBI{ 0 };
		if (!VirtualQuery(pCurrent, &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
			return nullptr;

		if (MBI.State == MEM_COMMIT && !(MBI.Protect & PAGE_NOACCESS || MBI.Protect & PAGE_GUARD))
		{
			if (pCurrent + MBI.RegionSize > pStart + RegionSize - Len)
				MBI.RegionSize = pStart + RegionSize - pCurrent + Len;

			char* Ret = INT_PatternScan(pCurrent, MBI.RegionSize, szPattern, szMask, Len);

			if (Ret)
				return Ret;
		}
		pCurrent += MBI.RegionSize;
	}

	return nullptr;
}

char* PatternScanEx(HANDLE hProc, char* pStart, UINT_PTR RegionSize, const char* szPattern, const char* szMask)
{
	DWORD Buffer = 0;
	if (!GetHandleInformation(hProc, &Buffer))
		return nullptr;

	char* pCurrent = pStart;
	auto Len = lstrlenA(szMask);

	SIZE_T BufferSize = 0x10000;
	char* Data = new char[BufferSize];

	while (pCurrent <= pStart + RegionSize - Len)
	{
		MEMORY_BASIC_INFORMATION MBI{ 0 };
		if (!VirtualQueryEx(hProc, pCurrent, &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
			return nullptr;

		if (MBI.State == MEM_COMMIT && !(MBI.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
		{
			if (BufferSize < MBI.RegionSize)
			{
				delete[] Data;
				BufferSize = MBI.RegionSize;
				Data = new char[BufferSize];
			}

			UINT_PTR Delta = pCurrent - reinterpret_cast<char*>(MBI.BaseAddress);
			MBI.RegionSize -= Delta;

			if (pCurrent + MBI.RegionSize > pStart + RegionSize - Len)
				MBI.RegionSize -= pCurrent + MBI.RegionSize - pStart - RegionSize + Len;

			if (!ReadProcessMemory(hProc, pCurrent, Data, MBI.RegionSize, nullptr))
			{
				pCurrent = pCurrent + MBI.RegionSize;

				continue;
			}

			char* Ret = INT_PatternScan(Data, MBI.RegionSize, szPattern, szMask, Len);

			if (Ret)
			{
				delete[] Data;
				return (Ret - Data + pCurrent);
			}
		}

		pCurrent = pCurrent + MBI.RegionSize;
	}

	delete[] Data;

	return nullptr;
}