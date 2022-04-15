#include "pch.h"
#include "PowerShellMonitor.h"
#include <string>
#include <shlwapi.h>
#include <Psapi.h>
#include <algorithm>
#include "utils.h"
#include "include/3rdparty/detours4/detours.h"

compileMethod_def real_compileMethod = nullptr;

#ifdef _WIN64
static const BYTE DynamicInsRun_stub[] = {
    0x9C,                                                                       //pushfd
    0x9C,                                                                       //pushfd
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x41, 0x50, 0x41, 0x51,     //±£´æ¸÷¼Ä´æÆ÷
    0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
    0x48, 0x83, 0xEC, 0x28,                                                     //sub esp,0x28   x64µ÷ÓÃÔ¼¶¨Õ»Æ½ºâ
    0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,                 //mov rax, proxy
    0xFF, 0xD0,                                                                 //call rax
    0x48, 0x83, 0xC4, 0x28,                                                     //add esp,0x28
    0x85, 0xC0,                                                                 //test eax,eax
    0x75, 0x28,                                                                 //jnz
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x9D,                                                                       //popfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,                                               //mov rax, stub
    0xC7, 0x44, 0x24, 0x04, 0xCC, 0xCC, 0xCC, 0xCC,
    0xC3,                                                                       //ret
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x9D,                                                                       //popfd
    0xC3                                                                        //ret
};
constexpr int DynamicInsRun_stub_push1 = 32;
constexpr int DynamicInsRun_stub_push2 = 77;
constexpr int DynamicInsRun_stub_push3 = 85;

static const BYTE DynamicClassInvoke_stub[] = {
    0x9C,                                                                       //pushfd
    0x9C,                                                                       //pushfd
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x41, 0x50, 0x41, 0x51,     //±£´æ¸÷¼Ä´æÆ÷
    0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
    0x48, 0x83, 0xEC, 0x28,                                                     //sub esp,0x28   x64µ÷ÓÃÔ¼¶¨Õ»Æ½ºâ
    0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,                 //mov rax, proxy
    0xFF, 0xD0,                                                                 //call rax
    0x48, 0x83, 0xC4, 0x28,                                                     //add esp,0x28
    0x85, 0xC0,                                                                 //test eax,eax
    0x75, 0x28,                                                                 //jnz
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x9D,                                                                       //popfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,                                               //mov rax, stub
    0xC7, 0x44, 0x24, 0x04, 0xCC, 0xCC, 0xCC, 0xCC,
    0xC3,                                                                       //ret
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x9D,                                                                       //popfd
    0x48, 0x33, 0xC0,                                                           //xor rax,rax
    0xC3                                                                        //ret
};
constexpr int DynamicClassInvoke_stub_push1 = 32;
constexpr int DynamicClassInvoke_stub_push2 = 77;
constexpr int DynamicClassInvoke_stub_push3 = 85;

static const BYTE InvokeMethod_stub[] = {
    0x52,                                                                       //push rdx
    0x49, 0x8B, 0xD0,                                                           //mov rdx, r8
    0x9C,                                                                       //pushfd
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x41, 0x50, 0x41, 0x51,     //±£´æ¸÷¼Ä´æÆ÷
    0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
    0x48, 0x83, 0xEC, 0x28,                                                     //sub esp,0x28   x64µ÷ÓÃÔ¼¶¨Õ»Æ½ºâ
    0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,                 //mov rax, proxy
    0xFF, 0xD0,                                                                 //call rax
    0x48, 0x83, 0xC4, 0x28,                                                     //add esp,0x28
    0x85, 0xC0,                                                                 //test eax,eax
    0x75, 0x28,                                                                 //jnz
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x5A,                                                                       //pop rdx
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,                                               //push stub
    0xC7, 0x44, 0x24, 0x04, 0xCC, 0xCC, 0xCC, 0xCC,
    0xC3,                                                                       //ret
    0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A,     //»Ö¸´¸÷¼Ä´æÆ÷
    0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58,
    0x9D,                                                                       //popfd
    0x5A,                                                                       //pop rdx
    0x48, 0x33, 0xC0,                                                           //xor rax,rax
    0xC3                                                                        //ret
};

constexpr int InvokeMethod_stub_push1 = 35;
constexpr int InvokeMethod_stub_push2 = 80;
constexpr int InvokeMethod_stub_push3 = 88;
#else
static const BYTE DynamicInsRun_stub[] = {
    0x60,                                 //pushad
    0x9C,                                 //pushfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push proxy
    0x58,                                 //pop eax
    0xFF, 0xD0,                           //call eax
    0x85, 0xC0,                           //test eax,eax
    0x75, 0x08,                           //jnz
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push stub
    0xC3,                                 //ret
    0x9D,                                 //popfd
    0x61,                                 //popad
    0xC3                                  //ret
};
constexpr int DynamicInsRun_stub_push1 = 3;
constexpr int DynamicInsRun_stub_push2 = 17;

static const BYTE DynamicClassInvoke_stub[] = {
    0x60,                                 //pushad
    0x9C,                                 //pushfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push proxy
    0x58,                                 //pop eax
    0xFF, 0xD0,                           //call eax
    0x85, 0xC0,                           //test eax,eax
    0x75, 0x08,                           //jnz
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push stub
    0xC3,                                 //ret
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x33, 0xC0,                           //xor eax,eax
    0xC3                                  //ret
};
constexpr int DynamicClassInvoke_stub_push1 = 3;
constexpr int DynamicClassInvoke_stub_push2 = 17;

static const BYTE InvokeMethod_stub[] = {
    0x52,                                 //push edx
    0x8B, 0x54, 0x24, 0x0C,               //mov edx, dword ptr [esp+8]
    0x60,                                 //pushad
    0x9C,                                 //pushfd
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push proxy
    0x58,                                 //pop eax
    0xFF, 0xD0,                           //call eax
    0x85, 0xC0,                           //test eax,eax
    0x75, 0x09,                           //jnz
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x5A,                                 //pop edx
    0x68, 0xCC, 0xCC, 0xCC, 0xCC,         //push stub
    0xC3,                                 //ret
    0x9D,                                 //popfd
    0x61,                                 //popad
    0x5A,                                 //pop edx
    0x33, 0xC0,                           //xor eax,eax
    0xC3                                  //ret
};

constexpr int InvokeMethod_stub_push1 = 8;
constexpr int InvokeMethod_stub_push2 = 23;
#endif

PowerShellMonitor::PowerShellMonitor()
{
    m_powershellhelper = nullptr;
    m_pLuaEngine = nullptr;
}

PowerShellMonitor::~PowerShellMonitor()
{
    m_pscmdhandler.Uninitialize();
}

BOOL PowerShellMonitor::PrePare(PREPARE_DATA& data)
{
    return SetLuaEngine(data.pLuaEngine);
}

BOOL PowerShellMonitor::DoWork()
{
    m_powershellinfo.InitPSVersionInfo();
    powershellhelperselector selector;
    m_powershellhelper = selector.GetHelper(m_powershellinfo.GetPSVersion());
    if (!m_powershellhelper)
    {
        return FALSE;
    }

    m_pscmdhandler.Initialize(m_pLuaEngine);

    if (PublicHook::getInstancePtr()->InstallLdrLoadDllHook())
    {
        PublicHook::getInstancePtr()->AddOberserver(this);
    }
    return TRUE;
}

BOOL PowerShellMonitor::PreCheck()
{

    if (!Utils::IsInPowershellProcess())
    {
        return FALSE;
    }

    m_powershellinfo.InitPSVersionInfo();
    if (!CheckPowershellVersion())
    {
        return FALSE;
    }

    return TRUE;
}

BOOL PowerShellMonitor::SetLuaEngine(LuaenginePtr pLuaEngine)
{
    m_pLuaEngine = pLuaEngine;
    return TRUE;
}

BOOL PowerShellMonitor::ModuleLoadDispatch(LPCWSTR lpModuleName, HMODULE hModule)
{
    if (lpModuleName == nullptr)
        return FALSE;

    if (_wcsicmp(lpModuleName, L"clrjit.dll") == 0
        || _wcsicmp(lpModuleName, L"mscorjit.dll") == 0)
    {
        HookCompileMethod(hModule);
    }
    else if (_wcsicmp(lpModuleName, L"System.Management.Automation.ni.dll") == 0)
    {
        HookSystemManagementniDll(hModule);
    }

    return TRUE;
}

BOOL PowerShellMonitor::CompileMethodHandler(
    ICorJitInfo* comp, 
    CORINFO_METHOD_INFO* info, 
    unsigned flags, 
    BYTE** nativeEntry, 
    ULONG* nativeSizeOfCode)
{
    std::string strMethodName;
    std::string strClassName;
    if (m_powershellhelper == nullptr)
        return FALSE;

    if (!m_powershellhelper->GetMethodName(comp, info, strMethodName, strClassName))
        return FALSE;

    if (m_powershellhelper->CheckNeedHookDynamicIns(strMethodName, strClassName))
    {
        HookDynamicInsRun(nativeEntry, nativeSizeOfCode);
    }

    if (m_powershellhelper->CheckNeedHookInvokeMethod(strMethodName, strClassName))
    {
        HookInvokeMethod(nativeEntry, nativeSizeOfCode);
    }

    if (m_powershellhelper->CheckNeedHookDynamicClassInvoke(strMethodName, strClassName))
    {
        HookDynamicClassInvoke(nativeEntry, nativeSizeOfCode);
    }

    if (m_powershellhelper->CheckNeedHookCommandProcessor(strMethodName, strClassName))
    {
        HookCommandProcessorProcessRecord(nativeEntry, nativeSizeOfCode);
    }

    return TRUE;
}

BOOL PowerShellMonitor::HookCompileMethod(HMODULE hModule)
{
    if (hModule == nullptr)
        return FALSE;

    getJit_Type pfngetJit = (getJit_Type)GetProcAddress(hModule, "getJit");

    if (pfngetJit == nullptr)
        return FALSE;

    JIT* pJit = (JIT*) *((ULONG_PTR *)pfngetJit());

    if (pJit == nullptr)
        return FALSE;

    DWORD OldProtect;
    if(VirtualProtect(pJit, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE, &OldProtect))
    { 
        real_compileMethod = pJit->compileMethod;
        pJit->compileMethod = compileMethod_Proxy;
        VirtualProtect(pJit, sizeof(ULONG_PTR), OldProtect, &OldProtect);
        return TRUE;
    }
    return FALSE;
}

BOOL PowerShellMonitor::HookDynamicMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode, ULONG_PTR proxy)
{
    BYTE* entry = *nativeEntry;
    ULONG sizeofcode = *nativeSizeOfCode;
    LONG error = NO_ERROR;

    if (sizeofcode == 0 || IsBadReadPtr(entry, 5))
        return FALSE;
    do
    {
        LPVOID lpAddress = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _countof(DynamicInsRun_stub));
        if (lpAddress == nullptr)
            break;
        DWORD dwOldProtect = 0;
        if (VirtualProtect(lpAddress, _countof(DynamicInsRun_stub), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            memcpy(lpAddress, DynamicInsRun_stub, _countof(DynamicInsRun_stub));
            BYTE* pStub = (BYTE*)lpAddress;
            *(ULONG_PTR*)(pStub + DynamicInsRun_stub_push1) = proxy;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)entry, lpAddress);
            error = DetourTransactionCommit();
#ifdef _WIN64
            ULONGLONG ullTemp = (ULONGLONG)entry;
            * (DWORD*)(pStub + DynamicInsRun_stub_push2) = (DWORD)ullTemp;
            ullTemp = ullTemp >> 32;
            *(DWORD*)(pStub + DynamicInsRun_stub_push3) = (DWORD)ullTemp;
#else
            * (ULONG_PTR*)(pStub + DynamicInsRun_stub_push2) = (ULONG_PTR)entry;
#endif
        }
        else
        {
            HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, lpAddress);
        }

    } while (FALSE);

    return error == NO_ERROR;
}

BOOL PowerShellMonitor::HookDynamicInsRun(BYTE** nativeEntry, ULONG* nativeSizeOfCode)
{
    return HookDynamicMethod(nativeEntry, nativeSizeOfCode, (ULONG_PTR)DynamicInsRun_Proxy);
}

BOOL PowerShellMonitor::HookCommandProcessorProcessRecord(BYTE** nativeEntry, ULONG* nativeSizeOfCode)
{
    return HookDynamicMethod(nativeEntry, nativeSizeOfCode, (ULONG_PTR)CommandProcessorProcessRecord_Proxy);
}

BOOL PowerShellMonitor::HookDynamicClassInvoke(BYTE** nativeEntry, ULONG* nativeSizeOfCode)
{
    BYTE* entry = *nativeEntry;
    ULONG sizeofcode = *nativeSizeOfCode;
    LONG error = NO_ERROR;

    if (sizeofcode == 0 || IsBadReadPtr(entry, 5))
        return FALSE;
    do
    {
        LPVOID lpAddress = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _countof(DynamicClassInvoke_stub));
        if (lpAddress == nullptr)
            break;
        DWORD dwOldProtect = 0;
        if (VirtualProtect(lpAddress, _countof(DynamicClassInvoke_stub), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            memcpy(lpAddress, DynamicClassInvoke_stub, _countof(DynamicClassInvoke_stub));
            BYTE* pStub = (BYTE*)lpAddress;
            *(ULONG_PTR*)(pStub + DynamicClassInvoke_stub_push1) = (ULONG_PTR)DynamicClassInvoke_Proxy;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)entry, lpAddress);
            error = DetourTransactionCommit();
#ifdef _WIN64
            ULONGLONG ullTemp = (ULONGLONG)entry;
            *(DWORD*)(pStub + DynamicClassInvoke_stub_push2) = (DWORD)ullTemp;
            ullTemp = ullTemp >> 32;
            *(DWORD*)(pStub + DynamicClassInvoke_stub_push3) = (DWORD)ullTemp;
#else
            * (ULONG_PTR*)(pStub + DynamicClassInvoke_stub_push2) = (ULONG_PTR)entry;
#endif
        }
        else
        {
            HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, lpAddress);
        }

    } while (FALSE);

    return error == NO_ERROR;
}

BOOL PowerShellMonitor::HookInvokeMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode)
{
    BYTE* entry = *nativeEntry;
    ULONG sizeofcode = *nativeSizeOfCode;
    LONG error = NO_ERROR;

    if (sizeofcode == 0 || IsBadReadPtr(entry, 5))
        return FALSE;
    do
    {
        LPVOID lpAddress = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _countof(InvokeMethod_stub));
        if (lpAddress == nullptr)
            break;
        DWORD dwOldProtect = 0;
        if (VirtualProtect(lpAddress, _countof(InvokeMethod_stub), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            memcpy(lpAddress, InvokeMethod_stub, _countof(InvokeMethod_stub));
            BYTE* pStub = (BYTE*)lpAddress;
            *(ULONG_PTR*)(pStub + InvokeMethod_stub_push1) = (ULONG_PTR)InvokeMethod_Proxy;
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)entry, lpAddress);
            error = DetourTransactionCommit();
#ifdef _WIN64
            ULONGLONG ullTemp = (ULONGLONG)entry;
            *(DWORD*)(pStub + InvokeMethod_stub_push2) = (DWORD)ullTemp;
            ullTemp = ullTemp >> 32;
            *(DWORD*)(pStub + InvokeMethod_stub_push3) = (DWORD)ullTemp;
#else
            *(ULONG_PTR*)(pStub + InvokeMethod_stub_push2) = (ULONG_PTR)entry;
#endif
        }
        else
        {
            HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, lpAddress);
        }

    } while (FALSE);

    return error == NO_ERROR;
}

BOOL PowerShellMonitor::HookSystemManagementniDll(HMODULE hModule)
{
    MODULEINFO mi = {0};
    GetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(MODULEINFO));
    HookAddrs hookAddrs;
    if (m_powershellhelper->SearchingForHookAddress(reinterpret_cast<PBYTE>(hModule), 
        static_cast<int>(mi.SizeOfImage), hookAddrs))
    {
        ULONG sizeofcode = 50;
        for (auto addr : hookAddrs)
        {
            if (m_powershellhelper->CheckNeedHookDynamicIns())
            {
                HookDynamicInsRun(&addr, &sizeofcode);
            }
            else if (m_powershellhelper->CheckNeedHookInvokeMethod())
            {
                HookInvokeMethod(&addr, &sizeofcode);
            }
        }
    }

    hookAddrs.clear();
    if(m_powershellhelper->SearchingForCommandProcessorAddr(reinterpret_cast<PBYTE>(hModule), 
        static_cast<int>(mi.SizeOfImage), hookAddrs))
    {
        ULONG sizeofcode = 50;
        for (auto addr : hookAddrs)
        {
            HookCommandProcessorProcessRecord(&addr, &sizeofcode);
        }
    }

    return TRUE;
}

inline void PowerShellMonitor::DealWithRegWrite(std::wstring& strFunctionName, void* arg1, void* arg2)
{
    if (_wcsicmp(strFunctionName.c_str(), L"RegWrite") == 0)
    {
        ArgumentVector vecArgs;
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING, ArgType::TYPE_IGNORE, ArgType::TYPE_STRING };
        if (m_powershellhelper->GetFunctionArguments(arg1, arg2, vecArgs, vecTypes))
        {
            std::wstring strRegType = std::get<std::wstring>(vecArgs[2]);
            if (_wcsicmp(strRegType.c_str(), L"REG_DWORD") == 0)
            {
                strFunctionName += L"Dword";
            }
            else if (_wcsicmp(strRegType.c_str(), L"REG_BINARY") == 0)
            {
                strFunctionName += L"Binary";
            }
            else if (_wcsicmp(strRegType.c_str(), L"REG_SZ") == 0)
            {
                strFunctionName += L"SZ";
            }
        }
    }
}

BOOL PowerShellMonitor::CheckPowershellVersion()
{
    if (m_powershellinfo.GetPSVersion() == PowershellVersion::PSVER2
        || m_powershellinfo.GetPSVersion() == PowershellVersion::PSVER5)
        return TRUE;

    return FALSE;
}

int PowerShellMonitor::DynamicInsRunHandler(DynamicIns* Dynamicins, InterpretedFrame* interpretedFrame)
{
    if (!m_powershellhelper->IsParmaterValid(Dynamicins, interpretedFrame))
    {
        return 0;
    }

    std::wstring strFunctionName;
    if (!m_powershellhelper->GetFunctionName(Dynamicins, strFunctionName))
    {
        return 0;
    }

    int nArgCount = m_powershellhelper->GetArgumentCount(Dynamicins);

    if (nArgCount == 3)
    {
        DealWithRegWrite(strFunctionName, Dynamicins, interpretedFrame);
    }

    if (_wcsicmp(strFunctionName.c_str(), L"Invoke") == 0)
    {
        return m_pscmdhandler.NativeMethodHandler(m_powershellhelper, nArgCount, Dynamicins, interpretedFrame);
    }
    else if(m_pscmdhandler.IsCmdNeedHandle(strFunctionName))
    {
        return m_pscmdhandler.NormalMethodHandler(m_powershellhelper, nArgCount, strFunctionName, Dynamicins, interpretedFrame);
    }

    return 0;
}

int PowerShellMonitor::InvokeMethodHandler(MethodCallNode* CallNode, ArgumentsArray* arguments)
{
    if (!m_powershellhelper->IsParmaterValid(CallNode, arguments))
    {
        return 0;
    }

    std::wstring strFunctionName;
    if (!m_powershellhelper->GetFunctionName(CallNode, strFunctionName))
    {
        return 0;
    }

    int nArgCount = m_powershellhelper->GetArgumentCount(arguments);

    if (nArgCount == 3)
    {
        DealWithRegWrite(strFunctionName, CallNode, arguments);
    }

    if (m_pscmdhandler.IsCmdNeedHandle(strFunctionName))
    {
        return m_pscmdhandler.NormalMethodHandler(m_powershellhelper, nArgCount, strFunctionName, CallNode, arguments);
    }

    return 0;
}

int PowerShellMonitor::DynamicClassInvokeHandler(MethodInfo* nativeMethod, ArgumentsArray* arguments)
{
    int nArgCount = m_powershellhelper->GetArgumentCount(arguments);
    return m_pscmdhandler.NativeMethodHandler(m_powershellhelper, nArgCount, nativeMethod, arguments);
    
}

int PowerShellMonitor::CommandProcessorProcessRecordHandler(void* CmdProcessor, void* empty)
{
    std::wstring strCommandName;
    if (!m_powershellhelper->GetCommandName(CmdProcessor, strCommandName))
        return FALSE;

    if (m_pscmdhandler.IsCommandNeedHanle(strCommandName))
    {
        return m_pscmdhandler.CommandHandler(m_powershellhelper, strCommandName, CmdProcessor);
    }

    return 0;
}

int __stdcall PowerShellMonitor::compileMethod_Proxy(
    ULONG_PTR classthis, 
    ICorJitInfo* comp, 
    CORINFO_METHOD_INFO* info, 
    unsigned flags, 
    BYTE** nativeEntry, 
    ULONG* nativeSizeOfCode)
{
    int nRet = real_compileMethod(classthis, comp, info, flags, nativeEntry, nativeSizeOfCode);

    PowerShellMonitor::getInstancePtr()->CompileMethodHandler(comp, info, flags, nativeEntry, nativeSizeOfCode);

    return nRet;
}

int __fastcall PowerShellMonitor::DynamicInsRun_Proxy(DynamicIns* Dynamicins, InterpretedFrame* interpretedFrame)
{
    return PowerShellMonitor::getInstancePtr()->DynamicInsRunHandler(Dynamicins, interpretedFrame);
}

int __fastcall PowerShellMonitor::InvokeMethod_Proxy(MethodCallNode* CallNode, ArgumentsArray* arguments)
{
    return PowerShellMonitor::getInstancePtr()->InvokeMethodHandler(CallNode, arguments);
}

int __fastcall PowerShellMonitor::DynamicClassInvoke_Proxy(MethodInfo* nativeMethod, ArgumentsArray* arguments)
{
    return PowerShellMonitor::getInstancePtr()->DynamicClassInvokeHandler(nativeMethod, arguments);
}

int __fastcall PowerShellMonitor::CommandProcessorProcessRecord_Proxy(void* CmdProcessor)
{
    return PowerShellMonitor::getInstancePtr()->CommandProcessorProcessRecordHandler(CmdProcessor, nullptr);
}
