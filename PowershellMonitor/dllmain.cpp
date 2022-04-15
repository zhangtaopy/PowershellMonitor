// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "PowerShellMonitor.h"
#include "include/luaengine/Luaengine.h"

PowerShellMonitor g_Monitor;
LuaenginePtr g_pLuaEngine;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_pLuaEngine = std::make_shared<Luaengine>();
        g_pLuaEngine->Initialize();
        PREPARE_DATA data;
        data.pLuaEngine = g_pLuaEngine;
        if (g_Monitor.getInstancePtr()->PrePare(data)
            && g_Monitor.getInstancePtr()->PreCheck())
        {
            g_Monitor.getInstancePtr()->DoWork();
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

