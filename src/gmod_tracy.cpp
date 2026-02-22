#include "GarrysMod/Lua/Interface.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"

using namespace GarrysMod::Lua;

// Manual zone control from Lua
LUA_FUNCTION(tracy_ZoneBeginN)
{
    const char* name = LUA->CheckString(1);
    // Use Tracy's C API for dynamic zones
    TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(
        ___tracy_alloc_srcloc_name(
            __LINE__, __FILE__, strlen(__FILE__), name, strlen(name)),
        true);
    // Store context for later ZoneEnd
    LUA->PushNumber(static_cast<double>(ctx.id));
    return 1;
}

LUA_FUNCTION(tracy_ZoneEnd)
{
    uint32_t      id = static_cast<uint32_t>(LUA->CheckNumber(1));
    TracyCZoneCtx ctx;
    ctx.id     = id;
    ctx.active = 1;
    ___tracy_emit_zone_end(ctx);
    return 0;
}

LUA_FUNCTION(tracy_FrameMark)
{
    FrameMark;
    return 0;
}

LUA_FUNCTION(tracy_Message)
{
    const char* msg = LUA->CheckString(1);
    TracyMessage(msg, strlen(msg));
    return 0;
}

// Auto-profiling via Lua debug hooks
LUA_FUNCTION(tracy_EnableAutoProfile)
{
    // Install lua debug hook that creates Tracy zones for every function call
    tracy::LuaRegister(LUA->GetState());
    return 0;
}

GMOD_MODULE_OPEN()
{
    LUA->PushSpecial(SPECIAL_GLOB);
    LUA->CreateTable();
    LUA->PushCFunction(tracy_ZoneBeginN);
    LUA->SetField(-2, "ZoneBeginN");

    LUA->PushCFunction(tracy_ZoneEnd);
    LUA->SetField(-2, "ZoneEnd");

    LUA->PushCFunction(tracy_FrameMark);
    LUA->SetField(-2, "FrameMark");

    LUA->PushCFunction(tracy_Message);
    LUA->SetField(-2, "Message");

    LUA->PushCFunction(tracy_EnableAutoProfile);
    LUA->SetField(-2, "EnableAutoProfile");
    LUA->SetField(-2, "tracy");
    LUA->Pop();

    return 0;
}

GMOD_MODULE_CLOSE()
{
    return 0;
}