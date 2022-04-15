#pragma once
#include "powershellhelperbase.h"

class powershellhelperV5 : public powershellhelperbase
{
public:
    powershellhelperV5();
    ~powershellhelperV5();

public:
    virtual BOOL IsParmaterValid(void* Dynamicins, void* interpretedFrame) override;
    virtual BOOL GetOriginalScriptText(void* interpretedFrame, std::wstring& strOriginalScriptText) override;
    virtual BOOL GetFunctionName(void* Dynamicins, std::wstring& strFunctionName) override;
    virtual BOOL GetFunctionArguments(void* Dynamicins, void* interpretedFrame, ArgumentVector& vecArgs, ArgumentTypeVector& vecType) override;
    virtual int GetArgumentCount(void* Dynamicins)  override;
    virtual BOOL GetNativeMethodPtr(void* Dynamicins, void* interpretedFrame, PUCHAR& FunctionPtr) override;
    virtual BOOL GetPSMethodName(void* Dynamicins, void* interpretedFrame, std::wstring& strPsmethodName) override;
    virtual BOOL CheckNeedHookInvokeMethod(std::string& strMethodName, std::string& strClassName) override;
    virtual BOOL CheckNeedHookInvokeMethod() override;
    virtual BOOL CheckNeedHookDynamicClassInvoke(std::string& strMethodName, std::string& strClassName) override;
    virtual BOOL GetExtraInfo(void* interpretedFrame, std::wstring& strExtraInfo) override;
    virtual BOOL GetCommandName(void* CmdProcessor, std::wstring& strFunctionName) override;
    virtual BOOL GetCommandArguments(void* CmdProcessor, CommandArgumentVector& vecArgs, CommandParameterVector& vecParam) override;

protected:
    BOOL IsFunctionContexValid(FunctionContext* functionContex);
    BOOL IsExtraInfoValid(Interpreter* _interpreter);
    BOOL IsCmdProcessorValid(CommandProcessor* CmdProcessor);
    BOOL IsCommandArgumentItemsValid(CommandArgumentItems* items);
    BOOL IsArgumentItemValid(ArgumentItem* ArgItem);
};