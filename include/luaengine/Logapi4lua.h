#pragma once

#include "luasrc/lua.hpp"

/**
* @brief      在调试器输出文本信息
* @param[in]  string strOutputText
* @par 代码示例
* @code
* Log.print("helloworld")
* @endcode
*/
int Logprint(lua_State *L);

/**
* @brief      输出日志
* @param[in]  string logpath
* @param[in]  string strOutputText
* @par 代码示例
* @code
* Log.write(logpath, "msg")
* @endcode
*/
int logwrite(lua_State* L);

/**
* @brief      输出二进制文件
* @param[in]  string logpath
* @param[in]  string strOutputText
* @par 代码示例
* @code
* Log.writebinary(logpath, "#binary")
* @endcode
*/
int logbinary(lua_State* L);