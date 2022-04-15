#pragma once
#include <windows.h>
#include "winternl.h"
#include <vector>
#include <string>
#include <ShellAPI.h>

namespace Utils
{
    std::wstring HexToString(std::vector<BYTE>& vecBin);
    std::wstring HexToString(PBYTE pData, ULONG ulSize);
    BOOL GetLdrLoadModuleName(PUNICODE_STRING pModuleFileName, PULONG pFlags, std::wstring& strFile);
    void FormatCheckerJsonString(std::wstring& strFunctioName, std::vector<std::wstring>& vecArgs, std::string& strJsonString, BOOL bArgLower = TRUE);
    BOOL IsInPowershellProcess();
    BOOL IsValidUrlPrefix(std::wstring& strUrl);
    BOOL IsPeFile(BYTE* pbyBuffer, int nSize);
}
