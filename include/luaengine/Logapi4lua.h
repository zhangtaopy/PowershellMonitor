#pragma once

#include "luasrc/lua.hpp"

/**
* @brief      �ڵ���������ı���Ϣ
* @param[in]  string strOutputText
* @par ����ʾ��
* @code
* Log.print("helloworld")
* @endcode
*/
int Logprint(lua_State *L);

/**
* @brief      �����־
* @param[in]  string logpath
* @param[in]  string strOutputText
* @par ����ʾ��
* @code
* Log.write(logpath, "msg")
* @endcode
*/
int logwrite(lua_State* L);

/**
* @brief      ����������ļ�
* @param[in]  string logpath
* @param[in]  string strOutputText
* @par ����ʾ��
* @code
* Log.writebinary(logpath, "#binary")
* @endcode
*/
int logbinary(lua_State* L);