
#include "GarrysMod/Lua/Interface.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <cstring>
#include <unordered_map>

using namespace GarrysMod::Lua;

// ═══════════════════════════════════════════════════════
//  Zone Handle Storage
// ═══════════════════════════════════════════════════════

struct ZoneEntry
{
    TracyCZoneCtx ctx{};
    bool          ended = false;
};

static std::unordered_map<uint32_t, ZoneEntry> g_zones;
static uint32_t                                g_next_handle = 1;

// ═══════════════════════════════════════════════════════
//  Helper
// ═══════════════════════════════════════════════════════

static void register_field(ILuaBase* lua_base, const char* name, CFunc func)
{
    lua_base->PushCFunction(func);
    lua_base->SetField(-2, name);
}

static ZoneEntry* find_active_zone(ILuaBase* lua_base, int pos)
{
    auto handle = static_cast<uint32_t>(lua_base->CheckNumber(pos));
    auto iter   = g_zones.find(handle);
    if (iter == g_zones.end() || iter->second.ended) {
        lua_base->ThrowError("Invalid or ended Tracy zone handle");
        return nullptr;
    }
    return &iter->second;
}

// ═══════════════════════════════════════════════════════
//  Zone Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_zone_begin_n)
{
    const char* name     = LUA->CheckString(1);
    size_t      name_len = std::strlen(name);

    auto srcloc = ___tracy_alloc_srcloc_name(static_cast<uint32_t>(__LINE__),
                                             __FILE__,
                                             std::strlen(__FILE__),
                                             __FUNCTION__,
                                             std::strlen(__FUNCTION__),
                                             name,
                                             name_len,
                                             0);

    TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(srcloc, 1);

    uint32_t handle = g_next_handle++;
    g_zones[handle] = ZoneEntry{ ctx, false };

    LUA->PushNumber(static_cast<double>(handle));
    return 1;
}

LUA_FUNCTION(tracy_zone_end)
{
    auto handle = static_cast<uint32_t>(LUA->CheckNumber(1));
    auto iter   = g_zones.find(handle);
    if (iter == g_zones.end()) {
        LUA->ThrowError("Invalid Tracy zone handle");
        return 0;
    }
    if (!iter->second.ended) {
        ___tracy_emit_zone_end(iter->second.ctx);
        iter->second.ended = true;
    }
    g_zones.erase(iter);
    return 0;
}

LUA_FUNCTION(tracy_zone_text)
{
    ZoneEntry* zone = find_active_zone(LUA, 1);
    if (!zone) {
        return 0;
    }
    const char* text = LUA->CheckString(2);
    ___tracy_emit_zone_text(zone->ctx, text, std::strlen(text));
    return 0;
}

LUA_FUNCTION(tracy_zone_name)
{
    ZoneEntry* zone = find_active_zone(LUA, 1);
    if (!zone) {
        return 0;
    }
    const char* name = LUA->CheckString(2);
    ___tracy_emit_zone_name(zone->ctx, name, std::strlen(name));
    return 0;
}

LUA_FUNCTION(tracy_zone_color)
{
    ZoneEntry* zone = find_active_zone(LUA, 1);
    if (!zone) {
        return 0;
    }
    auto r = static_cast<uint32_t>(LUA->CheckNumber(2));
    auto g = static_cast<uint32_t>(LUA->CheckNumber(3));
    auto b = static_cast<uint32_t>(LUA->CheckNumber(4));
    ___tracy_emit_zone_color(zone->ctx, (r << 16) | (g << 8) | b);
    return 0;
}

LUA_FUNCTION(tracy_zone_value)
{
    ZoneEntry* zone = find_active_zone(LUA, 1);
    if (!zone) {
        return 0;
    }
    auto val = static_cast<uint64_t>(LUA->CheckNumber(2));
    ___tracy_emit_zone_value(zone->ctx, val);
    return 0;
}

// ═══════════════════════════════════════════════════════
//  Frame Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_frame_mark)
{
    FrameMark;
    return 0;
}

LUA_FUNCTION(tracy_frame_mark_named)
{
    FrameMarkNamed(LUA->CheckString(1));
    return 0;
}

LUA_FUNCTION(tracy_frame_mark_start)
{
    ___tracy_emit_frame_mark_start(LUA->CheckString(1));
    return 0;
}

LUA_FUNCTION(tracy_frame_mark_end)
{
    ___tracy_emit_frame_mark_end(LUA->CheckString(1));
    return 0;
}

// ═══════════════════════════════════════════════════════
//  Message Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_message)
{
    const char* msg = LUA->CheckString(1);
    TracyMessage(msg, std::strlen(msg));
    return 0;
}

LUA_FUNCTION(tracy_message_color)
{
    const char* msg = LUA->CheckString(1);
    auto        r   = static_cast<uint32_t>(LUA->CheckNumber(2));
    auto        g   = static_cast<uint32_t>(LUA->CheckNumber(3));
    auto        b   = static_cast<uint32_t>(LUA->CheckNumber(4));
    TracyMessageC(msg, std::strlen(msg), (r << 16) | (g << 8) | b);
    return 0;
}

// ═══════════════════════════════════════════════════════
//  Utility Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_plot_value)
{
    const char* name = LUA->CheckString(1);
    double      val  = LUA->CheckNumber(2);
    TracyPlot(name, val);
    return 0;
}

LUA_FUNCTION(tracy_app_info)
{
    const char* info = LUA->CheckString(1);
    TracyAppInfo(info, std::strlen(info));
    return 0;
}

LUA_FUNCTION(tracy_is_connected)
{
    LUA->PushBool(static_cast<bool>(TracyIsConnected));
    return 1;
}

// ═══════════════════════════════════════════════════════
//  Module Open / Close
// ═══════════════════════════════════════════════════════

GMOD_MODULE_OPEN()
{
    // ── Build the module table ───────────────────────
    LUA->CreateTable(); // stack: [T]

    register_field(LUA, "ZoneBeginN", tracy_zone_begin_n);
    register_field(LUA, "ZoneEnd", tracy_zone_end);
    register_field(LUA, "ZoneText", tracy_zone_text);
    register_field(LUA, "ZoneName", tracy_zone_name);
    register_field(LUA, "ZoneColor", tracy_zone_color);
    register_field(LUA, "ZoneValue", tracy_zone_value);
    register_field(LUA, "FrameMark", tracy_frame_mark);
    register_field(LUA, "FrameMarkNamed", tracy_frame_mark_named);
    register_field(LUA, "FrameMarkStart", tracy_frame_mark_start);
    register_field(LUA, "FrameMarkEnd", tracy_frame_mark_end);
    register_field(LUA, "Message", tracy_message);
    register_field(LUA, "MessageColor", tracy_message_color);
    register_field(LUA, "PlotValue", tracy_plot_value);
    register_field(LUA, "AppInfo", tracy_app_info);
    register_field(LUA, "IsConnected", tracy_is_connected);

    // stack: [T]

    // ── Also set _G.tracy as a fallback ──────────────
    LUA->Push(-1);                  // stack: [T, T]
    LUA->PushSpecial(SPECIAL_GLOB); // stack: [T, T, _G]
    LUA->Push(-2);                  // stack: [T, T, _G, T]
    LUA->SetField(-2, "tracy");     // stack: [T, T, _G]   (_G.tracy = T)
    LUA->Pop(2); // stack: [T]          (pop _G and duplicate)

    TracyMessageL("gmsv_tracy module loaded");

    // ── Return the table so require("tracy") yields it ──
    return 1;
}

GMOD_MODULE_CLOSE()
{
    for (auto& pair : g_zones) {
        if (!pair.second.ended) {
            ___tracy_emit_zone_end(pair.second.ctx);
        }
    }
    g_zones.clear();

    TracyMessageL("gmsv_tracy module unloaded");
    return 0;
}
