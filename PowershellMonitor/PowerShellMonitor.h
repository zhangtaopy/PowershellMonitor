#pragma once
#include <list>
#include <memory>
#include "PowerShellDefine.h"
#include "ProtectionBase.h"
#include "powershellinfo.h"
#include "powershellhelperselector.h"
#include "PowershellCmdHandler.h"
#include "PublicHook.h"
#include "include/luaengine/Luaengine.h"
#include "include/singleton/singleton.h"

class PowerShellMonitor : 
    public ProtectionBase,
    public Singleton<PowerShellMonitor>,
    public LdrLoadDll_Observer
{
public:
    PowerShellMonitor();
    ~PowerShellMonitor();

public:
    virtual BOOL PrePare(PREPARE_DATA& data) override;
    virtual BOOL DoWork() override;
    virtual BOOL PreCheck() override;

    virtual BOOL ModuleLoadDispatch(LPCWSTR lpModuleName, HMODULE hModule) override;

protected:
    BOOL SetLuaEngine(LuaenginePtr pLuaEngine);
    BOOL CompileMethodHandler(ICorJitInfo* comp, CORINFO_METHOD_INFO* info, unsigned flags, BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    int DynamicInsRunHandler(DynamicIns* Dynamicins, InterpretedFrame* interpretedFrame);
    int InvokeMethodHandler(MethodCallNode* CallNode, ArgumentsArray* arguments);
    int DynamicClassInvokeHandler(MethodInfo* nativeMethod, ArgumentsArray* arguments);
    int CommandProcessorProcessRecordHandler(void* CmdProcessor, void* empty);

private:
    BOOL HookCompileMethod(HMODULE hModule);
    BOOL HookDynamicMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode, ULONG_PTR proxy);
    BOOL HookDynamicInsRun(BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    BOOL HookCommandProcessorProcessRecord(BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    BOOL HookDynamicClassInvoke(BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    BOOL HookInvokeMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    BOOL HookSystemManagementniDll(HMODULE hModule);
    inline void DealWithRegWrite(std::wstring& strFunctionName, void* arg1, void* arg2);
    BOOL CheckPowershellVersion();

private:
    static int __stdcall compileMethod_Proxy(ULONG_PTR classthis, ICorJitInfo* comp, CORINFO_METHOD_INFO* info, unsigned flags, BYTE** nativeEntry, ULONG* nativeSizeOfCode);
    static int __fastcall DynamicInsRun_Proxy(DynamicIns* Dynamicins, InterpretedFrame* interpretedFrame);
    static int __fastcall InvokeMethod_Proxy(MethodCallNode* CallNode, ArgumentsArray* arguments);
    static int __fastcall DynamicClassInvoke_Proxy(MethodInfo* nativeMethod, ArgumentsArray* arguments);
    static int __fastcall CommandProcessorProcessRecord_Proxy(void* CmdProcessor);

private:
    powershellinfo m_powershellinfo;
    HelpPtr m_powershellhelper;
    PowershellCmdHandler m_pscmdhandler;
    LuaenginePtr m_pLuaEngine;
};

