# Saves

Two save resources, both `Resource` subclasses serialized with Godot's normal resource writer:

- **`ZSaveGame`** — one per playthrough (slot saves).
- **`ZSaveGlobal`** — one meta save for the whole install (settings, unlocks, records). Created on
  first launch and reloaded every boot; you save it yourself, typically on quit.

Both extend **`ZSave`**.

## ZSave (base)

| Property | Type | Notes |
| --- | --- | --- |
| `params` | `Dictionary` | Free-form bag for your own data. |
| `date_created` | `String` | — |
| `date_last_saved` | `String` | — |
| `save_seed` | `int` | Random seed stamped at creation. |
| `creature_data` | `Dictionary[StringName, ZCreatureData]` | Persisted creature stats by id. See [creatures.md](creatures.md#zcreaturedata-resource). |

```gdscript
save.initialize_new()   # stamp dates + seed for a brand-new save
save.touch_saved()      # stamp date_last_saved (called for you before writing)

save.creature_data_get(id: StringName) -> ZCreatureData   # null when absent
save.creature_data_set(id: StringName, data: ZCreatureData)   # null data erases the entry
```

PascalCase aliases: `CreatureData_Get`, `CreatureData_Set`.

## ZSaveGame

| Property | Type | Notes |
| --- | --- | --- |
| `current_scene` | `String` | — |
| `current_transform` | `Transform3D` | — |
| `bgm_track` | `ZBgmTrack` | Music state, mirrored by `Z_BGM`. See [audio.md](audio.md#saves). |
| `bgm_variant` | `StringName` | — |
| `bgm_locked` | `bool` | — |

## API on Z

```gdscript
# Game saves
Z.SaveGame_Create() -> ZSaveGame
Z.SaveGame_Load_FromPath(path: String) -> ZSaveGame
Z.SaveGame_Load_FromSlot(slot: int) -> ZSaveGame
Z.SaveGame_Save_ToPath(save: ZSaveGame, path: String) -> bool
Z.SaveGame_Save_ToSlot(save: ZSaveGame, slot: int) -> bool

# Slot paths — for save/load UI (existence checks, thumbnails, deletion)
Z.save_game_slot_dir(slot: int) -> String
Z.save_game_slot_path(slot: int) -> String
Z.save_game_screenshot_path(slot: int) -> String

# Global save
Z.SaveGlobal_Create() -> ZSaveGlobal
Z.SaveGlobal_Load() -> ZSaveGlobal
Z.SaveGlobal_Save(save := null) -> bool     # defaults to Z.save_global
```

`SaveGame_Save_ToSlot` also captures a **screenshot** next to the `.tres` file, so save-slot UI can
show a thumbnail without extra work.

## Where files go

Controlled by `Z.save_location`:

| Value | Root |
| --- | --- |
| `Z.SAVE_LOCATION_USER` (default) | `{USERPROFILE}/Saved Games/Zandot/{application/config/name}/` — falls back to `user://` if unresolvable. |
| `Z.SAVE_LOCATION_GAMEDIR` | `{executable_dir}/save/` |

Layout inside the root, with `zandot/saves/save_game_prefix = "save_"`:

```
save_000/
├── save_000.tres
└── save_000.png
save_001/
├── save_001.tres
└── save_001.png
global.tres          ← name from zandot/saves/save_global_name
```

## Custom save classes

To add your own fields, subclass in GDScript and point the project settings at the script **file**:

```gdscript
# res://scripts/saves/my_save_game.gd
extends ZSaveGame
class_name MySaveGame

@export var chapter: int = 0
@export var flags: Dictionary = {}
```

Set `zandot/saves/savegame_script` to `res://scripts/saves/my_save_game.gd` (and
`zandot/saves/saveglobal_script` for the global equivalent) — both settings are file pickers, so
browse to the `.gd`/`.cs` rather than typing a class name. `Z.SaveGame_Create()` then attaches that
script, so `Z.save_game as MySaveGame` is valid everywhere.

If the script fails to load, or does not extend the right base, `Z` warns and falls back to a plain
`ZSaveGame` / `ZSaveGlobal`.

## Typical flow

```gdscript
# New game
Z.save_game = Z.SaveGame_Create()
Z.Scene_Transit(preload("res://levels/level_1.tscn"))

# Save to slot 0
Z.save_game.current_scene = get_tree().current_scene.scene_file_path
Z.save_game.current_transform = Z.Player_Get_Pawn().global_transform
Z.SaveGame_Save_ToSlot(Z.save_game, 0)

# Continue
var save := Z.SaveGame_Load_FromSlot(0)
if save != null:
    Z.save_game = save
    Z.Scene_Transit(load(save.current_scene))

# Settings on quit
Z.SaveGlobal_Save()
```
