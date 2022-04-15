#include "pch.h"
#include "PublicHook.h"
#include <Shlwapi.h>
#include "detours4/detours.h"
#include "utils.h"

using LdrLoaddll_Type = NTSTATUS(NTAPI*)(PWCHAR PathToFile, PULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
LdrLoaddll_Type real_LdrLoadDll_ = nullptr;

PublicHook::PublicHook()
{
    m_bAlreadyHooked = FALSE;
}

PublicHook::~PublicHook()
{

}

BOOL PublicHook::InstallLdrLoadDllHook()
{
    if (!InterlockedCompareExchange((volatile long*)&m_bAlreadyHooked, TRUE, FALSE))
    {
        return DoHookLdrLoadDll();
    }

    return TRUE;
}

void PublicHook::AddOberserver(LdrLoadDll_Observer* observer)
{
    std::lock_guard<std::mutex> MutexGuard(m_ObserverListMutex);
    m_LdrLoadObseverList.emplace_back(observer);
    return;
}

BOOL PublicHook::NotifyOberservers(
    PUNICODE_STRING pModuleFileName,
    PHANDLE pModuleHandle,
    PULONG pFlags)
{
    if (!pModuleHandle || !(*pModuleHandle))
        return FALSE;

    std::wstring strModuleName;
    if (!Utils::GetLdrLoadModuleName(pModuleFileName, pFlags, strModuleName))
        return FALSE;

    LPCWSTR lpModuleName = PathFindFileName(strModuleName.c_str());
    if (lpModuleName == nullptr)
        return FALSE;

    return DoNotifyObservers(lpModuleName, static_cast<HMODULE>(*pModuleHandle));
}

BOOL PublicHook::DoHookLdrLoadDll()
{
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (hNtdll == nullptr)
        return FALSE;

    real_LdrLoadDll_ = (LdrLoaddll_Type)GetProcAddress(hNtdll, "LdrLoadDll");
    if (real_LdrLoadDll_ == nullptr)
        return FALSE;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)real_LdrLoadDll_, LdrLoaddll_Proxy);
    LONG error = DetourTransactionCommit();

    return error == NO_ERROR;
}

BOOL PublicHook::DoNotifyObservers(LPCWSTR lpModuleName, HMODULE hModule)
{
    std::lock_guard<std::mutex> MutexGuard(m_ObserverListMutex);
    for (auto observer : m_LdrLoadObseverList)
    {
        observer->ModuleLoadDispatch(lpModuleName, hModule);
    }

    return TRUE;
}

NTSTATUS __stdcall PublicHook::LdrLoaddll_Proxy(
    PWCHAR PathToFile, 
    PULONG Flags, 
    PUNICODE_STRING ModuleFileName, 
    PHANDLE ModuleHandle)
{
    NTSTATUS status = real_LdrLoadDll_(PathToFile, Flags, ModuleFileName, ModuleHandle);
    PublicHook::getInstancePtr()->NotifyOberservers(ModuleFileName, ModuleHandle, Flags);
    return status;
}
