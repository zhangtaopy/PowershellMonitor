#include "pch.h"
#include "powershellhelperselector.h"


powershellhelperselector::powershellhelperselector()
{
}


powershellhelperselector::~powershellhelperselector()
{
}

HelpPtr powershellhelperselector::GetHelper(PowershellVersion ver)
{
    if (ver == PowershellVersion::PSVER5)
    {
        return std::make_unique<powershellhelperV5>();
    }
    else if (ver == PowershellVersion::PSVER3
        || ver == PowershellVersion::PSVER4)
    {
        return std::make_unique<powershellhelperV3V4>();
    }
    else if (ver == PowershellVersion::PSVER2)
    {
        return std::make_unique<powershellhelperV2>();
    }

    return nullptr;
}
