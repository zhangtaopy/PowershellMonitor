#pragma once
#include "PowerShellDefine.h"
#include <string>
#include <vector>
#include <atlstr.h>
#include "include/3rdparty/base64/Base64.h"

enum class ArgType { TYPE_INT, TYPE_POINT, TYPE_STRING, TYPE_BINARY, TYPE_BINARY_PE, TYPE_IGNORE};
using ArgumentVector = std::vector<std::variant<std::wstring, PUCHAR, int>>;
using ArgumentTypeVector = std::vector<ArgType>;
using HookAddrs = std::vector<PBYTE>;
using CommandArgumentVector = std::vector<std::wstring>;
using CommandParameterVector = std::vector<std::wstring>;

const static int MaximumBinarySize = 0x100;
const static int MaximumStringSize = 10 * 1024 * 1024;
const static ULONG MaximunPESize = 10 * 1024 * 1024;

class powershellhelperbase
{
public:
    powershellhelperbase(){ };
    virtual ~powershellhelperbase(){ };

public:
    virtual BOOL IsParmaterValid(void* Dynamicins, void* interpretedFrame) = 0;
    virtual BOOL GetOriginalScriptText(void* interpretedFrame, std::wstring& strOriginalScriptText) = 0;
    virtual BOOL GetFunctionName(void* Dynamicins, std::wstring& strFunctionName) = 0;
    virtual BOOL GetFunctionArguments(void* Dynamicins, void* interpretedFrame, ArgumentVector& vecArgs, ArgumentTypeVector& vecType) = 0;
    virtual int GetArgumentCount(void* Dynamicins) = 0;
    virtual BOOL GetNativeMethodPtr(void* Dynamicins, void* interpretedFrame, PUCHAR& FunctionPtr) = 0;
    virtual BOOL GetPSMethodName(void* Dynamicins, void* interpretedFrame, std::wstring& strPsmethodName) = 0;
    virtual BOOL GetExtraInfo(void* interpretedFrame, std::wstring& strExtraInfo) = 0;
    virtual BOOL GetCommandName(void* CmdProcessor, std::wstring& strFunctionName) = 0;
    virtual BOOL GetCommandArguments(void* CmdProcessor, CommandArgumentVector& vecArgs, CommandParameterVector& vecParam) = 0;
    virtual BOOL CheckNeedHookDynamicIns(std::string& strMethodName, std::string& strClassName);
    virtual BOOL CheckNeedHookInvokeMethod(std::string& strMethodName, std::string& strClassName);
    virtual BOOL CheckNeedHookDynamicClassInvoke(std::string& strMethodName, std::string& strClassName);
    virtual BOOL CheckNeedHookDynamicIns();
    virtual BOOL CheckNeedHookInvokeMethod();
    virtual BOOL SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs);
    virtual BOOL SearchingForCommandProcessorAddr(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs);
    virtual BOOL CovertArgument(void* ArgStruct, int& nArgCount, void** TrueArgs);
    BOOL GetMethodName(ICorJitInfo* comp, CORINFO_METHOD_INFO* info, std::string& strMethodName, std::string& strClassName);
    BOOL CheckNeedHookCommandProcessor(std::string& strMethodName, std::string& strClassName);

protected:
    BOOL IsSystemStringValid(System_string* systemstr);
    BOOL IsSystemStringValidEx(System_string* systemstr);
    BOOL IsNumValueValid(Num_value* NumValue);
    BOOL IsPointValueValid(Point_Value* PointValue);
    BOOL IsBinaryValueValid(Binary_value* BinaryValue);
    BOOL IsMethodInfoValid(MethodInfo* nativeMethod);
    BOOL IsPSMethodValid(PSmethodInfo* psMethod);
    BOOL IsCommandArgumentValid(CommandArguments* CommandArg);
};

struct ArgVisitor
{
    std::wstring operator() (int& arg) const
    {
        WCHAR szBuff[20] = { 0 };
        swprintf_s(szBuff, 20, L"%d", arg);
        std::wstring strValue = szBuff;
        return strValue;
    }
    std::wstring operator() (std::wstring& arg) const
    {
        std::wstring strValue = arg;
        return strValue;
    }

    std::wstring operator() (PUCHAR arg) const
    {
        WCHAR szBuff[20] = { 0 };
        swprintf_s(szBuff, 20, L"%p", arg);
        std::wstring strValue = szBuff;
        return strValue;
    }
};
