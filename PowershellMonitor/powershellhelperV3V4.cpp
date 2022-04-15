#include "pch.h"
#include "powershellhelperV3V4.h"
#include "utils.h"


powershellhelperV3V4::powershellhelperV3V4()
{
}


powershellhelperV3V4::~powershellhelperV3V4()
{
}

BOOL powershellhelperV3V4::IsParmaterValid(void* Dynamicins, void* interpretedFrame)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;

    if (IsBadReadPtr((const void*)_Dynamicins, sizeof(DynamicIns)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site, sizeof(CallSite)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site->binder, sizeof(Binder)))
        return FALSE;

    if (IsBadReadPtr((const void*)_Dynamicins->site->binder->callinfo, sizeof(CallInfo)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpretedFrame, sizeof(InterpretedFrameV3)))
        return FALSE;

    if (IsBadReadPtr((const void*)_interpretedFrame->Data, sizeof(InterpretedFrameDataV3)))
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

BOOL powershellhelperV3V4::GetOriginalScriptText(void* interpretedFrame, std::wstring& strOriginalScriptText)
{
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;
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

BOOL powershellhelperV3V4::GetFunctionArguments(
    void* Dynamicins, 
    void* interpretedFrame,
    ArgumentVector& vecArgs,
    ArgumentTypeVector& vecType)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;

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

BOOL powershellhelperV3V4::GetExtraInfo(void* interpretedFrame, std::wstring& strExtraInfo)
{
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;
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

BOOL powershellhelperV3V4::GetNativeMethodPtr(void* Dynamicins, void* interpretedFrame, PUCHAR& FunctionPtr)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;

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

BOOL powershellhelperV3V4::GetPSMethodName(void* Dynamicins, void* interpretedFrame, std::wstring& strPsmethodName)
{
    DynamicIns* _Dynamicins = (DynamicIns*)Dynamicins;
    InterpretedFrameV3* _interpretedFrame = (InterpretedFrameV3*)interpretedFrame;

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
