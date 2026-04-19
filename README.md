# Craft The World Mod Loader (Lua Edition)

A powerful, native C++ DLL mod loader for the game **Craft The World**, enabling dynamic game modification via Lua scripts and providing a complete developer UI via ImGui.

> **Note:** This is a fun, hobby project! Updates will be provided when possible, but there is no strict schedule. Contributions, pull requests, and forks are absolutely welcome.

## Features

- **Lua Event Engine**: Bind logic directly to game events like `OnWorldLoaded` without tricky C++ hooks. 
- **Modern ImGui UI**: A completely custom-styled dark theme control panel to toggle and delete mods in real time.
- **Topological Sorting**: Dependency-based mod loading (specify dependencies in `manifest.lua` and it loads in the perfect order).
- **Hot Reloading**: Press `F6` to instantly reconstruct the Lua State and reload all loaded mods without restarting the game.
- **In-Game Developer Console**: Type specific commands, view real-time standard output and errors straight in the ImGui overlay.

## Installation

1. Build the project using **Visual Studio 2022** (Target: `x86` / `Release` mode).
2. Grab the generated `CTW_Moodloader.dll` from the `Release` folder.
3. Inject the DLL into the `CraftTheWorld.exe` process using your preferred injector (e.g. Extreme Injector, Process Hacker, etc.).
4. In the folder containing `CraftTheWorld.exe`, create a `mods` folder. (It will be created automatically upon first injection).

### Mod Structure
Mods belong in the `mods/` folder. Every mod requires a `manifest.lua`:

```lua
return {
    id = "my-awesome-mod",
    name = "My Awesome Mod",
    version = "1.0.0",
    author = "Your Name",
    description = "Grants 1000 wood on startup.",
    scripts = {
        "main.lua"
    },
    dependencies = {
        -- "core-api-mod"
    }
}
```

## Global Lua API
Inside your `.lua` scripts, you get native access to the `CTW` global namespace provided by the engine.

| Function | Argument Description | Behavior |
| -------- | ---------- | -------- |
| `CTW.log(msg)` | `string` | Print a message to the Developer Console |
| `CTW.addExp(amount)` | `int` | Adds flat experience to the player's progression |
| `CTW.giveResource(id, amount)` | `int, int` | Gives specific resource ID to the inventory |
| `CTW.getResourceCount(id)` | `int` | Returns current inventory count for resource |
| `CTW.spawnCreature(name, x, y)`| `string, int, int`| Spawns creature by string name at `x`, `y` |
| `CTW.setDay()` / `CTW.setNight()`| `none` | Skips cycle cleanly to Dawn/Dusk |
| `CTW.buildBlock(x, y, id, isFront)` | `int, int, int, bool` | Places block in world coordinates natively |

### Timers
- `CTW.setTimeout(callback, ms)`
- `CTW.setInterval(callback, ms)`
- `CTW.clearTimer(timerId)`

### Events
Global events are hooked up natively. Simply define these functions in your script!
```lua
function OnWorldLoaded()
    CTW.log("World loaded successfully!")
    CTW.giveResource(4, 100) -- Example: give 100 wood
end

function OnWorldClosed()
    CTW.log("World is unloading!")
end
```

## Hotkeys
- **INSERT** - Toggle the Modder Control Panel and Developer Console.
- **F6** - Reload all Lua scripts immediately (great for rapid development).
- **END** - Unload DLL safely and release DirectX hooks.

## Roadmap & Disclaimers

This was primarily developed as an internal exploration of the `classgame` reverse engineering process and creating functional Lua bridging for an aging C++ engine. Expect occasional quirks. I'll maintain and provide updates whenever time allows! Feel free to modify and adapt it for other legacy native games!

**Have fun modding!**
