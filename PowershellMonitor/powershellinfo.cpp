#include "pch.h"
#include "powershellinfo.h"
#include <Shlwapi.h>
#include <atlbase.h>

const WCHAR szPowerShellEngine3[] = L"SOFTWARE\\Microsoft\\PowerShell\\3\\PowerShellEngine";
const WCHAR szPowerShellEngine1[] = L"SOFTWARE\\Microsoft\\PowerShell\\1\\PowerShellEngine";

powershellinfo::powershellinfo() : m_PowerShellVersion(PowershellVersion::UNKNOWN)
{
}


powershellinfo::~powershellinfo()
{
}

BOOL powershellinfo::InitPSVersionInfo()
{
    CRegKey cRegKey;
    LONG lRet = cRegKey.Open(HKEY_LOCAL_MACHINE, szPowerShellEngine3, KEY_READ);
    if (lRet == ERROR_FILE_NOT_FOUND)
    {
        lRet = cRegKey.Open(HKEY_LOCAL_MACHINE, szPowerShellEngine1, KEY_READ);
    }

    if (lRet != ERROR_SUCCESS)
        return FALSE;

    WCHAR szVer[20] = { 0 };
    ULONG ulChars = _countof(szVer);
    lRet = cRegKey.QueryStringValue(L"PowerShellVersion", szVer, &ulChars);
    if (lRet != ERROR_SUCCESS)
        return FALSE;
    m_strPowerShellVersion = szVer;
    
    ulChars = _countof(szVer);
    lRet = cRegKey.QueryStringValue(L"RuntimeVersion", szVer, &ulChars);
    if (lRet != ERROR_SUCCESS)
        return FALSE;
    m_strRuntimeVersion = szVer;

    if (_wcsnicmp(m_strPowerShellVersion.c_str(), L"5.1", wcslen(L"5.1")) == 0)
    {
        m_PowerShellVersion = PowershellVersion::PSVER5;
    }
    else if (_wcsnicmp(m_strPowerShellVersion.c_str(), L"4.0", wcslen(L"4.0")) == 0)
    {
        m_PowerShellVersion = PowershellVersion::PSVER4;
    }
    else if (_wcsnicmp(m_strPowerShellVersion.c_str(), L"3.0", wcslen(L"3.0")) == 0)
    {
        m_PowerShellVersion = PowershellVersion::PSVER3;
    }
    else if (_wcsnicmp(m_strPowerShellVersion.c_str(), L"2.0", wcslen(L"2.0")) == 0)
    {
        m_PowerShellVersion = PowershellVersion::PSVER2;
    }
    else if (_wcsnicmp(m_strPowerShellVersion.c_str(), L"1.0", wcslen(L"1.0")) == 0)
    {
        m_PowerShellVersion = PowershellVersion::PSVER1;
    }
    else
    {
        m_PowerShellVersion = PowershellVersion::PSVER6OrLater;
    }

    return TRUE;
}

PowershellVersion powershellinfo::GetPSVersion()
{
    return m_PowerShellVersion;
}

std::wstring powershellinfo::GetPSVersionString()
{
    return m_strPowerShellVersion;
}

std::wstring powershellinfo::GetRuntionVersionString()
{
    return m_strRuntimeVersion;
}

