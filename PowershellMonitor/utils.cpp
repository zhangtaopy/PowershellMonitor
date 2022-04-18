#include "pch.h"
#include "utils.h"
#include <string>
#include <algorithm>
#include "include/3rdparty/cJson/cJSON.h"

std::wstring Utils::HexToString(std::vector<BYTE>& vecBin)
{
    std::wstring strHex;
    WCHAR szTemp[4] = { 0 };
    for (auto OneByte : vecBin)
    {
        wsprintf(szTemp, L"%02X", OneByte);
        strHex.append(szTemp);
    }

    return std::move(strHex);
}

std::wstring Utils::HexToString(PBYTE pData, ULONG ulSize)
{
    std::wstring strHex;
    WCHAR szTemp[4] = { 0 };
    for (ULONG i = 0; i < ulSize; i++)
    {
        wsprintf(szTemp, L"%02X", pData[i]);
        strHex.append(szTemp);
    }

    return std::move(strHex);
}

BOOL Utils::GetLdrLoadModuleName(PUNICODE_STRING pModuleFileName, PULONG pFlags, std::wstring& strFile)
{
    if (NULL == pModuleFileName)
    {
        return FALSE;
    }
    // 只处理flag为0的情况，其它形式的loadlibraryex都不管
    if ((pFlags != NULL && *pFlags != 0) || NULL == pModuleFileName->Buffer)
    {
        return FALSE;
    }
    strFile = pModuleFileName->Buffer;
    return TRUE;
}

void Utils::FormatCheckerJsonString(std::wstring& strFunctionName,
    std::vector<std::wstring>& vecArgs,
    std::string& strJsonString,
    BOOL bArgLower)
{
    cJSON* root = cJSON_CreateObject();
    std::string strFunctionNameA(CW2A(strFunctionName.c_str()));
    std::transform(strFunctionNameA.begin(), strFunctionNameA.end(), strFunctionNameA.begin(), tolower);
    cJSON_AddItemToObject(root, "functionname", cJSON_CreateString(strFunctionNameA.c_str()));
    cJSON_AddItemToObject(root, "argcount", cJSON_CreateNumber(static_cast<double>(vecArgs.size())));
    for (SIZE_T i = 0; i < vecArgs.size(); i++)
    {
        std::string strArg(CW2A(vecArgs[i].c_str()));
        if (bArgLower)
        {
            std::transform(strArg.begin(), strArg.end(), strArg.begin(), tolower);
        }
        std::string strParamName;
        strParamName += "arg";
        strParamName += char(i + 1 + '0');
        cJSON_AddItemToObject(root, strParamName.c_str(), cJSON_CreateString(strArg.c_str()));
    }

    char* p = cJSON_PrintUnformatted(root);
    strJsonString = p;
    cJSON_Delete(root);
    free(p);
}

BOOL CheckProcessName(LPCWSTR szName)
{
    WCHAR szProcessName[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szProcessName, MAX_PATH);
    LPWSTR lpFileName = PathFindFileName(szProcessName);
    if (lpFileName == nullptr)
        return FALSE;

    if (_wcsicmp(lpFileName, szName) == 0)
        return TRUE;

    return FALSE;
}


BOOL Utils::IsInPowershellProcess()
{
    if (CheckProcessName(L"powershell.exe"))
        return TRUE;

    return FALSE;
}

BOOL Utils::IsValidUrlPrefix(std::wstring& strUrl)
{
    if (_wcsnicmp(strUrl.c_str(), L"http://", 7) == 0 ||
        _wcsnicmp(strUrl.c_str(), L"https://", 8) == 0 ||
        _wcsnicmp(strUrl.c_str(), L"www.", 4) == 0
        )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL Utils::IsPeFile(BYTE* pbyBuffer, int nSize)
{
    BOOL is_pe_file = FALSE;

    PIMAGE_DOS_HEADER p_dos_header = NULL;
    PIMAGE_NT_HEADERS p_nt_header = NULL;

    if (!pbyBuffer)
    {
        goto _abort;
    }

    if (::IsBadReadPtr(pbyBuffer, sizeof(IMAGE_DOS_HEADER)))
    {
        goto _abort;
    }

    __try
    {
        p_dos_header = (PIMAGE_DOS_HEADER)pbyBuffer;
        if (IMAGE_DOS_SIGNATURE != p_dos_header->e_magic)
        {
            goto _abort;
        }

        if (nSize &&
            (ULONG)p_dos_header->e_lfanew >= nSize - sizeof(IMAGE_DOS_HEADER))
        {
            goto _abort;
        }

        p_nt_header = (PIMAGE_NT_HEADERS)((BYTE*)p_dos_header + p_dos_header->e_lfanew);
        if (::IsBadReadPtr(p_nt_header, sizeof(IMAGE_NT_HEADERS)))
        {
            goto _abort;
        }

        if (IMAGE_NT_SIGNATURE == p_nt_header->Signature)
        {
            is_pe_file = TRUE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        is_pe_file = FALSE;
    }

_abort:

    return is_pe_file;
}