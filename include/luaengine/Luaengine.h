#pragma once
#include <windows.h>
#include <memory>
#include "LuaFunctionDefine.h"
#include "include/z_lua_bind/z_lua_bind.h"

class Luaengine
{
public:
    Luaengine();
    ~Luaengine();

public:
    BOOL Initialize();
    BOOL Uninitialize();
    int ExecuteFunc(const char* szFunc, const char* szParam);

protected:
    BOOL DoLoadLuaScript();
    BOOL LoadScriptFromFile();
    BOOL RegisterApiForLua();

private:
    z_lua_state m_Luastate;
    BOOL m_bInit;
};

using LuaenginePtr = std::shared_ptr<Luaengine>;