#pragma once
#include "powershellhelperbase.h"

class powershellhelperV2 : public powershellhelperbase
{
public:
    powershellhelperV2();
    ~powershellhelperV2();

public:
    virtual BOOL IsParmaterValid(void* CallNode, void* arguments) override;
    virtual BOOL GetOriginalScriptText(void* CallNode, std::wstring& strOriginalScriptText) override;
    virtual BOOL GetFunctionName(void* CallNode, std::wstring& strFunctionName) override;
    virtual BOOL GetFunctionArguments(void* arguments, void* empty, ArgumentVector& vecArgs, ArgumentTypeVector& vecType) override;
    virtual int GetArgumentCount(void* arguments)  override;
    virtual BOOL GetNativeMethodPtr(void* nativeMethod, void* empty, PUCHAR& FunctionPtr) override;
    virtual BOOL GetPSMethodName(void* PSMethod, void* empty, std::wstring& strPsmethodName) override;
    virtual BOOL GetExtraInfo(void* CallNode, std::wstring& strExtraInfo) override;
    virtual BOOL CovertArgument(void* ArgStruct, int& nArgCount, void** TrueArgs) override;
    virtual BOOL CheckNeedHookDynamicIns(std::string& strMethodName, std::string& strClassName) override;
    virtual BOOL CheckNeedHookDynamicIns();
    virtual BOOL SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs) override;
    virtual BOOL GetCommandName(void* CmdProcessor, std::wstring& strFunctionName) override;
    virtual BOOL GetCommandArguments(void* CmdProcessor, CommandArgumentVector& vecArgs, CommandParameterVector& vecParam) override;
private:
    BOOL IsSystemStringV2Valid(System_stringV2* systemstr);
    BOOL IsSystemStringV2ValidEx(System_stringV2* systemstr);
    BOOL IsArgumentsArrayValid(ArgumentsArray* _arguments);
    BOOL IsCmdProcessorV2Valid(CommandProcessorV2* CmdProcessor);
    BOOL IsCommandArgumentItemsV2Valid(CommandArgumentItemsV2* items);
    BOOL IsArgumentItemV2Valid(ArgumentItemV2* ArgItem);
};

