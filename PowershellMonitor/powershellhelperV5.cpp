#include "pch.h"
#include "powershellhelperV5.h"
#include "utils.h"


powershellhelperV5::powershellhelperV5()
{
}


powershellhelperV5::~powershellhelperV5()
{
}

BOOL powershellhelperV5::IsParmaterValid(void* Dynamicins, void* interpretedFrame)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;

    if (IsBadReadPtr((const void*)_Dynamicins, sizeof(DynamicIns)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site, sizeof(CallSite)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site->binder, sizeof(Binder)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site->binder->callinfo, sizeof(CallInfo)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpretedFrame, sizeof(InterpretedFrame)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpretedFrame->Data, sizeof(InterpretedFrameData)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpretedFrame->Data->first, sizeof(PUCHAR) * _interpretedFrame->Data->length))
        return FALSE;

    if (_interpretedFrame->StackIndex - 1 > _interpretedFrame->Data->length)
        return FALSE;

    if (_Dynamicins->site->binder->callinfo->argcount > 9
        || _Dynamicins->site->binder->callinfo->argcount > _interpretedFrame->Data->length)
        return FALSE;

    if (!IsSystemStringValid(_Dynamicins->site->binder->name))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV5::GetOriginalScriptText(void* interpretedFrame, std::wstring& strOriginalScriptText)
{
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;
    if (_interpretedFrame->Data->length > 0)
    {
        FunctionContext* functionContex = (FunctionContext*)_interpretedFrame->Data->first[0];
        if (functionContex->_sequencePoints->length > 0)
        {
            InternalScriptExtent* scriptExtend = (InternalScriptExtent*)functionContex->_sequencePoints->first[0];
            if (!IsBadReadPtr((const void*)scriptExtend, sizeof(InternalScriptExtent))
                && !IsBadReadPtr((const void*)scriptExtend->_positionHelper, sizeof(PositionHelper)))
            {
                System_string* scriptText = scriptExtend->_positionHelper->_scriptText;
                if (IsSystemStringValid(scriptText))
                {
                    strOriginalScriptText = (LPCWSTR)scriptText->buffer;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL powershellhelperV5::GetFunctionName(void* Dynamicins, std::wstring& strFunctionName)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    if (IsSystemStringValid(_Dynamicins->site->binder->name))
    {
        strFunctionName = (LPCWSTR)_Dynamicins->site->binder->name->buffer;
        return TRUE;
    }

    return FALSE;
}

BOOL powershellhelperV5::GetFunctionArguments(
    void* Dynamicins, 
    void* interpretedFrame,
    ArgumentVector& vecArgs,
    ArgumentTypeVector& vecType)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;

    int nArgcount = _Dynamicins->site->binder->callinfo->argcount;
    if (nArgcount != vecType.size())
        return FALSE;

    for (int i = nArgcount; i > 0; i--)
    {
        if (vecType[nArgcount - i] == ArgType::TYPE_STRING)
        {
            System_string* param = (System_string*)(_interpretedFrame->Data->first[_interpretedFrame->StackIndex - i]);
            if (IsSystemStringValid(param))
            {
                LPCWSTR lpParam = (LPCWSTR)param->buffer;
                vecArgs.push_back(lpParam);
            }
            else
            {
                vecArgs.push_back(L"");
            }
        }
        else if (vecType[nArgcount - i] == ArgType::TYPE_INT)
        {
            Num_value* NumParam = (Num_value*)(_interpretedFrame->Data->first[_interpretedFrame->StackIndex - i]);
            if (IsNumValueValid(NumParam))
            {
                vecArgs.push_back(NumParam->value);
            }
            else
            {
                vecArgs.push_back(0);
            }
        }
        else if(vecType[nArgcount - i] == ArgType::TYPE_POINT)
        {
            Point_Value* PointParam = (Point_Value*)(_interpretedFrame->Data->first[_interpretedFrame->StackIndex - i]);
            if (IsPointValueValid(PointParam))
            {
                vecArgs.push_back(PointParam->value);
            }
            else
            {
                vecArgs.push_back((PUCHAR)(0));
            }
        }
        else if (vecType[nArgcount - i] == ArgType::TYPE_BINARY)
        {
            Binary_value* BinaryParam = (Binary_value*)(_interpretedFrame->Data->first[_interpretedFrame->StackIndex - i]);
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
                    vecBin.resize(nSize);
                    memcpy_s(&vecBin[0], nSize, BinaryParam->buffer, nSize);
                    std::wstring strBin = Utils::HexToString(vecBin);
                    vecArgs.push_back(strBin);
                }
            }
            else
            {
                vecArgs.push_back(L"");
            }
        }
        else if (vecType[nArgcount - i] == ArgType::TYPE_BINARY_PE)
        {
            Binary_value* BinaryParam = (Binary_value*)(_interpretedFrame->Data->first[_interpretedFrame->StackIndex - i]);
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
        else if (vecType[nArgcount - i] == ArgType::TYPE_IGNORE)
        {
            vecArgs.push_back(L"");
        }
    }

    return (vecArgs.size() == nArgcount);
}

BOOL powershellhelperV5::CheckNeedHookInvokeMethod(std::string& strMethodName, std::string& strClassName)
{
    return FALSE;
}

BOOL powershellhelperV5::CheckNeedHookInvokeMethod()
{
    return FALSE;
}

BOOL powershellhelperV5::CheckNeedHookDynamicClassInvoke(std::string& strMethodName, std::string& strClassName)
{
    return FALSE;
}

BOOL powershellhelperV5::GetExtraInfo(void* interpretedFrame, std::wstring& strExtraInfo)
{
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;
    Interpreter* _interpreter = _interpretedFrame->_interpreter;
    if (!IsExtraInfoValid(_interpreter))
        return FALSE;

    ULONG nObjectCount = _interpreter->_objects->length;
    for (ULONG i = 1; i < nObjectCount; i++)
    {
        if (IsSystemStringValid((System_string*)_interpreter->_objects->first[i]))
        {
            System_string* ObjectString = (System_string*)_interpreter->_objects->first[i];
            if (_wcsicmp((LPCWSTR)ObjectString->buffer, L"New-Object") == 0)
            {
                if (i + 2 < _interpreter->_objects->length)
                {
                    System_string* obStr = (System_string*)_interpreter->_objects->first[i + 2];
                    strExtraInfo = (LPCWSTR)obStr->buffer;
                    if (_wcsicmp(strExtraInfo.c_str(), L"ComObject") == 0
                        || _wcsicmp(strExtraInfo.c_str(), L"com") == 0)
                    {
                        if (i + 5 < _interpreter->_objects->length)
                        {
                            System_string* obStr = (System_string*)_interpreter->_objects->first[i + 5];
                            strExtraInfo = (LPCWSTR)obStr->buffer;
                        }
                    }
                }
                break;
            }
        }
    }

    return TRUE;
}

BOOL powershellhelperV5::GetCommandName(void* CmdProcessor, std::wstring& strFunctionName)
{
    CommandProcessor* cmdProcessor = (CommandProcessor*)CmdProcessor;

    if (!IsCmdProcessorValid(cmdProcessor))
        return FALSE;

    if (!IsCommandArgumentValid(cmdProcessor->arguments))
        return FALSE;

    if (!IsSystemStringValid(cmdProcessor->commandinfo->name))
        return FALSE;

    strFunctionName = (LPCWSTR)cmdProcessor->commandinfo->name->buffer;
    return TRUE;
}

BOOL powershellhelperV5::GetCommandArguments(void* CmdProcessor, CommandArgumentVector& vecArgs, CommandParameterVector& vecParam)
{
    CommandProcessor* cmdProcessor = (CommandProcessor*)CmdProcessor;

    int nSize = cmdProcessor->arguments->items->_size;
    CommandArgumentItems* commandArgItems = (CommandArgumentItems*)cmdProcessor->arguments->items->_items;
    if (!IsCommandArgumentItemsValid(commandArgItems))
        return FALSE;

    for (int i = 0; i < nSize; i++)
    {
        ArgumentItem* ArgItem = (ArgumentItem*)commandArgItems->first[i];
        if (!IsArgumentItemValid(ArgItem))
            return FALSE;

        if (ArgItem->_parameter != nullptr)
        {
            std::wstring strParam = (LPCWSTR)ArgItem->_parameter->parameterText->buffer;
            vecParam.push_back(strParam);
        }
        else if (ArgItem->_argument != nullptr)
        {
            std::wstring strArg = (LPCWSTR)ArgItem->_argument->value->buffer;
            vecArgs.push_back(strArg);
        }
        else 
        {

        }
    }

    return TRUE;
}

int powershellhelperV5::GetArgumentCount(void* Dynamicins)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;

    int nArgcount = _Dynamicins->site->binder->callinfo->argcount;

    return nArgcount;
}

BOOL powershellhelperV5::GetNativeMethodPtr(void* Dynamicins, void* interpretedFrame, PUCHAR& FunctionPtr)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;

    int nArgcount = _Dynamicins->site->binder->callinfo->argcount;

    //> 检查数组是否越界
    if (_interpretedFrame->StackIndex - nArgcount - 1 < 0)
        return FALSE;

    MethodInfo* nativeMethod = (MethodInfo*)_interpretedFrame->Data->first[_interpretedFrame->StackIndex - nArgcount - 1];
    if (!IsMethodInfoValid(nativeMethod))
        return FALSE;

    FunctionPtr = nativeMethod->_methodPtrAux;
    return TRUE;
}

BOOL powershellhelperV5::GetPSMethodName(void* Dynamicins, void* interpretedFrame, std::wstring& strPsmethodName)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrame* _interpretedFrame = (InterpretedFrame*)interpretedFrame;

    int nArgcount = _Dynamicins->site->binder->callinfo->argcount;

    //> 检查数组是否越界
    if (_interpretedFrame->StackIndex - nArgcount - 1 < 0)
        return FALSE;

    PSmethodInfo* psMethod = (PSmethodInfo*)_interpretedFrame->Data->first[_interpretedFrame->StackIndex - nArgcount - 1];
    if (!IsPSMethodValid(psMethod))
        return FALSE;

    System_string* methodName = (System_string*)psMethod->name;
    if (!IsSystemStringValid(methodName))
        return FALSE;
    
    strPsmethodName = (LPCWSTR)methodName->buffer;
    return TRUE;
}

BOOL powershellhelperV5::IsFunctionContexValid(FunctionContext* functionContex)
{
    if (IsBadReadPtr((const void*)functionContex, sizeof(FunctionContext)))
        return FALSE;

    if (functionContex->_sequencePoints->length > 10)
        return FALSE;

    if (IsBadReadPtr((const void*)functionContex->_sequencePoints, sizeof(PUCHAR) * functionContex->_sequencePoints->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV5::IsExtraInfoValid(Interpreter* _interpreter)
{
    if (IsBadReadPtr((const void*)_interpreter, sizeof(Interpreter)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpreter->_objects, sizeof(Objects)))
        return FALSE;

    if (_interpreter->_objects->length <= 0)
        return FALSE;

    if (IsBadReadPtr((const void*)_interpreter->_objects->first, sizeof(pObject) * _interpreter->_objects->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV5::IsCmdProcessorValid(CommandProcessor* CmdProcessor)
{
    if (IsBadReadPtr((const void*)CmdProcessor, sizeof(CommandProcessor)))
        return FALSE;

    if (IsBadReadPtr((const void*)CmdProcessor->commandinfo, sizeof(CommandInfo)))
        return FALSE;

    if(IsBadReadPtr((const void*)CmdProcessor->arguments, sizeof(CommandArguments)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV5::IsCommandArgumentItemsValid(CommandArgumentItems* items)
{
    if (IsBadReadPtr((const void*)items, sizeof(CommandArgumentItems)))
        return FALSE;

    if (IsBadReadPtr((const void*)items->first, sizeof(pObject) * items->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperV5::IsArgumentItemValid(ArgumentItem* ArgItem)
{
    if (IsBadReadPtr((const void*)ArgItem, sizeof(ArgumentItem)))
        return FALSE;

    if (ArgItem->_argument != nullptr)
    {
        if (IsBadReadPtr((const void*)ArgItem->_argument, sizeof(Argument)))
            return FALSE;

        if (!IsSystemStringValidEx(ArgItem->_argument->value))
            return FALSE;
    }

    if (ArgItem->_parameter != nullptr)
    {
        if (IsBadReadPtr((const void*)ArgItem->_parameter, sizeof(Parameter)))
            return FALSE;

        if (!IsSystemStringValidEx(ArgItem->_parameter->parameterName))
            return FALSE;
    }

    return TRUE;
}
