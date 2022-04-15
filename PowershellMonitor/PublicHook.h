#pragma once
#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <mutex>
#include "include/singleton/singleton.h"

class LdrLoadDll_Observer
{
public:
    LdrLoadDll_Observer() {};
    virtual ~LdrLoadDll_Observer() {};

    virtual BOOL ModuleLoadDispatch(
        LPCWSTR lpModuleName,
        HMODULE hModule) = 0;
};

using LdrLoadObseverList = std::vector<LdrLoadDll_Observer*>;

class PublicHook 
    : public Singleton<PublicHook>
{

public:
    PublicHook();
    ~PublicHook();

public:
    BOOL InstallLdrLoadDllHook();
    void AddOberserver(LdrLoadDll_Observer* observer);
    BOOL NotifyOberservers(PUNICODE_STRING pModuleFileName, PHANDLE pModuleHandle, PULONG pFlags);

protected:
    BOOL DoHookLdrLoadDll();
    BOOL DoNotifyObservers(LPCWSTR lpModuleName, HMODULE hModule);

private:
    static NTSTATUS __stdcall LdrLoaddll_Proxy(PWCHAR PathToFile, PULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);

private:
    LdrLoadObseverList m_LdrLoadObseverList;
    std::mutex m_ObserverListMutex;
    volatile BOOL m_bAlreadyHooked;
};

