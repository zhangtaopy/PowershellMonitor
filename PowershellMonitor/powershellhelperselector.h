#pragma once
#include <memory>
#include "powershellhelperV5.h"
#include "powershellhelperV3V4.h"
#include "powershellhelperV2.h"
#include "powershellinfo.h"

using HelpPtr = std::unique_ptr<powershellhelperbase>;

class powershellhelperselector
{
public:
    powershellhelperselector();
    ~powershellhelperselector();

public:
    HelpPtr GetHelper(PowershellVersion ver);
};

