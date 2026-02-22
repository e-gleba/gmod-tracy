#include "GarrysMod/Lua/Interface.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <gsl/gsl>

#include <cstring>
#include <string_view>
#include <unordered_map>

using namespace GarrysMod::Lua;

// ═══════════════════════════════════════════════════════
//  Types & State
// ═══════════════════════════════════════════════════════

using ZoneHandle = uint32_t;

struct ZoneEntry
{
    TracyCZoneCtx ctx{};
    bool          ended{ false };
};

static std::unordered_map<ZoneHandle, ZoneEntry> g_zones;
static ZoneHandle                                g_next_handle{ 1 };

// ═══════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════

[[nodiscard]] constexpr auto pack_rgb(uint32_t r,
                                      uint32_t g,
                                      uint32_t b) noexcept -> uint32_t
{
    return (r << 16U) | (g << 8U) | b;
}

static void register_field(gsl::not_null<ILuaBase*> lua_base,
                           gsl::czstring            name,
                           CFunc                    func)
{
    lua_base->PushCFunction(func);
    lua_base->SetField(-2, name);
}

[[nodiscard]] static auto find_active_zone(gsl::not_null<ILuaBase*> lua_base,
                                           int pos) -> ZoneEntry*
{
    const auto handle =
        gsl::narrow_cast<ZoneHandle>(lua_base->CheckNumber(pos));
    auto iter = g_zones.find(handle);
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
    const std::string_view name{ LUA->CheckString(1) };

    const auto srcloc =
        ___tracy_alloc_srcloc_name(gsl::narrow_cast<uint32_t>(__LINE__),
                                   __FILE__,
                                   std::strlen(__FILE__),
                                   __FUNCTION__,
                                   std::strlen(__FUNCTION__),
                                   name.data(),
                                   name.size(),
                                   0);

    const auto ctx    = ___tracy_emit_zone_begin_alloc(srcloc, 1);
    const auto handle = g_next_handle++;
    g_zones[handle]   = ZoneEntry{ .ctx = ctx, .ended = false };

    LUA->PushNumber(static_cast<double>(handle));
    return 1;
}

LUA_FUNCTION(tracy_zone_end)
{
    const auto handle = gsl::narrow_cast<ZoneHandle>(LUA->CheckNumber(1));
    auto       iter   = g_zones.find(handle);
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
    auto* zone = find_active_zone(LUA, 1);
    if (zone == nullptr) {
        return 0;
    }
    const std::string_view text{ LUA->CheckString(2) };
    ___tracy_emit_zone_text(zone->ctx, text.data(), text.size());
    return 0;
}

LUA_FUNCTION(tracy_zone_name)
{
    auto* zone = find_active_zone(LUA, 1);
    if (zone == nullptr) {
        return 0;
    }
    const std::string_view name{ LUA->CheckString(2) };
    ___tracy_emit_zone_name(zone->ctx, name.data(), name.size());
    return 0;
}

LUA_FUNCTION(tracy_zone_color)
{
    auto* zone = find_active_zone(LUA, 1);
    if (zone == nullptr) {
        return 0;
    }
    const auto r = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(2));
    const auto g = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(3));
    const auto b = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(4));
    ___tracy_emit_zone_color(zone->ctx, pack_rgb(r, g, b));
    return 0;
}

LUA_FUNCTION(tracy_zone_value)
{
    auto* zone = find_active_zone(LUA, 1);
    if (zone == nullptr) {
        return 0;
    }
    const auto val = gsl::narrow_cast<uint64_t>(LUA->CheckNumber(2));
    ___tracy_emit_zone_value(zone->ctx, val);
    return 0;
}

// ═══════════════════════════════════════════════════════
//  Frame Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_frame_mark)
{
    (void)LUA; // macro-injected param — unused here
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
    const std::string_view msg{ LUA->CheckString(1) };
    TracyMessage(msg.data(), msg.size());
    return 0;
}

LUA_FUNCTION(tracy_message_color)
{
    const std::string_view msg{ LUA->CheckString(1) };
    const auto             r = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(2));
    const auto             g = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(3));
    const auto             b = gsl::narrow_cast<uint32_t>(LUA->CheckNumber(4));
    TracyMessageC(msg.data(), msg.size(), pack_rgb(r, g, b));
    return 0;
}

// ═══════════════════════════════════════════════════════
//  Utility Functions
// ═══════════════════════════════════════════════════════

LUA_FUNCTION(tracy_plot_value)
{
    const gsl::czstring name = LUA->CheckString(1);
    const auto          val  = LUA->CheckNumber(2);
    TracyPlot(name, val);
    return 0;
}

LUA_FUNCTION(tracy_app_info)
{
    const std::string_view info{ LUA->CheckString(1) };
    TracyAppInfo(info.data(), info.size());
    return 0;
}

LUA_FUNCTION(tracy_is_connected)
{
    LUA->PushBool(TracyIsConnected != 0);
    return 1;
}

// ═══════════════════════════════════════════════════════
//  Module Open / Close
// ═══════════════════════════════════════════════════════

GMOD_MODULE_OPEN()
{
    LUA->CreateTable();

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

    // Also set _G.tracy as a fallback
    LUA->Push(-1);
    LUA->PushSpecial(SPECIAL_GLOB);
    LUA->Push(-2);
    LUA->SetField(-2, "tracy");
    LUA->Pop(2);

    TracyMessageL("gmsv_tracy module loaded");
    return 1;
}

GMOD_MODULE_CLOSE()
{
    (void)LUA; // macro-injected param — unused here

    for (auto& [_, zone] : g_zones) { // C++26 placeholder name
        if (!zone.ended) {
            ___tracy_emit_zone_end(zone.ctx);
        }
    }
    g_zones.clear();

    TracyMessageL("gmsv_tracy module unloaded");
    return 0;
}
