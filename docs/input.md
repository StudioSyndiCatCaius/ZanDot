# Players & Input

## ZPlayer (Resource)

A `ZPlayer` represents one human player — not a pawn. It holds which pawn is possessed, the
camera/aim rotation, and the list of objects that receive this player's input.

There is always at least one; they live in `Z.players` and are created with `Z.Player_Create()`.

| Property | Type | Notes |
| --- | --- | --- |
| `player_index` | `int` | Index in `Z.players`; kept in sync by `Z`. |
| `control_rotation` | `Vector3` | Camera/aim euler in **radians** (X pitch, Y yaw, Z roll). |
| `possessed_scene` | `Node` | The pawn. Runtime-only, held weakly (not serialized). |
| `input_targets` | `Array` | Objects receiving input. Runtime-only, held weakly. |
| `selected_data_option` | `ZDataOption` | At most one selected UI option per player. Runtime-only, weak. |

```gdscript
var player: ZPlayer = Z.players[0]
player.possess(pawn)               # or: player.possessed_scene = pawn
player.unpossess()
var pawn := player.get_possessed_scene()      # == Z.Player_Get_Pawn(0)

# UI lists write here when the player picks a ZDataOption:
player.selected_data_option        # or clear_selected_data_option()
```

---

## Input routing

Input is **not** delivered directly to the pawn. A `ZPlayer` fans each action out to every
object in its `input_targets` list. That lets a pawn, a camera rig, and a HUD all listen to the
same player without knowing about each other.

### The target contract

An object qualifies as an input target if it has this method:

```gdscript
func input_triggered(player: ZPlayer, action: StringName, state: int, dt: float) -> void:
```

`state` is a `ZPlayer.EInputState`:

| Value | When |
| --- | --- |
| `ZPlayer.NONE` | Unused sentinel. |
| `ZPlayer.PRESSED` | First frame the action went down. |
| `ZPlayer.DOWN` | Every frame while held (`dt` = frame delta). |
| `ZPlayer.RELEASED` | The frame the action came up. |

Optionally, a target can gate itself:

```gdscript
func input_is_enabled() -> bool:
    return visible and not is_stunned
```

If `input_is_enabled()` returns `false` the target is skipped for that dispatch. If the method
doesn't exist at all, the target always processes input.

### Managing targets

```gdscript
player.input_target_add(obj) -> bool     # false if obj lacks input_triggered() (pushes a warning)
player.input_target_remove(obj)
player.input_targets_clear()
player.has_input_target(obj) -> bool
player.input_targets                     # Array of live targets (dead ones are dropped)
```

Targets are held **weakly** — freeing a node removes it from the list automatically. `ZPlayer`
does not keep targets alive, so they must be owned by the scene tree (or something else).

### Dispatching

```gdscript
player.input_action(action: StringName, state: int, dt := 0.0)

# Convenience wrappers, ready to connect to ZInputListener's signals:
player.input_begin(action)            # → PRESSED
player.input_update(action, dt)       # → DOWN
player.input_end(action)              # → RELEASED
```

### Full example

```gdscript
# --- The driver (usually a game state) -------------------------------------
extends ZGameState

@onready var listener: ZInputListener = $ZInputListener
var _player: ZPlayer

func _on_state_enter(context) -> void:
    _player = Z.players[0]
    listener.on_input_begin.connect(_player.input_begin)
    listener.on_input_update.connect(_player.input_update)
    listener.on_input_end.connect(_player.input_end)
    _player.input_target_add(Z.Player_Get_Pawn())

func _on_state_exit() -> void:
    listener.on_input_begin.disconnect(_player.input_begin)
    listener.on_input_update.disconnect(_player.input_update)
    listener.on_input_end.disconnect(_player.input_end)
    _player.input_targets_clear()

# --- A target (the pawn) ----------------------------------------------------
extends CharacterBody3D

@onready var mover: ZMover = $ZMover

func input_is_enabled() -> bool:
    return not is_dead

func input_triggered(player: ZPlayer, action: StringName, state: int, dt: float) -> void:
    match action:
        &"Jump":
            if state == ZPlayer.PRESSED:
                mover.launch(Vector3.UP, 10.0)
        &"Sprint":
            is_sprinting = state == ZPlayer.PRESSED or state == ZPlayer.DOWN
```

---

## ZInputListener (Node)

Polls a list of InputMap actions every frame and emits begin/update/end signals. It is the usual
source that feeds `ZPlayer.input_begin/update/end`, but it is a standalone node — you can connect
it to anything.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `actions` | `PackedStringArray` | `[]` | InputMap action names to track. |
| `visibility_root` | `NodePath` | — | When set, input only fires while that node is visible in the tree. |
| `process_unhandled` | `bool` | `true` | Reserved; polling happens in `_process`. |

```gdscript
signal on_input_begin(action: String)
signal on_input_update(action: String, dt: float)
signal on_input_end(action: String)

listener.poll_input(delta)   # manual poll; also called automatically each frame
```

When the visibility gate turns off, every currently-held action fires `on_input_end` so nothing
gets stuck down.

**Note:** analog movement is usually polled directly rather than routed through actions, because
`ZMover` needs its wish direction re-supplied every frame. Godot's forward is `-Z`, so the "up"
action is the **negative** y argument:

```gdscript
func _process(delta: float) -> void:
    var v := Input.get_vector("Move_Left", "Move_Right", "Move_Up", "Move_Down")
    mover.move_by_look(v, Z.players[0].control_rotation)
```

---

## ZInputCommon (Node)

One node with plain signals for the input every game needs. Where `ZInputListener` is generic
(you give it action names, it gives you begin/update/end), `ZInputCommon` is opinionated: it knows
what confirm, cancel, shoulders, triggers, move, look and menu nav are, and gives each its own
signal.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `visibility_dependency` | `Node` | — | When set, input only fires while that node is visible in the tree. |
| `is_input_target` | `bool` | `false` | Register with `Z.players[player_index]` instead of polling actions directly. |
| `player_index` | `int` | `0` | Which `ZPlayer` to register with. |
| `use_mouse_look` | `bool` | `true` | Feed mouse motion into `input_rotate`. Only while the mouse is **captured**. |
| `mouse_look_sensitivity` | `float` | `0.12` | Look axis per pixel of mouse motion, before the frame delta. |
| `action_*` | `StringName` | see below | 22 rebindable action names. |

```gdscript
signal input_confirm()
signal input_cancel()
signal input_face_top()                              # Y on an Xbox pad
signal input_face_left()                             # X on an Xbox pad
signal special_L()                                   # L3
signal special_R()                                   # R3
signal shoulder(index: int)                          # -1 = L, 1 = R
signal input_trigger(index: int)                     # -1 = L, 1 = R (press edge)
signal input_trigger_update(index: int, dt: float)   # every frame while held
signal input_move(axis: Vector3, dt: float)          # left stick / WASD; (x, y, 0)
signal input_rotate(axis: Vector3, dt: float)        # right stick + mouse; (pitch, yaw, 0)
signal input_nav(axis: Vector2i)                     # dpad / arrows, one press edge per direction
```

Button signals fire on the **press edge** only. `input_move` / `input_rotate` fire every frame while
non-zero, plus one final zero-vector emit on release, so a listener can tell when input stopped.

`input_move.axis` matches `ZMover.move_by_input` — `Vector2(axis.x, axis.y)` feeds it directly, with
"up" as negative y. `input_rotate.axis` matches `ZPlayer.control_rotation` — X pitch, Y yaw, radians.
`input_nav` uses screen convention (`+y` is down), so a list index can just add `axis.y`.

### Driving it

By default the node polls the InputMap itself every frame. Set `is_input_target` and it registers
with `Z.players[player_index]` instead, so button edges arrive through that player's dispatch — a
pause menu can then take input away from the pawn just by managing the player's target list. The
analog axes are still polled either way, since a stick has no press/release edge.

### Bindings

Every binding is an exported action name. Confirm and cancel default to Godot's own `ui_select` /
`ui_cancel`; the rest default to `z_*` actions ZanDot adds to the InputMap on load when they don't
already exist:

| Actions | Default binding |
| --- | --- |
| `z_move_up` / `_down` / `_left` / `_right` | WASD + left stick |
| `z_look_up` / `_down` / `_left` / `_right` | Right stick (mouse handled separately) |
| `z_nav_up` / `_down` / `_left` / `_right` | Arrows + dpad |
| `z_face_top` / `z_face_left` | R / F + pad Y / X |
| `z_special_l` / `z_special_r` | Z / C + L3 / R3 |
| `z_shoulder_l` / `z_shoulder_r` | Q / E + LB / RB |
| `z_trigger_l` / `z_trigger_r` | Right / left mouse + LT / RT |

Existing actions are never touched, so defining `z_move_up` yourself keeps your events. In the editor
the defaults are also written into **Project Settings → Input Map**, where they can be re-bound
normally; set `zandot/input/register_default_actions` to `false` to skip registration entirely.

```gdscript
@onready var input: ZInputCommon = $ZInputCommon

func _ready() -> void:
    input.input_confirm.connect(_on_confirm)
    input.input_nav.connect(func(axis: Vector2i): _move_cursor(axis.y))
    input.input_move.connect(func(axis: Vector3, dt: float):
        mover.move_by_look(Vector2(axis.x, axis.y), Z.players[0].control_rotation))
```

Action names are rebindable from script through the `EAction` enum:

```gdscript
input.set_action(ZInputCommon.ACTION_CONFIRM, &"ui_accept")
```
