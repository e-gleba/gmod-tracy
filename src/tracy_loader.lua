-- tracy_loader.lua
-- Auto-generated install places this in the correct autorun subfolder.

if tracy then
    print("[Tracy] Already loaded in this process, skipping duplicate require")
    return
end

local ok, err = pcall(require, "tracy")
if ok then
    print("[Tracy] Module loaded successfully!")

    -- Frame mark every tick
    hook.Add("Think", "TracyFrameMark", function()
        tracy.FrameMark()
    end)
else
    print("[Tracy] Failed to load binary module: " .. tostring(err))
end