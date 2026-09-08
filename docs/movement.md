# Movement: ZMover, ZMoveMode, ZLagger3D, ZCameraFollower3D

## ZMover (Node)

Physical mover for `CharacterBody2D` and/or `CharacterBody3D`. Runs in `_physics_process`:
integrates a wish direction into velocity, optionally rotates the body toward it, applies
gravity, and calls `move_and_slide()`.

Add it as a **child of the character body** and it finds the body automatically.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `body_2d` | `CharacterBody2D` | — | Set manually, or auto-found from the parent. |
| `body_3d` | `CharacterBody3D` | — | Same; 3D takes priority when both would match. |
| `auto_find_body` | `bool` | `true` | Resolve the parent as the body if unset. |
| `move_mode` | `ZMoveMode` | auto | Locomotion tuning. A default is instantiated if null. |
| `velocity` | `Vector3` | `(0,0,0)` | Shared velocity. 2D uses `x`,`y`. |
| `apply_gravity` | `bool` | `true` | — |

### Driving it

**Re-call a `Move_By*` method every frame while input is held.** If you skip a frame, the wish
direction is cleared and the character decelerates. That's deliberate — it means "stop moving"
requires no explicit call.

```gdscript
mover.Move_ByInput(input: Vector2)
# World axes. 3D: input.x → +X, input.y → +Z. 2D: input → XY.

mover.Move_ByLook(input: Vector2, rotation: Vector3)
# Camera/look-relative. rotation is euler radians — e.g. ZPlayer.control_rotation.

mover.Move_ByLookDegrees(input: Vector2, rotation_degrees: Vector3)
mover.Move_ByLookQuaternion(input: Vector2, look: Quaternion)

mover.Move_ToPosition(position: Vector3)
# Convenience: heads straight at a world position. Also per-frame.

mover.Launch(dir: Vector3, scale: float,
             override_side_velocity := true,
             override_up_velocity := true,
             reset_gravity_curve := false)
# Impulse — jumps, knockback, launch pads.

mover.is_on_floor() -> bool
```

All of the above also exist in snake_case (`move_by_input`, `move_by_look`, `launch`, …).

### Typical usage

```gdscript
func _process(delta: float) -> void:
    var v := Input.get_vector("Move_Left", "Move_Right", "Move_Down", "Move_Up")
    mover.move_by_look(v, Z.players[0].control_rotation)

# Jump
mover.launch(Vector3.UP, 10.0)

# Stop cleanly when leaving a movement state
mover.move_by_input(Vector2.ZERO)
```

---

## ZMoveMode (Resource)

Locomotion tuning shared by movers. Make one `.tres` per movement style (walk, swim, sprint) and
swap `mover.move_mode` at runtime.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `move_speed` | `float` | `5.0` | Target horizontal speed. |
| `move_accel` | `float` | `20.0` | Units/s² toward the target velocity. |
| `move_decel` | `float` | `25.0` | Grounded deceleration with no input. |
| `air_control` | `float` | `1.0` | Multiplies `move_accel` while airborne (`0` = none). |
| `air_friction` | `float` | `0.0` | Replaces `move_decel` airborne. `0` preserves momentum. |
| `gravity_scale` | `float` | `1.0` | Multiplies project gravity. |
| `gravity_dir` | `Vector3` | `(0,-1,0)` | Also defines "up" for the whole mover. |
| `gravity_accel_curve` | `Curve` | — | Optional gravity multiplier sampled by elapsed fall time. |
| `rotate_with_movement` | `bool` | `false` | Rotate the body to face movement. |
| `velocity_rotation_rate` | `Vector3` | `(0,10,10)` | Max rad/s per euler axis. 3D uses Y (yaw); 2D uses Z. |
| `forward_adjust_rotation` | `bool` | `false` | `true` = tank-like; see below. |

### Rotation behaviour

When `rotate_with_movement` is on, the body turns toward the **input (wish) direction** at a flat
`velocity_rotation_rate` rad/s — the turn rate does not scale with how fast you are moving. This
matches Unreal's `bOrientRotationToMovement`. With no input the body keeps its current facing.

`forward_adjust_rotation` picks how velocity relates to that rotation:

- **`false` (strafe, default)** — velocity lerps directly toward the wish direction. The body
  turns to match, but movement is immediate and camera-relative.
- **`true` (tank-like)** — the body rotates first, then velocity is pushed along the body's
  current facing. The character has to turn before it can go that way, giving weightier,
  momentum-driven movement.

---

## ZLagger3D (Node3D)

Makes direct `Node3D` children trail behind this node's transform, like a soft spring arm. Useful
for camera booms and follow rigs. Children are made top-level, so their global transform is driven
here instead of inherited instantly.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `lag_position` | `bool` | `true` | — |
| `lag_rotation` | `bool` | `true` | — |
| `position_speed` | `float` | `10.0` | Catch-up rate (1/seconds). Higher snaps faster. |
| `rotation_speed` | `float` | `10.0` | Same, for rotation. |

## ZCameraFollower3D (Camera3D)

A `Camera3D` that keeps looking toward a target `Node3D` **only while it is the current
camera**. When another camera is active it does nothing (and does not fight other controllers).

On becoming current it captures a **starting rotation**. Desired orientation each frame is:

```
desired = slerp(start_rotation, look_at(target), target_lerp)
```

Current orientation then eases toward `desired` with `look_speed` (exponential smooth,
same idea as `ZLagger3D` / `Z_Math`).

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `target` | `Node3D` | — | World node to look toward. When `null` (or freed), eases back to the default/start rotation at `look_speed`. |
| `target_lerp` | `float` | `1.0` | `0`…`1`. Blend between starting rotation and full look-at. |
| `look_speed` | `float` | `1.0` | Catch-up rate (1/seconds) toward desired (look-at **or** back to start). `0` snaps every frame. |
| `snap_when_current` | `bool` | `true` | On becoming current, jump straight to desired rotation. |

```gdscript
# Make this the active camera; it will look at the player head.
cam.target = player_head
cam.target_lerp = 1.0
cam.look_speed = 4.0
cam.snap_when_current = true
cam.make_current()

# After a cinematic cut that changes facing, re-baseline the "start" rotation:
cam.capture_start_rotation()
```

`capture_start_rotation()` re-reads the camera's current global basis as the new start
(used automatically when the camera becomes current).
