#include <windows.h>
#include <string>
#include <atlbase.h>
#include <Shlwapi.h>
#include "include/inject/inject.h"

#ifdef _WIN64
constexpr auto MODULE_NAME = L"PowershellMonitor64.dll";
#else
constexpr auto MODULE_NAME = L"PowershellMonitor.dll";
#endif 

const WCHAR szPowerShellEngine3[] = L"SOFTWARE\\Microsoft\\PowerShell\\3\\PowerShellEngine";
const WCHAR szPowerShellEngine1[] = L"SOFTWARE\\Microsoft\\PowerShell\\1\\PowerShellEngine";

BOOL GetPowershellPath(std::wstring& strPath)
{
    CRegKey cRegKey;
    LONG lRet = cRegKey.Open(HKEY_LOCAL_MACHINE, szPowerShellEngine3, KEY_READ);
    if (lRet == ERROR_FILE_NOT_FOUND)
    {
        lRet = cRegKey.Open(HKEY_LOCAL_MACHINE, szPowerShellEngine1, KEY_READ);
    }

    if (lRet != ERROR_SUCCESS)
        return FALSE;

    WCHAR szPath[MAX_PATH] = { 0 };
    ULONG ulChars = _countof(szPath);
    lRet = cRegKey.QueryStringValue(L"ApplicationBase", szPath, &ulChars);
    if (lRet != ERROR_SUCCESS)
        return FALSE;

    PathAppend(szPath, L"powershell.exe");
    strPath = szPath;
    return TRUE;
}

BOOL GetDllPath(std::wstring& strDllPath)
{
    WCHAR szDllPath[MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, szDllPath, MAX_PATH) == 0)
        return FALSE;

    PathRemoveFileSpec(szDllPath);
    PathAppend(szDllPath, MODULE_NAME);
    strDllPath = szDllPath;
    return TRUE;
}

int main()
{
    std::wstring strPowershellPath;
    std::wstring strDllPath;
    if (GetPowershellPath(strPowershellPath)
        && GetDllPath(strDllPath))
    {
        WCHAR szPath[MAX_PATH] = { 0 };
        wcscpy_s(szPath, MAX_PATH, strPowershellPath.c_str());
        STARTUPINFO si = {0};
        si.cb = sizeof(STARTUPINFO);
        si.wShowWindow = SW_SHOW;
        PROCESS_INFORMATION pi = {0};
        BOOL bRet = CreateProcess(
            NULL,
            szPath,
            NULL,
            NULL,
            FALSE,
            CREATE_SUSPENDED,
            NULL,
            NULL,
            &si, 
            &pi);

        if (bRet)
        {
            InjectdllByModifyEP(pi.dwProcessId, strDllPath.c_str());
            ResumeThread(pi.hThread);
            if (pi.hProcess)
                CloseHandle(pi.hProcess);
            
            if(pi.hThread)
                CloseHandle(pi.hThread);
        }
    }

    return 0;
    
}