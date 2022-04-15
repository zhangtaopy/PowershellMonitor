#pragma once
#include <memory>
#include "include/luaengine/Luaengine.h"

typedef struct _PREPARE_DATA
{
    std::shared_ptr<Luaengine> pLuaEngine;
}PREPARE_DATA;

class ProtectionBase
{
public:
    ProtectionBase();
    ~ProtectionBase();

public:
    virtual BOOL PrePare(PREPARE_DATA& data) = 0;
    virtual BOOL PreCheck() = 0;
    virtual BOOL DoWork() = 0;
};

