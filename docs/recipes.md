# Recipes

Copy-paste answers to common tasks. Each one links to the page with the full story.

## Set up a new game from scratch

1. **Project Settings → zandot →** set `default_game_mode` to a `gm_*.tscn` whose root is a
   `ZGameMode`. This runs in any level that doesn't provide its own mode.
2. On that mode set `default_player_scene` (the pawn) and `default_state` (e.g. an explore state).
3. In each level, drop a `ZPlayerStart3D` where the pawn should appear.
4. Optionally add a `ZMasterScene` for fades.

`Z` resolves the mode and calls `setup()` automatically on every scene change.
→ [core.md](core.md)

## Get the player and their pawn

```gdscript
var player: ZPlayer = Z.players[0]
var pawn: Node3D = Z.Player_Get_Pawn() as Node3D
```

Don't hold long-lived references — cache on state enter, guard with `is_instance_valid()`, and
null them on state exit.
→ [input.md](input.md)

## Cut to a look-at camera

Use `ZCameraFollower3D` for a shot that tracks a node while active, then hand off cleanly:

```gdscript
$CutCam.target = boss
$CutCam.target_lerp = 1.0
$CutCam.look_speed = 6.0
$CutCam.snap_when_current = true
$CutCam.make_current()
# Later: another camera.make_current() — follower stops updating automatically.
```

→ [movement.md](movement.md#zcamerafollower3d-camera3d)

## World-space UI (nameplate / prompt)

1. Add a `ZCanvas3D` at the 3D anchor (e.g. above a character's head).
2. Add a `Control` or `Label` (or any `Node2D`) as a **child** of the `ZCanvas3D`.
3. Optionally enable distance scale / fade / screen clamp:

```gdscript
$NamePlate.center_content = true
$NamePlate.scale_by_distance = true
$NamePlate.distance_scale_reference = 10.0
$NamePlate.use_distance_fade = true
$NamePlate.distance_fade_begin = 20.0
$NamePlate.distance_fade_end = 10.0  # additional range after begin
$NamePlate.clamp_to_screen = true
$NamePlate.screen_clamp_margins = Rect2(16, 16, 16, 16)
```

→ [utilities.md](utilities.md#zcanvas3d-node3d)

## Build a selectable UI list

1. Make an option scene with a `ZDataOption` **Control** root **or child** (set `custom_minimum_size`; optionally wire `select_button` / labels / icon).
2. Place a `ZDataList` Control in the UI, set `option_scene`, layout (`list_layout` / `sections` / `is_scrollable`), and `player_index`.
3. Feed it data objects that implement `get_z_data()` (e.g. `ZResource` subclasses), or assign `default_options` in the inspector (editor shows a live preview):

```gdscript
list.option_add_list(inventory_items, true)
list.on_option_selected.connect(func(opt, i):
    print(Z.players[0].selected_data_option, opt.source_object)
)
# Manual label override when needed:
opt.override_title = "Special"
```

→ [utilities.md](utilities.md#zdatalist-control)

## Switch game state

```gdscript
Z.current_game_mode.change_state_scene(preload("res://states/gs_pause.tscn"))

# with a payload
Z.current_game_mode.change_state_scene(preload("res://states/gs_death.tscn"), {from_fall = true})

# from inside a state
get_game_mode().change_to_default_state()
```
→ [core.md](core.md#zgamestate-node)

## Fire a global event

`Z.gEvent` is a string + Dictionary pub/sub on the singleton. Listeners filter on the event name.
This is not `ZEvent` (menus).

```gdscript
func _ready() -> void:
    Z.on_gEvent.connect(_on_g_event)

func _on_g_event(event: String, meta: Dictionary) -> void:
    if event == "star_collected":
        print(meta.get("id"))

# anywhere
Z.gEvent("star_collected", {"id": 3})
```
→ [core.md](core.md#z-engine-singleton)

## Change level with a fade

```gdscript
Z.Scene_Transit(preload("res://levels/level_2.tscn"))
```

Fades come from `ZMasterScene._hook_play_scene_transit`. Watch progress with
`Z.on_scene_transit_state_change`.
→ [core.md](core.md#scene-transit)

## Make a door between two levels

In level A: add a `ZSceneTransit3D`, set `linked_scene` to level B and `link_id` to `&"north_door"`.
In level B: add a `ZPlayerStart3D` with `link_id = &"north_door"` and `linked_map` = level A.

Walking the pawn into the portal triggers it (`transit_on_player_enter` is true by default). Add a
`CollisionShape3D` child to control the trigger volume; otherwise a 1×2×1 box is used.
→ [core.md](core.md#zscenetransit2d--zscenetransit3d--portals)

## Swap a character model / skin

Add a `ZSkinModel` on the pawn, point `model_default` at the imported mesh scene instance and
`anim_tree` at the shared `AnimationTree`. Then:

```gdscript
skin.set_model_from_scene(preload("res://chars/sk_stella_winter.glb"))
# AnimationTree.anim_player is rewired to the new model's AnimationPlayer automatically.
```

For modular parts, pre-declare keys in `model_slots` and pass the slot name as the second arg.
→ [creatures.md](creatures.md#zskinmodel-node)

## Move a character

Child a `ZMover` under the `CharacterBody3D`, then feed it every frame:

```gdscript
func _process(delta: float) -> void:
    var v := Input.get_vector("Move_Left", "Move_Right", "Move_Down", "Move_Up")
    mover.move_by_look(v, Z.players[0].control_rotation)
```

Stop by simply not calling it (or `move_by_input(Vector2.ZERO)` to be explicit).
→ [movement.md](movement.md)

## Make the character turn to face movement

On its `ZMoveMode`: `rotate_with_movement = true`, and set `velocity_rotation_rate.y` (3D yaw) to
the turn rate in rad/s. Turn rate is flat — it does not scale with speed. Set
`forward_adjust_rotation = true` for tank-like movement where the body must turn before it moves.
→ [movement.md](movement.md#rotation-behaviour)

## Route input to the pawn

Give the receiving object an `input_triggered` method, then register it:

```gdscript
Z.players[0].input_target_add(pawn)
```

```gdscript
# on the pawn
func input_triggered(player: ZPlayer, action: StringName, state: int, dt: float) -> void:
    if action == &"Jump" and state == ZPlayer.PRESSED:
        $ZMover.launch(Vector3.UP, 10.0)

func input_is_enabled() -> bool:      # optional gate
    return not is_dead
```

Drive it from a `ZInputListener`: connect `on_input_begin/update/end` to
`player.input_begin/input_update/input_end`.
→ [input.md](input.md#input-routing)

## Add a jump ability

Child a `ZCreature` under the pawn, then a `ZAbility` under that with `input_action = "Jump"`:

```gdscript
extends ZAbility

@export var jump_speed := 10.0

func _ability_can_activate(context) -> bool:
    return _mover() != null and _mover().is_on_floor()

func _on_ability_activated(context) -> void:
    _mover().launch(Vector3.UP, jump_speed)
    ability_stop(false)

func _mover() -> ZMover:
    return get_root_owner().get_node_or_null("ZMover") as ZMover
```

The creature polls input itself — no wiring from the game state needed.
→ [creatures.md](creatures.md)

## Give an enemy utility AI

On its `ZCreature` set `autorun_utility = true`. In each ability, override `_get_utility_score`
and return > 0 when the ability is worth doing. Highest score wins; running abilities are never
interrupted.
→ [creatures.md](creatures.md#utility-ai)

## Save and load

```gdscript
Z.save_game = Z.SaveGame_Create()          # new game
Z.SaveGame_Save_ToSlot(Z.save_game, 0)     # writes .sav + screenshot

var s := Z.SaveGame_Load_FromSlot(0)       # null if the slot is empty
if s != null:
    Z.save_game = s
```

Store custom fields in `save.params`, or subclass `ZSaveGame` and point
`zandot/saves/savegame_script` at the script file.
→ [saves.md](saves.md)

## Play a dialogue graph

```gdscript
flow_player.flow_graph = load("res://dialog/d_intro.tres")
flow_player.flow_end.connect(_on_done, CONNECT_ONE_SHOT)
flow_player.play()
```

Message nodes stay **active** after `_on_enter` (no `trigger_output` yet); your UI calls
`node.trigger_output(0)` (or a choice index) to continue. Use `nodes_get_active()` /
`nodes_get_recorded()` for multi-node state.
→ [flow.md](flow.md)

## Add a custom dialogue node type

`@tool` script extending `ZFlowNode`, override `_get_caption`, `_get_category`, `_define`, and the
runtime hooks (`_on_enter(pin, player, previous_node, previous_out_pin)`, `_on_exit`). Continue the
graph with `trigger_output`. The `@tool` annotation is required.
→ [flow.md](flow.md#writing-a-custom-node-type)

## Ask "where should the AI go?"

Add a `ZWorldQuery3D`, fill `shapes` (e.g. a Donut) and `utilities` (Distance + Trace), then:

```gdscript
var result := query.run_query(self)
var best := result.ordered_positions[0]
```

Turn on `debug_draw` to see the scored points (red = low, green = high) live in the editor.
→ [worldquery.md](worldquery.md)

## Smooth a value without framerate bugs

```gdscript
cam.global_position = Z_Math.V3_Interp(cam.global_position, target, delta, 8.0)
```
→ [utilities.md](utilities.md#z_math-refcounted-static)

## Convert a dictionary to CSV / YAML / INI / TOML

```gdscript
var rows := [{name = "Ada", hp = 10}, {name = "Bob", hp = 8}]
var csv := Z_Parse.to_CSV(rows)
var back := Z_Parse.from_CSV(csv)   # Array[Dictionary]

var yaml := Z_Parse.to_YAML({player = {name = "Ada", hp = 10}})
var data := Z_Parse.from_YAML(yaml)
```

Also `to_INI` / `from_INI` and `to_TOML` / `from_TOML`.
→ [utilities.md](utilities.md#z_parse-refcounted-static)

## Play a one-shot sound with no node to manage

```gdscript
Z_Sound.Oneshot(preload("res://sfx/hit.ogg"), -6.0)
```
→ [utilities.md](utilities.md#z_sound-refcounted-static)

## Keep debug content out of the shipped game

Put it under a `ZDebugRoot3D` (or `2D`/`UI`). It's visible while editing, disabled at runtime, and
freed entirely in exports unless the game is launched with `--z-debug`.
→ [utilities.md](utilities.md#zdebugroot2d--zdebugroot3d--zdebugrootui)

## Data-drive from a resource's filename

```gdscript
# res://data/enemy_goblin_3.tres
var kind := Z_Obj.gNameVar_string(res, 1)   # "goblin"
var tier := Z_Obj.gNameVar_int(res, 2)      # 3
```
→ [utilities.md](utilities.md#z_obj-refcounted-static--namevars)

## Rebuild after changing C++

```bash
scons platform=windows target=template_debug -j8
copy_to_project.bat
```

Close the Godot editor first if the DLL fails to overwrite, and restart it after copying.
→ [building.md](building.md)
