# Core: Z, Game Modes, Game States, Scene Transit

## Z (Engine singleton)

`Z` is an `Object` registered as an Engine singleton when the extension loads. It is available
everywhere in GDScript as `Z` — no autoload, no `get_node`.

Z owns: the save files, the list of players, the current game mode, the master scene, and the
scene-transit state machine.

### Boot order

1. Extension loads → `Z` singleton created, project settings registered.
2. `Z.bootstrap()` runs once the main loop exists (runtime only):
   - loads or creates the global save,
   - hooks `SceneTree.scene_changed`,
   - resolves the game mode for the current scene.
3. On every scene change, `Z` re-resolves the game mode and calls `setup()` on it.

### Properties

| Property | Type | Notes |
| --- | --- | --- |
| `save_game` | `ZSaveGame` | Current playthrough save. See [saves.md](saves.md). |
| `save_global` | `ZSaveGlobal` | Meta save (settings, unlocks). |
| `save_location` | `Z.SaveLocation` | `SAVE_LOCATION_USER` (default) or `SAVE_LOCATION_GAMEDIR`. |
| `players` | `Array[ZPlayer]` | At least one per game instance. |
| `current_game_mode` | `ZGameMode` | Mode for the current root scene. Read-only in practice. |
| `master_scene` | `ZMasterScene` | Optional global scene providing fades. |

### Methods

```gdscript
# Players
Z.Player_Create() -> ZPlayer                 # appends to Z.players
Z.Player_Remove(player: ZPlayer)
Z.Player_RemoveIndex(player_index: int)
Z.Player_Get_Pawn(player_index := 0) -> Node # possessed pawn, or null

# Levels
Z.on_level_start()                           # find/spawn the ZGameMode and call setup()
Z.bootstrap()                                # normally automatic

# Scene changes (see "Scene transit" below)
Z.Scene_Transit(new_scene: PackedScene, skip_fade_out := false, skip_fade_in := false)
Z.hook_play_scene_transit(fade_out: bool) -> Signal
Z.get_scene_transit_state() -> int
Z.is_scene_transiting() -> bool

# Global conditions / scripts (see utilities.md)
Z.global_condition_check(condition: ZGlobalCondition) -> bool
Z.global_condition_check_list(conditions: Array[ZGlobalCondition], if_one_true := false) -> bool
Z.global_script_run(gscript: ZGlobalScript)
Z.global_script_run_list(gscripts: Array[ZGlobalScript])

# Global events (string + dict pub/sub; not ZEvent menus)
Z.gEvent(event: String, meta := {})
# alias: Z.g_event(event, meta)

# Saves — see saves.md for the full list
Z.SaveGame_Create() / SaveGame_Load_FromSlot(slot) / SaveGame_Save_ToSlot(save, slot) / …
```

### Signals

```gdscript
Z.on_game_mode_ready(game_mode: ZGameMode)
Z.on_scene_transit_state_change(state: int)   # Z.SCENE_TRANSIT_*
Z.on_gEvent(event: String, meta: Dictionary)  # fired by Z.gEvent
```

---

## ZGameMode (Node)

One per level / root scene. Joins the group `z_game_mode`. Usually the root of a `gm_*.tscn`
scene, or a child placed in the level.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `default_player_scene` | `PackedScene` | — | Pawn spawned for each `ZPlayer`. |
| `default_state` | `PackedScene` | — | State entered when none is active. |
| `starting_state` | `PackedScene` | — | State for the **first** `setup()`. Falls back to `zandot/general/default_starting_state`, then `default_state`. |
| `player_spawn_delay` | `float` | `0.05` | Seconds after setup before `spawn_players()`. |
| `active_state` | `ZGameState` | — | Read-only at runtime. |

```gdscript
mode.setup()                                       # called by Z on level load
mode.change_state_scene(scene: PackedScene, context := null)
mode.change_state(state: ZGameState, context := null)   # activate an existing instance
mode.change_to_default_state(context := null)
mode.change_to_starting_state(context := null)
mode.spawn_players()

signal on_state_change(new_state: ZGameState, old_state: ZGameState, context)
```

**Mode resolution.** On level load, if the scene contains a `ZGameMode` it is used (a warning
is emitted if there is more than one). Otherwise the project setting `zandot/general/default_game_mode`
is instanced.

**Auto-revert.** If the active state leaves the tree or is freed outside of `change_state*`
(e.g. it calls `queue_free()` on itself), the mode automatically reverts to `default_state`.

---

## ZGameState (Node)

A state is a whole scene representing one "mode of play": explore, combat, pause, dialogue,
menus. The mode instances it as its own child and swaps it out.

```gdscript
extends ZGameState

func _on_state_enter(context) -> void:
    # Setup: capture the mouse, cache the pawn, connect signals.
    pass

func _on_state_exit() -> void:
    # Undo everything _on_state_enter did. Always symmetric.
    pass
```

| Member | Notes |
| --- | --- |
| `context` | Arbitrary payload from `change_state*`. Set **before** `_ready`, so it is readable there too. |
| `get_game_mode() -> ZGameMode` | The owning mode. |
| `state_enter(context)` / `state_exit()` | Called by the mode; you override `_on_state_enter` / `_on_state_exit` instead. |

Use `_on_state_enter` / `_on_state_exit` for state logic and keep `_ready` for one-time scene
setup. Anything you change globally (mouse mode, `get_tree().paused`, cached references) must be
reverted on exit.

---

## ZMasterScene (Node)

Optional global/root scene providing cross-level services — mainly screen fades. When
`autoset_as_master_scene` is true (default) it registers itself as `Z.master_scene` on entering
the tree.

```gdscript
extends ZMasterScene

@export var control: Control
@export var anim_fades: AnimationPlayer

func _ready() -> void:
    control.visible = false

# Return a Signal that fires when the fade finishes.
# Return an invalid Signal (or don't override) to skip fading.
func _hook_play_scene_transit(fade_out: bool) -> Signal:
    control.visible = true
    if fade_out:
        anim_fades.play("fade")
    else:
        anim_fades.play_backwards("fade")
    return anim_fades.animation_finished
```

The returned signal may carry any arguments — `Z` discards them. `AnimationPlayer.animation_finished`
(one arg), `Tween.finished` (none), and your own custom signals all work.

**The signal must actually fire, exactly once, per hook call.** `Z` waits on it and will not advance
until it emits. Two things to watch:

- Don't return a signal that has already fired, or one from an animation you didn't start.
- `play_backwards()` on an `AnimationPlayer` does emit `animation_finished` when it reaches the
  start, so the fade-in leg is fine.

If the hook returns an invalid `Signal` (the default when you don't override it, or when there is
no master scene), that fade step is skipped and the transit continues immediately.

---

## Scene transit

`Z.Scene_Transit()` is an async scene change with optional fades:

```
NONE → FADE_OUT → TRANSITING → FADE_IN → NONE
```

Each step emits `Z.on_scene_transit_state_change(state)` with one of
`Z.SCENE_TRANSIT_NONE / FADE_OUT / TRANSITING / FADE_IN`. Fades come from
`master_scene.hook_play_scene_transit()`; if there is no master scene, the fade steps are skipped.

While a transit is in progress and `zandot/general/block_default_state_during_scene_transit` is true
(the default), the new level's game mode **defers** entering its starting state until fade-in
finishes — so you don't see one frame of gameplay behind the fade.

### ZSceneTransit2D / ZSceneTransit3D — portals

`ZSceneTransit3D` is an **Area3D**, `ZSceneTransit2D` is an **Area2D**. Overlapping the possessed
player pawn triggers the transit.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `linked_scene` | `PackedScene` | — | Destination scene. Required. |
| `link_id` | `StringName` | `&""` | Pairs with the destination `ZPlayerStart3D.link_id`. |
| `transit_on_player_enter` | `bool` | `true` | Fire automatically when a possessed pawn overlaps. |

```gdscript
portal.transit(skip_fade_out := false, skip_fade_in := false)   # manual trigger
```

Notes:
- If you don't add a `CollisionShape3D`/`CollisionShape2D` child, a default shape matching the
  editor gizmo is created at runtime (3D: 1×2×1 box, 2D: 64×96 rect). Add your own child shape
  to control the trigger volume.
- "Player pawn" means the node possessed by any `ZPlayer` in `Z.players`, or any descendant of it.
- The portal only changes scenes; it never spawns the pawn. It records the origin scene and
  `link_id` on `Z` so the destination `ZPlayerStart3D` can place the pawn.
- Auto-trigger is ignored while another transit is already running.

### ZPlayerStart3D (Marker3D)

Spawn point for a player pawn. Joins the group `z_player_start`.

| Property | Type | Notes |
| --- | --- | --- |
| `player_index` | `int` | Which `Z.players[i]` spawns here. |
| `linked_map` | `PackedScene` | Preferred start when arriving from that map via transit. |
| `link_id` | `StringName` | Pairs with `ZSceneTransit.link_id`. |

Editor-only visuals: a cylinder, a forward arrow, and a "Player Start" label.

**Two-way door recipe:** give the portal in map A and the start in map B the same `link_id`,
and set the start's `linked_map` to map A. Repeat mirrored in map B.

---

## Project settings

`Project Settings → search "zandot"`.

| Setting | Default | Meaning |
| --- | --- | --- |
| `zandot/general/default_game_mode` | `""` | PackedScene path used when a level has no `ZGameMode`. |
| `zandot/general/default_starting_state` | `""` | Fallback for `ZGameMode.starting_state`. |
| `zandot/general/block_default_state_during_scene_transit` | `true` | Defer starting state until fade-in ends. |
| `zandot/saves/save_game_prefix` | `"save_"` | Slot folder/file prefix. |
| `zandot/saves/save_global_name` | `"global"` | Global save filename. |
| `zandot/saves/savegame_script` | `""` | Script file attached to newly created `ZSaveGame`s — pick it with the file browser. |
| `zandot/saves/saveglobal_script` | `""` | Same for `ZSaveGlobal`. |
| `zandot/audio/bgm_bus` | `"Master"` | Audio bus used by `Z_BGM`. See [audio.md](audio.md). |
| `zandot/menus/menu_confirm` | `""` | Scene opened by `ZEvent.Confirm`; root must be a `ZMenu_Confirm`. See [menus.md](menus.md). |
| `zandot/menus/menu_info` | `""` | Scene opened by `ZEvent.Info`; root must be a `ZMenu_Info`. |
| `zandot/menus/menu_process` | `""` | Scene opened by `ZEvent.Process`; root must be a `ZMenu_Process`. |
| `zandot/menus/menu_choice` | `""` | Scene opened by `ZEvent.Choice`; root must be a `ZMenu_Choice`. |
| `zandot/input/register_default_actions` | `true` | Add the missing `z_*` InputMap actions `ZInputCommon` defaults to. See [input.md](input.md). |
| `zandot/performance/cache_object_name_vars` | `true` | Cache `Z_Obj.getNameVars` results on `Z`. |

Projects written against the older flat keys (`zandot/general/default_game_mode`,
`zandot/savegame_script_class`, …) are migrated automatically on load; in the editor the moved
values are written back to `project.godot` and the old keys removed.
