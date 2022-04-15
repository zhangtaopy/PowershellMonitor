#include "pch.h"
#include "Luaengine.h"
#include "Logapi4lua.h"
#include "utils.h"

#ifdef _WIN64
constexpr auto THIS_MODULE_NAME = L"PowershellMonitor64.dll";
#else
constexpr auto THIS_MODULE_NAME = L"PowershellMonitor.dll";
#endif 

constexpr auto LUA_DAT_NAME = "luascript.lua";

Luaengine::Luaengine() : m_bInit(FALSE)
{
}


Luaengine::~Luaengine()
{
}

BOOL Luaengine::Initialize()
{
    if (DoLoadLuaScript())
    {
        m_bInit = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL Luaengine::Uninitialize()
{
    m_bInit = FALSE;
    return TRUE;
}

int Luaengine::ExecuteFunc(const char* szFunc, const char* szParam)
{
    if (!m_bInit)
        return 0;

    int nRet = m_Luastate.call<int>(szFunc, szParam);
    return nRet;
}

BOOL Luaengine::DoLoadLuaScript()
{
    if (m_Luastate.create() != LUA_OK)
        return FALSE;

    if (m_Luastate.open_all_libs() != LUA_OK)
        return FALSE;

    if (!RegisterApiForLua())
        return FALSE;

    return LoadScriptFromFile();
}

BOOL Luaengine::LoadScriptFromFile()
{
    char szLuaPath[MAX_PATH] = { 0 };
    HMODULE hMod = GetModuleHandle(THIS_MODULE_NAME);
    if (hMod == nullptr)
        return FALSE;

    if (GetModuleFileNameA(hMod, szLuaPath, MAX_PATH) == 0)
        return FALSE;

    PathRemoveFileSpecA(szLuaPath);
    PathAppendA(szLuaPath, LUA_DAT_NAME);

    m_Luastate.dofile(szLuaPath);
    return TRUE;
}

BOOL Luaengine::RegisterApiForLua()
{
    z_lua_function_reg reg;
    reg.insert_no_prefix("print", Logprint);
    reg.insert_no_prefix("write", logwrite);
    reg.insert_no_prefix("writebinary", logbinary);
    reg.libname("Log");


    if (m_Luastate.reg_lib(reg) != LUA_OK)
    {
        return FALSE;
    }

    return TRUE;
}
