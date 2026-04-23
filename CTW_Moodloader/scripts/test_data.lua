-- test_data.lua
-- Verification script for Craft The World Modloader Data API

local function log(msg)
    if CTW and CTW.log then
        CTW.log(tostring(msg))
    else
        print(msg)
    end
end

log("--- World Data Verification ---")

-- 1. Check Craft Resources
local resources = CTW.getCraftResources()
log("Total Craft Resources: " .. #resources)
if resources[1] then
    log("Example Resource [1]:")
    log("  Name: " .. (resources[1].name or "nil"))
    log("  Title: " .. (resources[1].title or "nil"))
    log("  Desc: " .. (resources[1].desc or "nil"))
end

-- 2. Check Block Types
local blocks = CTW.getBlockTypes()
log("Total Block Types: " .. #blocks)
if blocks[1] then
    log("Example Block [1]:")
    log("  Name: " .. (blocks[1].name or "nil"))
    log("  Extract: " .. (blocks[1].extract or "nil"))
end

-- 3. Check Recipes (Iterate 0 to 2000 as requested)
log("--- Scanning Recipes (0 to 2000) ---")
local recipes = CTW.getRecipes()
log("Found recipes in data: " .. #recipes)

local limit = math.min(#recipes, 2000)
local found_count = 0

for i = 1, limit do
    local r = recipes[i]
    if r then
        found_count = found_count + 1
        log(string.format("Recipe [%d]: %s (Allow: %s)", i, r.name or "???", tostring(r.allow)))
    end
end

log("Scan complete. Processed " .. found_count .. " recipes.")
log("--- Verification End ---")
