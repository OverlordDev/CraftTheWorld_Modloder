-- block_all_recipes.lua
-- Stress test: block every recipe in the game

print("--- Bulk Blocking All Recipes ---")

local recipes = CTW.getRecipes()
local count = #recipes
print("Found " .. count .. " recipes to block.")

for i = 1, count do
    -- Set everything to false
    CTW.setRecipeAllow(i, false)
    
    -- Periodic logging for long lists
    if i % 100 == 0 then
        print("Progress: " .. i .. "/" .. count)
    end
end

-- Force UI to update once after all changes
print("Triggering UI Sync...")
CTW.syncRecipeUI()

print("--- Done! All recipes should be blocked. ---")
