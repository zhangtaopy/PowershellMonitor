#include "inject.h"
#ifdef _WIN64
const BYTE byShellcode[] = {
    0x9C,                                                                       // pushfd
    0x9C,                                                                       // pushfd
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x41, 0x50, 0x41, 0x51,     // store register
    0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
    0x48, 0x83, 0xEC, 0x28,                                                     // sub       rsp,0x28   x64调用约定栈平衡
    0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,                 // mov rcx,  param addr
    0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,                 // mov rax,  proxy
    0xff, 0xd0,                                                                 // call      rax
    0x48, 0x83, 0xC4, 0x28,                                                     // add       rsp,0x28
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     // restore register
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       // popfd
    0x9D,                                                                       // popfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,                                               // push stub
    0xC7, 0x44, 0x24, 0x04, 0xCC, 0xCC, 0xCC, 0xCC,
    0xC3                                                                        // ret
};
constexpr int nShellcode_stub_param = 32;
constexpr int nShellcode_stub_call = 42;
constexpr int nShellcode_stub_retaddr1 = 83;
constexpr int nShellcode_stub_retaddr2 = 91;
#else
static const BYTE byShellcode[] = {
    0x60,                                 //pushad
    0x9C,                                 //pushfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push proxy
    0x58,                                 //pop eax
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push param addr
    0xFF, 0xD0,                           //call eax
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push stub
    0xC3,                                 //ret
};
constexpr int nShellcode_stub_call = 3;
constexpr int nShellcode_stub_param = 9;
constexpr int nShellcode_stub_retaddr = 18;
#endif

typedef struct
{
    USHORT Length;
    USHORT MaximumLength;
    PVOID  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

using LdrLoaddll_Type = NTSTATUS(NTAPI*)(PWCHAR PathToFile, PULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
using VirtualProtect_Type = BOOL(WINAPI*)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
using WirteProcessMemory_Type = BOOL(WINAPI*)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);


typedef struct
{
    WCHAR szFileName[MAX_PATH];
    BYTE byOldEp[8];
    ULONG_PTR EntryPoint;
    DWORD dwOldProtect;
    HANDLE ModuleHandle;
    UNICODE_STRING ModuleFileName;
    VirtualProtect_Type pfnVirtualProtect;
    WirteProcessMemory_Type pfnWriteProcessMemory;
    LdrLoaddll_Type pfnLdrLoadDll;
} INJECT_PARAM;

#pragma optimize("", off)
BOOL WINAPI LoadDll(PVOID p)
{
    INJECT_PARAM* param = (INJECT_PARAM*)p;

    if (param->pfnVirtualProtect(PVOID(param->EntryPoint), 8, PAGE_EXECUTE_READWRITE, &param->dwOldProtect))
    {
        //param->pfnWriteProcessMemory(NULL, (PVOID)param->EntryPoint, param->byOldEp, 8, NULL);
        *(DWORD*)param->EntryPoint = *(DWORD*)param->byOldEp;
        *(DWORD*)(param->EntryPoint + 4) = *(DWORD*)&param->byOldEp[4];
        param->pfnLdrLoadDll(NULL, NULL, &param->ModuleFileName, &param->ModuleHandle);
        return TRUE;
    }

    return FALSE;
}
#pragma optimize("", on)

BOOL IsExe(HANDLE hProcess, ULONG_PTR Module)
{
    IMAGE_DOS_HEADER idh;
    ZeroMemory(&idh, sizeof(idh));

    if (!ReadProcessMemory(hProcess, (LPCVOID)Module, &idh, sizeof(idh), NULL))
    {
        return FALSE;
    }

    if (idh.e_magic != IMAGE_DOS_SIGNATURE)
    {
        return FALSE;
    }

    IMAGE_NT_HEADERS64 inh;
    ZeroMemory(&inh, sizeof(inh));

    if (!ReadProcessMemory(hProcess, (LPCVOID)(Module + idh.e_lfanew), &inh, sizeof(inh), NULL))
    {
        return FALSE;
    }

    if (inh.Signature != IMAGE_NT_SIGNATURE)
    {
        return FALSE;
    }

    if (inh.FileHeader.Characteristics & IMAGE_FILE_DLL)
    {
        return FALSE;
    }

    return TRUE;
}

ULONG_PTR FindExe(HANDLE hProcess)
{

    MEMORY_BASIC_INFORMATION mbi;

    ZeroMemory(&mbi, sizeof(mbi));

    for (ULONG_PTR pbLast = (ULONG_PTR)0x10000;;
        pbLast = (ULONG_PTR)mbi.BaseAddress + mbi.RegionSize)
    {
        if (VirtualQueryEx(hProcess, (LPCVOID)pbLast, &mbi, sizeof(mbi)) <= 0)
        {
            if (GetLastError() == ERROR_INVALID_PARAMETER)
            {
                break;
            }
            break;
        }

        if ((mbi.State != MEM_COMMIT) || (mbi.Protect & PAGE_GUARD))
        {
            continue;
        }

        if (IsExe(hProcess, pbLast))
        {
            return pbLast;
        }
    }
    return NULL;
}

ULONG_PTR GetEntryPointAddress(HANDLE hProcess)
{
    ULONG_PTR EntryPoint = 0;

    if (hProcess == NULL)
        return NULL;

    ULONG_PTR ExeAddr = FindExe(hProcess);
    if (ExeAddr == 0)
        return NULL;

    BYTE* pbPeHead = new BYTE[4096];
    memset(pbPeHead, 0, 4096);
    do
    {
        SIZE_T szReadNum = 0;
        if (FALSE == ReadProcessMemory(hProcess, (LPCVOID)ExeAddr, pbPeHead, 4096, &szReadNum))
            break;

        PIMAGE_DOS_HEADER pDosHeader = NULL;
        pDosHeader = (PIMAGE_DOS_HEADER)pbPeHead;
        if (pDosHeader->e_magic != 0x5A4D)
            break;

        if (pDosHeader->e_lfanew > 4 * 1024)
            break;

        PIMAGE_NT_HEADERS pNtHeader = NULL;
        pNtHeader = (PIMAGE_NT_HEADERS)(pbPeHead + pDosHeader->e_lfanew);

        if (pNtHeader->Signature != 0x4550)
            break;

        DWORD dwRva = pNtHeader->OptionalHeader.AddressOfEntryPoint;

        EntryPoint = ExeAddr + dwRva;
    } while (0);

    if (NULL != pbPeHead)
    {
        delete[] pbPeHead;
        pbPeHead = NULL;
    }

    return EntryPoint;
}

BOOL PrepareParam(HANDLE hProcess, LPCWSTR szDllName, ULONG_PTR EntryPoint, PVOID& RemoteParamBuffer)
{
    BOOL bRet = FALSE;

    RemoteParamBuffer = VirtualAllocEx(hProcess, 0, sizeof(INJECT_PARAM), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (RemoteParamBuffer == nullptr)
        return FALSE;

    do 
    {
        INJECT_PARAM param = { 0 };
        int nLength = sizeof(WCHAR) * wcslen(szDllName);
        param.ModuleFileName.Length = nLength;
        param.ModuleFileName.MaximumLength = nLength + 2;
        wcsncpy_s(param.szFileName, MAX_PATH, szDllName, wcslen(szDllName));
        param.ModuleFileName.Buffer = (WCHAR*)(RemoteParamBuffer);
        param.EntryPoint = EntryPoint;

        SIZE_T NumberOfBytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)EntryPoint, (LPVOID)param.byOldEp, 8, &NumberOfBytesRead)
            || NumberOfBytesRead != 8)
        {
            break;
        }

        HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
        if(hNtdll == nullptr)
            break;

        param.pfnLdrLoadDll = (LdrLoaddll_Type)GetProcAddress(hNtdll, "LdrLoadDll");
        param.pfnVirtualProtect = VirtualProtect;
        param.pfnWriteProcessMemory = WriteProcessMemory;
        if (!param.pfnLdrLoadDll
            || !param.pfnVirtualProtect
            || !param.pfnWriteProcessMemory)
        {
            break;
        }
        
        SIZE_T NumberOfBytesWrite = 0;
        bRet = WriteProcessMemory(hProcess, RemoteParamBuffer, &param, sizeof(param), &NumberOfBytesWrite);
    } while (0);

    if (!bRet)
        VirtualFreeEx(hProcess, RemoteParamBuffer, 0, MEM_RELEASE);

    return bRet;
}

BOOL CopyShellcodeToProcess(HANDLE hProcess, PVOID RemoteParamBuffer, ULONG_PTR EntryPoint, PVOID& RemoteShellcode)
{
    BOOL bRet = FALSE;
    int nLen = _countof(byShellcode);
    PVOID RemoteFunction = nullptr;
    
    LPBYTE lpShellcodeBuffer = new BYTE[nLen + 1];
    if (lpShellcodeBuffer == nullptr)
        return FALSE;

    memcpy_s(lpShellcodeBuffer, nLen + 1, byShellcode, nLen);

    do 
    {
        RemoteShellcode = VirtualAllocEx(hProcess, 0, nLen + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (RemoteShellcode == nullptr)
            break;

        RemoteFunction = VirtualAllocEx(hProcess, 0, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if(RemoteFunction == nullptr)
            break;

        *(ULONG_PTR*)&lpShellcodeBuffer[nShellcode_stub_call] = (ULONG_PTR)RemoteFunction;
        *(ULONG_PTR*)&lpShellcodeBuffer[nShellcode_stub_param] = (ULONG_PTR)RemoteParamBuffer;
#ifdef _WIN64
        ULONGLONG ullTemp = (ULONGLONG)EntryPoint;
        *(DWORD*)(&lpShellcodeBuffer[nShellcode_stub_retaddr1]) = (DWORD)ullTemp;
        ullTemp = ullTemp >> 32;
        *(DWORD*)(&lpShellcodeBuffer[nShellcode_stub_retaddr2]) = (DWORD)ullTemp;
#else
        * (ULONG_PTR*)&lpShellcodeBuffer[nShellcode_stub_retaddr] = EntryPoint;
#endif

        ///> Besides some useless code copy to target process, It works fine.
        SIZE_T NumberOfBytesWrite = 0;
        if (!WriteProcessMemory(hProcess, RemoteFunction, LoadDll, 4096, &NumberOfBytesWrite))
            break;

        bRet = WriteProcessMemory(hProcess, RemoteShellcode, lpShellcodeBuffer, nLen , &NumberOfBytesWrite);
    } while (0);

    if (!bRet && RemoteShellcode)
        VirtualFreeEx(hProcess, RemoteShellcode, 0, MEM_RELEASE);

    if (!bRet && RemoteFunction)
        VirtualFreeEx(hProcess, RemoteFunction, 0, MEM_RELEASE);

    if (lpShellcodeBuffer)
        delete[] lpShellcodeBuffer;

    return bRet;
}

BOOL ModifyEntryPoint(HANDLE hProcess, ULONG_PTR EntryPoint, PVOID RemoteShellcode)
{
    ULONG_PTR pIntermediateJmpAddress = NULL;

#ifdef _WIN64
    ULONG_PTR lpAddress = (EntryPoint & 0xFFFFFFFFF0000000ULL);
    ULONG_PTR lpEnd = lpAddress + 0x10000000;

    for (; lpAddress < lpEnd; lpAddress += 0x1000)
    {
        pIntermediateJmpAddress = (ULONG_PTR)VirtualAllocEx(hProcess, (LPVOID)lpAddress, 14, MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (GetLastError() == 1655)
            return FALSE;

        if (pIntermediateJmpAddress != NULL)
            break;
    }
#endif

    pIntermediateJmpAddress = (ULONG_PTR)VirtualAllocEx(hProcess, (LPVOID)pIntermediateJmpAddress, 14, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (pIntermediateJmpAddress == NULL)
        return FALSE;

    BYTE pStub[14] = { 0 };
    pStub[0] = 0x68;
#ifdef _WIN64
    ULONGLONG uTempAddr = (ULONGLONG)RemoteShellcode;
    *(DWORD*)&pStub[1] = uTempAddr;
    uTempAddr = uTempAddr >> 32;
    *(DWORD*)&pStub[5] = 0x042444C7;
    *(DWORD*)&pStub[9] = uTempAddr;
    pStub[13] = 0xC3;
#else
    *(DWORD*)&pStub[1] = (DWORD)RemoteShellcode;
    pStub[5] = 0xC3;
#endif
    if (!WriteProcessMemory(hProcess, (LPVOID)pIntermediateJmpAddress, pStub, 14, NULL))
        return FALSE;

    BYTE pHookCode[5] = { 0 };
    pHookCode[0] = 0xE9;
    *(DWORD*)&pHookCode[1] = (DWORD)((ULONG_PTR)pIntermediateJmpAddress - (ULONG_PTR)EntryPoint - 5);

    if (!WriteProcessMemory(hProcess, (LPVOID)EntryPoint, pHookCode, 5, NULL))
        return FALSE;

    return TRUE;
}

BOOL InjectdllInernal(HANDLE hProcess, LPCWSTR szDllName)
{
    ULONG_PTR EntryPoint = GetEntryPointAddress(hProcess);
    if (EntryPoint == 0)
        return FALSE;

    PVOID RemoteParamBuffer = nullptr;
    if (!PrepareParam(hProcess, szDllName, EntryPoint, RemoteParamBuffer))
        return FALSE;

    PVOID RemoteShellcode = nullptr;
    if (!CopyShellcodeToProcess(hProcess, RemoteParamBuffer, EntryPoint, RemoteShellcode))
        return FALSE;

    if (!ModifyEntryPoint(hProcess, EntryPoint, RemoteShellcode))
        return FALSE;

    return TRUE;
}

BOOL InjectdllByModifyEP(DWORD dwPid, LPCWSTR szDllName)
{
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 
        FALSE, 
        dwPid);

    if (hProcess == nullptr)
        return FALSE;

    BOOL bRet = InjectdllInernal(hProcess, szDllName);
    if (hProcess)
    {
        CloseHandle(hProcess);
    }
    return bRet;
}
