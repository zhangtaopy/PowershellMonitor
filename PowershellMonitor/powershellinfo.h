#pragma once
#include <string>

enum class PowershellVersion {UNKNOWN = 0, PSVER1, PSVER2, PSVER3, PSVER4, PSVER5, PSVER6OrLater};

class powershellinfo
{
public:
    powershellinfo();
    ~powershellinfo();

public:
    BOOL InitPSVersionInfo();
    PowershellVersion GetPSVersion();
    std::wstring GetPSVersionString();
    std::wstring GetRuntionVersionString();

private:
    PowershellVersion m_PowerShellVersion;
    std::wstring m_strPowerShellVersion;
    std::wstring m_strRuntimeVersion;
};

