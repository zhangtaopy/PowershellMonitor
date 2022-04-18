#pragma once
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "powershellhelperselector.h"
#include "include/luaengine/Luaengine.h"


class NativeStrategy;
class NormalStrategy;
class CommandStrategy;

using NormalStrategyPtr = std::shared_ptr<NormalStrategy>;
using NativeStrategyPtr = std::shared_ptr<NativeStrategy>;
using CommandStrategyPtr = std::shared_ptr<CommandStrategy>;
using CmdHandlerSet = std::unordered_set<std::wstring>;
using CmdActionResult = std::unordered_map<unsigned int, int>;
using NativeCmdStragetyFunc = std::function<int(HelpPtr&, void*, void*)>;
using NativeCallBackFunc = std::function<int(int, std::wstring&, ArgumentVector&)>;
using NormalCallBackFunc = std::function<int(int, std::wstring&, ArgumentVector&)>;
using CommandCallBackFunc = std::function<int(int, std::wstring&, std::wstring&)>;

class NativeStrategy
{
public:
    int id;
    std::wstring strFunctionName;
    PUCHAR FunctionPtr;
    ArgumentTypeVector vecTypes;
    NativeCallBackFunc callback;
};


class NormalStrategy
{
public:
    int id;
    std::wstring strFunctionName;
    std::vector<std::wstring> vecExtraInfo;
    ArgumentTypeVector vecTypes;
    NormalCallBackFunc callback;
};

class CommandStrategy
{
public:
    int id;
    std::wstring strCommandName;
    CommandCallBackFunc callback;
};

using NativeStrategyMap = std::map<int, std::vector<NativeStrategyPtr>>;
using NormalStrategyMap = std::map<int, std::vector<NormalStrategyPtr>>;
using CommnadStrategyVecor = std::vector<CommandStrategyPtr>;


class PowershellCmdHandler
{
public:
    PowershellCmdHandler();
    ~PowershellCmdHandler();

public:
    BOOL Initialize(LuaenginePtr pLuaEngine);
    BOOL Uninitialize();
    void InitNativeStrategys();
    void InitNormalStrategys();
    void InitCommandStrategys();
    BOOL IsCmdNeedHandle(std::wstring& strFunctionName);
    BOOL IsCommandNeedHanle(std::wstring& strCommandName);
    int NativeMethodHandler(HelpPtr& helper, int nArgCount, void* DynamicinsOrNativeMethod, void* InterpretedFrameOrarguments);
    int NormalMethodHandler(HelpPtr& helper, int nArgCount, std::wstring& strFunction, void* DynamicinsOrNativeMethod, void* InterpretedFrameOrarguments);
    int NormalMethodHandlerForInvoke(HelpPtr& helper, int nArgCount, std::wstring& strFunction, void* Dynamicins, void* interpretedFrame);
    int CommandHandler(HelpPtr& helper, std::wstring& strCommandName, void* CmdProcessor);

protected:
    int CommonCallback(int nId, std::wstring& strFunctionName, ArgumentVector& vecArgs);
    int CommandCallback(int nId, std::wstring& strFunctionName, std::wstring& strArg);
    int LoadAssemblyCallback(int nId, std::wstring& strFunctionName, ArgumentVector& vecArgs);
    int OpenReadCallback(int nId, std::wstring& strFunctionName, ArgumentVector& vecArgs);

private:
    BOOL SetActionResult(std::wstring& strContex, int nActionResult);
    BOOL GetActionResult(std::wstring& strContex, int& nActionResult);

private:
    CmdHandlerSet m_CmdSet;
    CmdHandlerSet m_CommandSet;
    NormalStrategyMap m_mapNormalStrategys;
    NativeStrategyMap m_mapNativeStrategys;
    CommnadStrategyVecor m_vecCommandStrategys;
    CmdActionResult m_CmdActionResult;
    std::mutex m_ActionResultMutex;
    LuaenginePtr m_pLuaEngine;
};

