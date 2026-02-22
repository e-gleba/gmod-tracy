#include "GarrysMod/Lua/Interface.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <cstring>

using namespace GarrysMod::Lua;

// ─────────────────────────────────────────────
// tracy.ZoneBeginN(name) → zone_id, zone_active
// ─────────────────────────────────────────────
static int tracy_ZoneBeginN(lua_State* state)
{
    const char* name = LUA->CheckString(1);
    const auto  len  = std::strlen(name);

    const auto srcloc = ___tracy_alloc_srcloc_name(__LINE__,
                                                   __FILE__,
                                                   std::strlen(__FILE__),
                                                   __FUNCTION__,
                                                   std::strlen(__FUNCTION__),
                                                   name,
                                                   len,
                                                   0);

    TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(srcloc, true);

    LUA->PushNumber(static_cast<double>(ctx.id));
    LUA->PushNumber(static_cast<double>(ctx.active));
    return 2;
}

// ─────────────────────────────────────────────
// tracy.ZoneEnd(zone_id, zone_active)
// ─────────────────────────────────────────────
static int tracy_ZoneEnd(lua_State* state)
{
    TracyCZoneCtx ctx{};
    ctx.id     = static_cast<uint32_t>(LUA->CheckNumber(1));
    ctx.active = static_cast<int>(LUA->CheckNumber(2));
    ___tracy_emit_zone_end(ctx);
    return 0;
}

// ─────────────────────────────────────────────
// tracy.ZoneText(zone_id, zone_active, text)
// ─────────────────────────────────────────────
static int tracy_ZoneText(lua_State* state)
{
    TracyCZoneCtx ctx{};
    ctx.id     = static_cast<uint32_t>(LUA->CheckNumber(1));
    ctx.active = static_cast<int>(LUA->CheckNumber(2));

    const char* text = LUA->CheckString(3);
    ___tracy_emit_zone_text(ctx, text, std::strlen(text));
    return 0;
}

// ─────────────────────────────────────────────
// tracy.FrameMark()
// ─────────────────────────────────────────────
static int tracy_FrameMark(lua_State*)
{
    FrameMark;
    return 0;
}

// ─────────────────────────────────────────────
// tracy.FrameMarkNamed(name)
// ─────────────────────────────────────────────
static int tracy_FrameMarkNamed(lua_State* state)
{
    const char* name = LUA->CheckString(1);
    FrameMarkNamed(name);
    return 0;
}

// ─────────────────────────────────────────────
// tracy.Message(text)
// ─────────────────────────────────────────────
static int tracy_Message(lua_State* state)
{
    const char* msg = LUA->CheckString(1);
    TracyMessage(msg, std::strlen(msg));
    return 0;
}

// ─────────────────────────────────────────────
// tracy.MessageColor(text, r, g, b)
// ─────────────────────────────────────────────
static int tracy_MessageColor(lua_State* state)
{
    const char* msg   = LUA->CheckString(1);
    const auto  r     = static_cast<uint32_t>(LUA->CheckNumber(2));
    const auto  g     = static_cast<uint32_t>(LUA->CheckNumber(3));
    const auto  b     = static_cast<uint32_t>(LUA->CheckNumber(4));
    const auto  color = (r << 16) | (g << 8) | b;

    TracyMessageC(msg, std::strlen(msg), color);
    return 0;
}

// ─────────────────────────────────────────────
// tracy.PlotValue(name, number)
// ─────────────────────────────────────────────
static int tracy_PlotValue(lua_State* state)
{
    const char*  name = LUA->CheckString(1);
    const double val  = LUA->CheckNumber(2);
    TracyPlot(name, val);
    return 0;
}

// ═════════════════════════════════════════════
//  Module entry / exit
// ═════════════════════════════════════════════
GMOD_MODULE_OPEN()
{
    // Trigger Tracy's lazy init now that GMod is ready
    TracyMessageL("gmod_tracy module loaded");

    LUA->PushSpecial(SPECIAL_GLOB);

    LUA->CreateTable();

    LUA->PushCFunction(tracy_ZoneBeginN);
    LUA->SetField(-2, "ZoneBeginN");

    LUA->PushCFunction(tracy_ZoneEnd);
    LUA->SetField(-2, "ZoneEnd");

    LUA->PushCFunction(tracy_ZoneText);
    LUA->SetField(-2, "ZoneText");

    LUA->PushCFunction(tracy_FrameMark);
    LUA->SetField(-2, "FrameMark");

    LUA->PushCFunction(tracy_FrameMarkNamed);
    LUA->SetField(-2, "FrameMarkNamed");

    LUA->PushCFunction(tracy_Message);
    LUA->SetField(-2, "Message");

    LUA->PushCFunction(tracy_MessageColor);
    LUA->SetField(-2, "MessageColor");

    LUA->PushCFunction(tracy_PlotValue);
    LUA->SetField(-2, "PlotValue");

    LUA->SetField(-2, "tracy");
    LUA->Pop();

    return 0;
}

GMOD_MODULE_CLOSE()
{
    // The macro expands to int gmod13_close(lua_State* state) but you never use
    // state. Silence it
    (void)state;
    return 0;
}
