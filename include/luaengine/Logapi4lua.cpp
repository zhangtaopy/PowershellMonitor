#include "pch.h"
#include "Logapi4lua.h"
#include <string>
#include <atlbase.h>
#include "base64/Base64.h"

int Logprint(lua_State* L)
{
    std::string strMsg(lua_tostring(L, 1));
    strMsg += "\n";
    OutputDebugStringA(strMsg.c_str());

    return 0;
}

int logwrite(lua_State* L)
{
    std::string strLogFileName(lua_tostring(L, 1));
    std::string strMsg(lua_tostring(L, 2));
    strMsg.append("\n");

    HANDLE hFile = INVALID_HANDLE_VALUE;

    DWORD dwDisposition = CREATE_ALWAYS;
    if (PathFileExistsA(strLogFileName.c_str()))
    {
        dwDisposition = OPEN_EXISTING;
    }

    hFile = CreateFileA(strLogFileName.c_str(),
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_READ,
        NULL,
        dwDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    SetFilePointer(hFile, 0, 0, FILE_END);

    DWORD dwWritten = 0;
    WriteFile(hFile, strMsg.c_str(), strMsg.length(), &dwWritten, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return 0;
}

int logbinary(lua_State* L)
{
#define BASE64_8BIT 3
#define BASE64_6BIT 4
#define base64_decode_length(A) (((A + BASE64_6BIT - 1) / BASE64_6BIT) * BASE64_8BIT)

    std::string strLogFileName(lua_tostring(L, 1));
    std::string strMsg(lua_tostring(L, 2));

    HANDLE hFile = INVALID_HANDLE_VALUE;
    char* buffer = NULL;

    do 
    {
        DWORD dwDisposition = CREATE_ALWAYS;
        if (PathFileExistsA(strLogFileName.c_str()))
        {
            dwDisposition = OPEN_EXISTING;
        }

        hFile = CreateFileA(strLogFileName.c_str(),
            GENERIC_WRITE | GENERIC_READ,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_READ,
            NULL,
            dwDisposition,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            break;

        SetFilePointer(hFile, 0, 0, FILE_END);

        int nLen = base64_decode_length(strMsg.length());
        buffer = new char[nLen];
        if(0 == base64_decode_buffer(buffer, nLen, strMsg.c_str(), strMsg.length()))
            break;

        DWORD dwWritten = 0;
        WriteFile(hFile, buffer, nLen, &dwWritten, NULL);
    } while (0);
    

    if(buffer)
        delete[] buffer;

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return 0;
}
