# World Query (EQS)

An EQS-style spatial query system: generate candidate positions around a querier, score each one
0..1 with a stack of utilities, and get them back sorted best-first. Use it for "where should this
AI stand / strafe to / take cover".

```
ZWorldQuery3D
├── shapes:    [WorldQueryShape_Donut, …]      → candidate positions
└── utilities: [WorldQueryUtility_Distance, …] → score each candidate
run_query(querier) → WorldQueryResult (sorted, best first)
```

Scores from every utility are **multiplied**, so any utility returning `0` vetoes a position.

## ZWorldQuery3D (Node3D) / ZWorldQuery2D (Node2D)

Identical APIs; positions are always `Vector3` (2D uses `x`/`y` and leaves `z` at 0).

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `shapes` | `Array[WorldQueryShape]` | `[]` | Position generators; results are combined. |
| `utilities` | `Array[WorldQueryUtility]` | `[]` | Scoring filters. |
| `debug_draw` | `bool` | `false` | Draw scored points. Also live-previews in the editor. |
| `debug_follow_node` | `bool` | `false` | `false`: points stay where the last query ran. `true`: they move with this node. |
| `debug_sphere_radius` (3D) | `float` | `0.25` | 2D uses `debug_circle_radius`. |
| `debug_low_color` | `Color` | red | Score 0. |
| `debug_high_color` | `Color` | green | Score 1. |

```gdscript
query.run_query(querier: Node) -> WorldQueryResult
signal on_query_finish(result: WorldQueryResult)
```

The **querier** is the node the query is relative to — its position anchors the shapes and its
facing drives angle scoring. Pass the AI actor, or the player when you want positions around the
player.

## WorldQueryResult (Resource)

| Member | Notes |
| --- | --- |
| `ordered_positions: PackedVector3Array` | Sorted, highest score first. |
| `ordered_scores: PackedFloat32Array` | Parallel to `ordered_positions`. |
| `get_positions_over_score(score, invert := false)` | Filter to `>= score`, or `<= score` when inverted. |

## Shapes

Both generate on the movement plane (3D: XZ, 2D: XY), centered on the querier.

**`WorldQueryShape_Grid`** — flat axis-aligned grid (not rotated by facing).

| Property | Default |
| --- | --- |
| `grid_size: Vector2i` | `(5, 5)` |
| `cell_spacing: float` | `2.0` |

**`WorldQueryShape_Donut`** — concentric rings.

| Property | Default |
| --- | --- |
| `inner_radius: float` | `2.0` |
| `outer_radius: float` | `6.0` |
| `ring_count: int` | `2` |
| `points_per_ring: int` | `8` |

## Utilities

**`WorldQueryUtility_Distance`** — scores by distance from the querier.

| Property | Default | Notes |
| --- | --- | --- |
| `max_distance: float` | `20.0` | Beyond this scores 0. |
| `prefer_far: bool` | `false` | `false`: closer is better. `true`: farther is better. |

**`WorldQueryUtility_Angle`** — scores by the angle between the querier's facing and the direction
to the candidate. 0° = facing it directly = score 1.

| Property | Default |
| --- | --- |
| `max_angle_degrees: float` | `180.0` |

**`WorldQueryUtility_Trace`** — raycasts from the querier to the candidate. Blocked = 0, clear = 1.

| Property | Default |
| --- | --- |
| `collision_mask: int` | `1` |

## Custom shapes and utilities

```gdscript
extends WorldQueryShape
class_name MyRingShape

func _get_query_positions(querier: Node) -> PackedVector3Array:
    var out := PackedVector3Array()
    var origin: Vector3 = (querier as Node3D).global_position
    for i in 12:
        var a := TAU * i / 12.0
        out.append(origin + Vector3(cos(a), 0.0, sin(a)) * 5.0)
    return out
```

```gdscript
extends WorldQueryUtility
class_name AvoidHazards

func _score_position(querier: Node, position: Vector3) -> float:
    # Return 0..1. Returning 0 removes the position from consideration.
    return 0.0 if _is_in_lava(position) else 1.0
```

## Example: pick a strafe spot

```gdscript
@onready var query: ZWorldQuery3D = $ZWorldQuery3D

func pick_reposition_target() -> Vector3:
    var result := query.run_query(self)
    if result.ordered_positions.is_empty():
        return global_position
    return result.ordered_positions[0]
```

Turn on `debug_draw` while tuning: red spheres are low-scoring, green are high. The preview
updates live in the editor using the query node itself as the querier.
