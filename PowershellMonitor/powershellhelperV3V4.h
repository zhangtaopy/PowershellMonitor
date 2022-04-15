#pragma once
#include "powershellhelperV5.h"

class powershellhelperV3V4 : public powershellhelperV5
{
public:
    powershellhelperV3V4();
    ~powershellhelperV3V4();

public:
    virtual BOOL IsParmaterValid(void* Dynamicins, void* interpretedFrame) override;
    virtual BOOL GetOriginalScriptText(void* interpretedFrame, std::wstring& strOriginalScriptText) override;
    virtual BOOL GetFunctionArguments(void* Dynamicins, void* interpretedFrame, ArgumentVector& vecArgs, ArgumentTypeVector& vecType) override;
    virtual BOOL GetNativeMethodPtr(void* Dynamicins, void* interpretedFrame, PUCHAR& FunctionPtr) override;
    virtual BOOL GetPSMethodName(void* Dynamicins, void* interpretedFrame, std::wstring& strPsmethodName) override;
    virtual BOOL GetExtraInfo(void* interpretedFrame, std::wstring& strExtraInfo) override;
};