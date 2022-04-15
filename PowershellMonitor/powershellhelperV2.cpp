#include "pch.h"
#include "powershellhelperV2.h"
#include "hooksigns.h"
#include "utils.h"

powershellhelperV2::powershellhelperV2()
{

}

powershellhelperV2::~powershellhelperV2()
{

}


BOOL powershellhelperV2::IsParmaterValid(void* CallNode, void* arguments)
{
    MethodCallNode* _CallNode = (MethodCallNode*)CallNode;
    ArgumentsArray* _arguments = (ArgumentsArray*)arguments;
    if (IsBadReadPtr((const void*)_CallNode, sizeof(MethodCallNode)))
        return FALSE;

    if (IsBadReadPtr((const void*)_CallNode->_nodeToken, sizeof(NodeToken)))
        return FALSE;

    if (!IsSystemStringV2Valid(_CallNode->_nodeToken->_text)
        || !IsSystemStringV2Valid(_CallNode->_nodeToken->_script))
        return FALSE;

    if (!IsArgumentsArrayValid(_arguments))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::GetOriginalScriptText(void* CallNode, std::wstring& strOriginalScriptText)
{
    MethodCallNode* _CallNode = (MethodCallNode*)CallNode;
    strOriginalScriptText = (LPCWSTR)_CallNode->_nodeToken->_script->buffer;
    return TRUE;
}

BOOL powershellhelperV2::GetFunctionName(void* CallNode, std::wstring& strFunctionName)
{
    MethodCallNode* _CallNode = (MethodCallNode*)CallNode;
    strFunctionName = (LPCWSTR)_CallNode->_nodeToken->_text->buffer;
    return TRUE;
}

BOOL powershellhelperV2::GetFunctionArguments(void* empty, void* arguments, ArgumentVector& vecArgs, ArgumentTypeVector& vecType)
{
    ArgumentsArray* _arguments = (ArgumentsArray*)arguments;
    int nArgCout = _arguments->length;
    if (nArgCout != vecType.size())
        return FALSE;

    for (int i = 0; i < nArgCout; i++)
    {
        if (vecType[i] == ArgType::TYPE_STRING)
        {
            System_stringV2* arg = (System_stringV2*)(_arguments->first[i]);
            if (IsSystemStringV2Valid(arg))
            {
                LPCWSTR szArg = (LPCWSTR)arg->buffer;
                vecArgs.push_back(szArg);
            }
            else
            {
                vecArgs.push_back(L"");
            }
        }
        else if (vecType[i] == ArgType::TYPE_INT)
        {
            Num_value* NumParam = (Num_value*)(_arguments->first[i]);
            if(IsNumValueValid(NumParam))
            {
                vecArgs.push_back(NumParam->value);
            }
            else
            {
                vecArgs.push_back(0);
            }
        }
        else if (vecType[i] == ArgType::TYPE_POINT)
        {
            Point_Value* PointParam = (Point_Value*)(_arguments->first[i]);
            if (IsPointValueValid(PointParam))
            {
                vecArgs.push_back(PointParam->value);
            }
            else
            {
                vecArgs.push_back((PUCHAR)(0));
            }
        }
        else if (vecType[i] == ArgType::TYPE_BINARY)
        {
            Binary_value* BinaryParam = (Binary_value*)(_arguments->first[i]);
            if (IsBinaryValueValid(BinaryParam))
            {
                ULONG_PTR nSize = BinaryParam->length < MaximumBinarySize ? BinaryParam->length : MaximumBinarySize;
                if (IsBadReadPtr(BinaryParam->buffer, nSize))
                {
                    vecArgs.push_back(L"");
                }
                else
                {
                    std::vector<BYTE> vecBin;
                    vecBin.resize(nSize + 1);
                    memcpy_s(&vecBin[0], nSize + 1, BinaryParam->buffer, nSize);
                    std::wstring strBin = Utils::HexToString(vecBin);
                    vecArgs.push_back(strBin);
                }
            }
            else
            {
                vecArgs.push_back(L"");
            }
        }
        else if (vecType[i] == ArgType::TYPE_BINARY_PE)
        {
            Binary_value* BinaryParam = (Binary_value*)(_arguments->first[i]);
            if (IsBinaryValueValid(BinaryParam)
                && BinaryParam->length < MaximunPESize)
            {
                ULONG_PTR nSize = BinaryParam->length;

                if (IsBadReadPtr(BinaryParam->buffer, nSize)
                    || !Utils::IsPeFile(BinaryParam->buffer, static_cast<int>(nSize)))
                {
                    vecArgs.push_back(L"");
                }
                else
                {
                    std::vector<BYTE> vecBin;
                    vecBin.resize(nSize);
                    memcpy_s(&vecBin[0], nSize, BinaryParam->buffer, nSize);
                    std::wstring strBin = CA2W(base64_encode((char*)&vecBin[0], nSize).c_str());
                    vecArgs.push_back(strBin);
                }
            }
            else
            {
                vecArgs.push_back(L"");
            }
        }
        else if (vecType[i] == ArgType::TYPE_IGNORE)
        {
            vecArgs.push_back(L"");
        }
    }

    return (vecArgs.size() == nArgCout);
}

int powershellhelperV2::GetArgumentCount(void* arguments)
{
    ArgumentsArray* _arguments = (ArgumentsArray*)arguments;
    int nArgCout = _arguments->length;
    return nArgCout;
}

BOOL powershellhelperV2::GetNativeMethodPtr(void* nativeMethod, void* empty, PUCHAR& FunctionPtr)
{
    MethodInfo* _nativeMethod = (MethodInfo*)nativeMethod;
    if (!IsMethodInfoValid(_nativeMethod))
        return FALSE;
    FunctionPtr = _nativeMethod->_methodPtrAux;
    return TRUE;
}

BOOL powershellhelperV2::GetPSMethodName(void* PSMethod, void* empty, std::wstring& strPsmethodName)
{
    PSmethodInfo* psMethod = (PSmethodInfo*)PSMethod;
    if (!IsPSMethodValid(psMethod))
        return FALSE;

    System_stringV2* methodName = (System_stringV2*)psMethod->name;
    if (!IsSystemStringV2Valid(methodName))
        return FALSE;

    strPsmethodName = (LPCWSTR)methodName->buffer;

    return TRUE;
}

BOOL powershellhelperV2::GetExtraInfo(void* CallNode, std::wstring& strExtraInfo)
{
    MethodCallNode* _CallNode = (MethodCallNode*)CallNode;
    if (!IsSystemStringV2Valid(_CallNode->_nodeToken->_line))
        return FALSE;
    strExtraInfo = (LPCWSTR)_CallNode->_nodeToken->_line->buffer;
    return TRUE;
}

BOOL powershellhelperV2::CovertArgument(void* ArgStruct, int& nArgCount, void** TrueArgs)
{
    ArgumentsArray* ArgsStruct = (ArgumentsArray*)ArgStruct;
    if (!IsArgumentsArrayValid(ArgsStruct))
        return FALSE;

    if (ArgsStruct->length < 1)
        return FALSE;

    ArgumentsArray* Args = (ArgumentsArray*)ArgsStruct->first[0];

    if (!IsArgumentsArrayValid(Args))
        return FALSE;

    nArgCount = Args->length;
    *TrueArgs = Args;
    return TRUE;
}

BOOL powershellhelperV2::CheckNeedHookDynamicIns(std::string& strMethodName, std::string& strClassName)
{
    return FALSE;
}

BOOL powershellhelperV2::CheckNeedHookDynamicIns()
{
    return FALSE;
}

#ifdef _WIN64
BOOL powershellhelperV2::SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs)
{
    PBYTE pAddr = nullptr;
    if (BM_SearchSign(pBuffer, nBufferLen, byInvokeMethod1, _countof(byInvokeMethod1), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    return TRUE;
}
#else
BOOL powershellhelperV2::SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs)
{
    PBYTE pAddr = nullptr;
    if (BM_SearchSign(pBuffer, nBufferLen, byInvokeMethod1, _countof(byInvokeMethod1), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }
    if (BM_SearchSign(pBuffer, nBufferLen, byInvokeMethod2, _countof(byInvokeMethod2), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }
    return TRUE;
}
#endif


BOOL powershellhelperV2::GetCommandName(void* CmdProcessor, std::wstring& strFunctionName)
{
    CommandProcessorV2* cmdProcessor = (CommandProcessorV2*)CmdProcessor;

    if (!IsCmdProcessorV2Valid(cmdProcessor))
        return FALSE;

    if (!IsCommandArgumentValid(cmdProcessor->arguments))
        return FALSE;

    if (!IsSystemStringV2Valid(cmdProcessor->commandinfo->name))
        return FALSE;

    strFunctionName = (LPCWSTR)cmdProcessor->commandinfo->name->buffer;
    return TRUE;
}

BOOL powershellhelperV2::GetCommandArguments(void* CmdProcessor, CommandArgumentVector& vecArgs, CommandParameterVector& vecParam)
{
    CommandProcessorV2* cmdProcessor = (CommandProcessorV2*)CmdProcessor;

    int nSize = cmdProcessor->arguments->items->_size;
    CommandArgumentItemsV2* commandArgItems = (CommandArgumentItemsV2*)cmdProcessor->arguments->items->_items;
    if (!IsCommandArgumentItemsV2Valid(commandArgItems))
        return FALSE;

    for (int i = 0; i < nSize; i++)
    {
        ArgumentItemV2* ArgItem = (ArgumentItemV2*)commandArgItems->first[i];
        if (!IsArgumentItemV2Valid(ArgItem))
            return FALSE;

        if (ArgItem->_argument1 != nullptr)
        {
            std::wstring strArg = (LPCWSTR)ArgItem->_argument1->buffer;
            vecArgs.push_back(strArg);
        }
    }

    return TRUE;
}

BOOL powershellhelperV2::IsSystemStringV2Valid(System_stringV2* systemstr)
{
    if (IsBadReadPtr((const void*)systemstr, sizeof(System_stringV2)))
        return FALSE;

    if (systemstr->arrayLength <= 0
        ||systemstr->stringLength <= 0
        ||systemstr->arrayLength > MaximumStringSize)
        return FALSE;

    if (systemstr->stringLength > 0 && *(WCHAR*)systemstr->buffer == 0)
        return FALSE;

    if (IsBadReadPtr((const void*)systemstr->buffer, sizeof(WCHAR) * systemstr->arrayLength))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::IsSystemStringV2ValidEx(System_stringV2* systemstr)
{
    if (IsBadReadPtr((const void*)systemstr, sizeof(System_stringV2)))
        return FALSE;

    if (systemstr->arrayLength <= 0
        || systemstr->stringLength <= 0)
        return FALSE;

    if (systemstr->stringLength > 0 && *(WCHAR*)systemstr->buffer == 0)
        return FALSE;

    if (IsBadReadPtr((const void*)systemstr->buffer, sizeof(WCHAR) * systemstr->arrayLength))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::IsArgumentsArrayValid(ArgumentsArray* _arguments)
{
    if (IsBadReadPtr((const void*)_arguments, sizeof(ArgumentsArray)))
        return FALSE;

    if (_arguments->length > 10)
        return FALSE;

    if (IsBadReadPtr((const void*)_arguments->first, sizeof(PUCHAR) * _arguments->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::IsCmdProcessorV2Valid(CommandProcessorV2* CmdProcessor)
{
    if (IsBadReadPtr((const void*)CmdProcessor, sizeof(CommandProcessorV2)))
        return FALSE;

    if (IsBadReadPtr((const void*)CmdProcessor->commandinfo, sizeof(CommandInfoV2)))
        return FALSE;

    if (IsBadReadPtr((const void*)CmdProcessor->arguments, sizeof(CommandArguments)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::IsCommandArgumentItemsV2Valid(CommandArgumentItemsV2* items)
{
    if (IsBadReadPtr((const void*)items, sizeof(CommandArgumentItemsV2)))
        return FALSE;

    if (IsBadReadPtr((const void*)items->first, sizeof(pObject) * items->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV2::IsArgumentItemV2Valid(ArgumentItemV2* ArgItem)
{
    if (IsBadReadPtr((const void*)ArgItem, sizeof(ArgumentItemV2)))
        return FALSE;

    if (ArgItem->_argument1 != nullptr)
    {
        if (!IsSystemStringV2ValidEx(ArgItem->_argument1))
            return FALSE;
    }
    return TRUE;
}
