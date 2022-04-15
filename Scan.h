#include <Windows.h>

char* PatternScan(char* pStart, UINT_PTR RegionSize, const char* szPattern, const char* szMask, int Len);
char* PatternScanEx(HANDLE hProc, char* pStart, UINT_PTR RegionSize, const char* szPattern, const char* szMask);

template <class T>
UINT_PTR ScanMemory(BYTE* pData, UINT_PTR RegionSize, T Val, T** pOut, int MaxCount, BYTE Alignment, BYTE* OriginalBase = nullptr)
{
	int Count = 0;
	for (ULONG i = 0; i < (RegionSize - sizeof(T)); i += Alignment, pData += Alignment)
		if (*reinterpret_cast<T*>(pData) == Val)
		{
			if (MaxCount > 0 && Count < MaxCount)
				if (OriginalBase)
					pOut[Count] = reinterpret_cast<T*>(OriginalBase + i);
				else
					pOut[Count] = reinterpret_cast<T*>(pData);
			Count++;
		}
	return Count;
}

template <class T>
UINT_PTR ScanMemoryEx(HANDLE hProc, BYTE* pStart, UINT_PTR RegionSize, T Val, T** pOut, int MaxCount, BYTE Alignment)
{
	int	Count = 0;
	DWORD Buffer = 0;
	if (!GetHandleInformation(hProc, &Buffer) || !Alignment || !RegionSize || !pOut || MaxCount < 0)
		return 0;

	BYTE* pCurrent = pStart;
	SIZE_T OldPageSize = 0;
	BYTE* Data = nullptr;


	while (pCurrent <= pStart + RegionSize - Alignment)
	{
		MEMORY_BASIC_INFORMATION MBI{ 0 };
		if (!VirtualQueryEx(hProc, pCurrent, &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
			break;

		if (MBI.State == MEM_COMMIT && !(MBI.Protect & PAGE_NOACCESS || MBI.Protect & PAGE_GUARD))
		{
			if (pCurrent + MBI.RegionSize > pStart + RegionSize)
				MBI.RegionSize = pStart + RegionSize - pCurrent;

			if (OldPageSize < MBI.RegionSize)
			{

				OldPageSize = MBI.RegionSize;
				Data = new BYTE[MBI.RegionSize];
				if (!Data)
					break;
			}

			if (ReadProcessMemory(hProc, pCurrent, Data, MBI.RegionSize, nullptr))
				Count += ScanMemory<T>(Data, MBI.RegionSize, Val, &pOut[Count], MaxCount - Count, Alignment, pCurrent);
		}
		pCurrent += MBI.RegionSize;
	}

	if (Data)
		delete[] Data;

	return Count;
}