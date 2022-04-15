#include "pch.h"
#include "powershellhelperbase.h"
#include "hooksigns.h"

BOOL powershellhelperbase::CheckNeedHookDynamicIns(std::string& strMethodName, std::string& strClassName)
{
    const char szDynamicInstuction[] = "DynamicInstruction`";
    const char szRun[] = "Run";
    if (_strnicmp(strClassName.c_str(), szDynamicInstuction, strlen(szDynamicInstuction)) == 0
        && _strnicmp(strMethodName.c_str(), szRun, strlen(szRun)) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL powershellhelperbase::CheckNeedHookDynamicIns()
{
    return TRUE;
}

BOOL powershellhelperbase::CheckNeedHookInvokeMethod(std::string& strMethodName, std::string& strClassName)
{
    const char szMethodCallNode[] = "MethodCallNode";
    const char szInvokeMethod[] = "InvokeMethod";
    if (_strnicmp(strClassName.c_str(), szMethodCallNode, strlen(szMethodCallNode)) == 0
        && _strnicmp(strMethodName.c_str(), szInvokeMethod, strlen(szInvokeMethod)) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL powershellhelperbase::CheckNeedHookInvokeMethod()
{
    return TRUE;
}

BOOL powershellhelperbase::CheckNeedHookDynamicClassInvoke(std::string& strMethodName, std::string& strClassName)
{
    const char szDynamicClass[] = "DynamicClass";
    const char szInvoke[] = "Invoke";
    if (_strnicmp(strClassName.c_str(), szDynamicClass, strlen(szDynamicClass)) == 0
        && _strnicmp(strMethodName.c_str(), szInvoke, strlen(szInvoke)) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

#ifdef _WIN64
BOOL powershellhelperbase::SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs)
{
    PBYTE pAddr = nullptr;
    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns1, _countof(bySignDynamicIns1), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns1_, _countof(bySignDynamicIns1_), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns2, _countof(bySignDynamicIns2), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns2_, _countof(bySignDynamicIns2_), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns3, _countof(bySignDynamicIns3), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns3_, _countof(bySignDynamicIns3_), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns4, _countof(bySignDynamicIns4), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns4_, _countof(bySignDynamicIns4_), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns6, _countof(bySignDynamicIns6), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns7, _countof(bySignDynamicIns7), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }
    return TRUE;
}
#else
BOOL powershellhelperbase::SearchingForHookAddress(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs)
{
    PBYTE pAddr = nullptr;
    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns1, _countof(bySignDynamicIns1), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns2, _countof(bySignDynamicIns2), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns3, _countof(bySignDynamicIns3), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns4, _countof(bySignDynamicIns4), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns6, _countof(bySignDynamicIns6), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, bySignDynamicIns7, _countof(bySignDynamicIns7), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    return TRUE;
}
#endif

BOOL powershellhelperbase::SearchingForCommandProcessorAddr(PBYTE pBuffer, int nBufferLen, HookAddrs& addrs)
{
    PBYTE pAddr = nullptr;
    if (BM_SearchSign(pBuffer, nBufferLen, byCommandProcessorProcessRecord, _countof(byCommandProcessorProcessRecord), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    if (BM_SearchSign(pBuffer, nBufferLen, byCommandProcessorProcessRecord_, _countof(byCommandProcessorProcessRecord_), 0xCC, &pAddr))
    {
        addrs.push_back(pAddr);
    }

    return TRUE;
}

BOOL powershellhelperbase::CovertArgument(void* ArgStruct, int& nArgCount, void** TrueArgs)
{
    *TrueArgs = ArgStruct;
    return TRUE;
}

BOOL powershellhelperbase::IsSystemStringValid(System_string* systemstr)
{
    if (IsBadReadPtr((const void*)systemstr, sizeof(System_string)))
        return FALSE;

    if (systemstr->length <= 0)
        return FALSE;

    if (systemstr->length > 0 && *(WCHAR*)systemstr->buffer == 0)
        return FALSE;

    if (systemstr->length > MaximumStringSize)
        return FALSE;

    if (IsBadReadPtr((const void*)systemstr->buffer, sizeof(WCHAR) * systemstr->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsSystemStringValidEx(System_string* systemstr)
{
    if (IsBadReadPtr((const void*)systemstr, sizeof(System_string)))
        return FALSE;

    if (systemstr->length <= 0)
        return FALSE;

    if (systemstr->length > 0 && *(WCHAR*)systemstr->buffer == 0)
        return FALSE;

    if (IsBadReadPtr((const void*)systemstr->buffer, sizeof(WCHAR) * systemstr->length))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::GetMethodName(
    ICorJitInfo* comp,
    CORINFO_METHOD_INFO* info,
    std::string& strMethodName,
    std::string& strClassName)
{
    if (!comp || !info)
        return FALSE;
    const char* szMethodName = nullptr;
    const char* szClassName = nullptr;
    CEEInfoIndexs * infoIndex = (CEEInfoIndexs*)comp->vBTable;
    ICorMethodInfo_Hack * methodInfo_hack = (ICorMethodInfo_Hack*)((ULONG_PTR)(comp)+sizeof(PUCHAR) + infoIndex->dwICorMethodInfo_Hack_Index);
    szMethodName = (methodInfo_hack->VTable->getMethodName)((ICorJitInfo*)methodInfo_hack, info->ftn, &szClassName);
    if (szMethodName != nullptr && szClassName != nullptr)
    {
        strMethodName = szMethodName;
        strClassName = szClassName;
        return TRUE;
    }

    return FALSE;
}

BOOL powershellhelperbase::CheckNeedHookCommandProcessor(std::string& strMethodName, std::string& strClassName)
{
    const char szCommandProcessor[] = "CommandProcessor";
    const char szProcessRecord[] = "ProcessRecord";
    if (_strnicmp(strClassName.c_str(), szCommandProcessor, strlen(szCommandProcessor)) == 0
        && _strnicmp(strMethodName.c_str(), szProcessRecord, strlen(szProcessRecord)) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL powershellhelperbase::IsNumValueValid(Num_value* NumValue)
{
    if (IsBadReadPtr((const void*)NumValue, sizeof(Num_value)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsPointValueValid(Point_Value* PointValue)
{
    if (IsBadReadPtr((const void*)PointValue, sizeof(Point_Value)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsBinaryValueValid(Binary_value* BinaryValue)
{
    if (IsBadReadPtr((const void*)BinaryValue, sizeof(Binary_value)))
        return FALSE;

    if (BinaryValue->length <= 0)
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsMethodInfoValid(MethodInfo* nativeMethod)
{
    if (IsBadReadPtr((const void*)nativeMethod, sizeof(MethodInfo)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsPSMethodValid(PSmethodInfo* psMethod)
{
    if (IsBadReadPtr((const void*)psMethod, sizeof(PSmethodInfo)))
        return FALSE;

    return TRUE;
}

BOOL powershellhelperbase::IsCommandArgumentValid(CommandArguments* CommandArg)
{
    if (IsBadReadPtr((const void*)CommandArg, sizeof(CommandArguments)))
        return FALSE;

    if (IsBadReadPtr((const void*)CommandArg->items, sizeof(ItemsList)))
        return FALSE;

    return TRUE;
}
