if tracy and type(tracy) == "table" then
    print("[Tracy] Already loaded, skipping duplicate require")
    return
end

local ok, result = pcall(require, "gmod_tracy")

if not ok then
    print("[Tracy] Failed to load binary module: " .. tostring(result))
    return
end

-- GMod binary require may return the table, or just true, or nil.
-- The C module also tries to set _G.tracy directly.
-- Handle ALL cases:

if type(result) == "table" then
    -- Module returned the table directly (return 1 from gmod13_open)
    tracy = result
    print("[Tracy] Loaded via require return value")
elseif type(tracy) == "table" then
    -- Module set _G.tracy via SPECIAL_GLOB
    print("[Tracy] Loaded via _G.tracy global")
else
    -- Neither worked — dump debug info
    print("[Tracy] ERROR: Module loaded but tracy table not found!")
    print(
        "[Tracy]   require returned: "
            .. type(result)
            .. " = "
            .. tostring(result)
    )
    print("[Tracy]   _G.tracy type:    " .. type(tracy))
    return
end

-- Verify the table has functions
if not tracy.FrameMark then
    print("[Tracy] ERROR: tracy table exists but FrameMark is missing!")
    return
end

print("[Tracy] Module ready! Functions available: " .. table.concat(
    (function()
        local keys = {}
        for k in pairs(tracy) do
            keys[#keys + 1] = k
        end
        table.sort(keys)
        return keys
    end)(),
    ", "
))

-- Frame mark every tick
hook.Add("Think", "TracyFrameMark", function()
    tracy.FrameMark()
end)
