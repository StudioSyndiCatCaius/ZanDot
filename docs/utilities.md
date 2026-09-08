# Utilities

Small helper classes that don't belong to a bigger system.

## Z_Math (RefCounted, static)

Framerate-independent interpolation helpers. All take the same shape:

```gdscript
Z_Math.F_Interp(current, to, dt, speed := 1.0, const_ := false) -> float
Z_Math.V2_Interp(current: Vector2, …) -> Vector2
Z_Math.V3_Interp(current: Vector3, …) -> Vector3
Z_Math.T2_Interp(current: Transform2D, …) -> Transform2D
Z_Math.T3_Interp(current: Transform3D, …) -> Transform3D
```

- `const_ = false` (default): exponential smoothing; `speed` is the response rate. Framerate
  independent — unlike a raw `lerp(a, b, 0.1)`.
- `const_ = true`: move toward the target at a constant `speed` units/second.

snake_case aliases: `f_interp`, `v2_interp`, `v3_interp`, `t2_interp`, `t3_interp`.

```gdscript
camera.global_position = Z_Math.V3_Interp(camera.global_position, target, delta, 8.0)
```

## Z_Obj (RefCounted, static) — NameVars

Parses structured data out of object/resource **names**. Handy for data-driven assets where the
filename encodes properties: `enemy_goblin_3_true.tres`.

```gdscript
Z_Obj.getNameVars(obj: Object, delimiter := "_", force_recache := false) -> PackedStringArray
Z_Obj.gNameVar_string(obj, nvar: int, default := "") -> String
Z_Obj.gNameVar_bool(obj, nvar: int, true_str := "true", default := false) -> bool
Z_Obj.gNameVar_int(obj, nvar: int, default := 0) -> int
Z_Obj.resolve_object_name(obj) -> String   # Resource path basename, Node name, …
```

Index is 0-based; out-of-range returns the default. snake_case aliases: `get_name_vars`,
`g_name_var_string`, `g_name_var_bool`, `g_name_var_int`.

Results are cached on `Z` by instance id when `zandot/performance/cache_object_name_vars` is true (default).
Clear with `Z.object_name_vars_cache_clear()`.

```gdscript
# "enemy_goblin_3_true"
var kind := Z_Obj.gNameVar_string(res, 1)    # "goblin"
var tier := Z_Obj.gNameVar_int(res, 2)       # 3
var elite := Z_Obj.gNameVar_bool(res, 3)     # true
```

## Z_Parse (RefCounted, static)

Convert a `Dictionary` (or, for CSV, an `Array[Dictionary]`) to and from common text formats.

```gdscript
Z_Parse.to_CSV(data) -> String     # Dictionary or Array[Dictionary]
Z_Parse.from_CSV(str) -> Array[Dictionary]

Z_Parse.to_YAML(data: Dictionary) -> String
Z_Parse.from_YAML(str) -> Dictionary

Z_Parse.to_INI(data: Dictionary) -> String
Z_Parse.from_INI(str) -> Dictionary

Z_Parse.to_TOML(data: Dictionary) -> String
Z_Parse.from_TOML(str) -> Dictionary
```

snake_case aliases: `to_csv` / `from_csv` / `to_yaml` / `from_yaml` / `to_ini` / `from_ini` / `to_toml` / `from_toml`.

**CSV.** First row is the header. Each following row becomes a dictionary. A single `Dictionary` is written as one data row. Nested `Array`/`Dictionary` cells are JSON. Cells are typed on read (`true`/`false`/`null`, ints, floats, JSON objects).

**YAML.** Block maps and sequences, flow `[…]`/`{…}`, quoted scalars, `|` / `>` blocks, comments, `---`. No anchors, tags, or merge keys. A document whose root is not a mapping is returned as `{ "_": value }`.

**INI.** Nested dictionaries become `[dotted.sections]`. Root scalars sit above any section. Arrays/deeper values are JSON. `#` and `;` start comments.

**TOML.** Nested dictionaries become `[tables]`; arrays of dictionaries become `[[array tables]]`. Supports dotted keys, basic/literal/multi-line strings, arrays, inline tables, comments. Datetimes are read as strings.

Empty input → empty result. Typical game-config subset, not a full language implementation.

```gdscript
var csv := Z_Parse.to_CSV([{name = "Ada", hp = 10}, {name = "Bob", hp = 8}])
var rows := Z_Parse.from_CSV(csv)

var yaml := Z_Parse.to_YAML({player = {name = "Ada", hp = 10}})
var data := Z_Parse.from_YAML(yaml)
```

## Z_Node (RefCounted, static)

```gdscript
Z_Node.children_remove_all(node: Node)
# Removes and frees every direct child.

Z_Node.get_overlapping_bodies_of_class(area: Node, class_name: String) -> Array
# area must be an Area2D/Area3D. Matches native class OR GDScript class_name,
# walking the base_script chain — so subclasses match too.
```

## Z_Sound (RefCounted, static)

```gdscript
Z_Sound.Oneshot(stream: AudioStream, volume_db := 0.0, pitch_scale := 1.0)
```

Spawns a transient `AudioStreamPlayer` under the root viewport, plays once, frees itself. Fire and
forget — no node to manage. snake_case alias: `oneshot`.

For music (looping tracks, crossfades, lock) use `Z_BGM` — see [audio.md](audio.md).

## ZDataList (Control)

Dynamic list widget (like `ItemList`, but each row is a custom `ZDataOption` scene). Places
itself in the UI tree, lays out options in a vertical or horizontal sectioned grid, and can
scroll. Option mounts are parented under an internal content host.

**Selection is per-player** — only one `ZDataOption` may be selected on a given `ZPlayer` at a
time (`ZPlayer.selected_data_option`). Any number of options may be **highlighted**.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `clear_list_on_ready` | `bool` | `false` | `true`: wipe on ready. `false`: adopt existing option children. |
| `list_owner` | `Object` | — | Free-form meta object options can read via `on_list_owner_changed`. |
| `option_scene` | `PackedScene` | — | Instantiated when `option_add` is given a non-option source. Root **or a child** must be `ZDataOption`. |
| `player_index` | `int` | `0` | Which `Z.players[i]` receives selection. |
| `list_layout` | `EListLayout` | `Vertical` | `Vertical` → `VBoxContainer` (or `GridContainer` when `sections` > 1). `Horizontal` → `HBoxContainer` (or HBox of VBoxes when `sections` > 1). |
| `sections` | `int` | `1` | Columns when vertical, rows when horizontal. |
| `scale_sections_to_size` | `bool` | `false` | Derive section count from available size ÷ option size. |
| `is_scrollable` | `bool` | `false` | Wrap content in a `ScrollContainer` (scroll axis follows layout). |
| `horizontal_alignment` | `EAlign` | `Begin` | Begin / Center / End. Main-axis for HBox; cross-axis shrink flags for children. |
| `vertical_alignment` | `EAlign` | `Begin` | Begin / Center / End. Main-axis for VBox; cross-axis shrink flags for children. |
| `expand_horizontal` | `bool` | `false` | Option mounts get `SIZE_EXPAND_FILL` on X. |
| `expand_vertical` | `bool` | `false` | Option mounts get `SIZE_EXPAND_FILL` on Y. |
| `default_options` | `Array[Resource]` | `[]` | Runtime: filled on ready when the list is empty. Editor: live preview (not saved into the scene). |

Script-only (bound, **not** shown in the inspector):

- `option_is_selectable: Callable` — `(option, index) -> bool` extra select gate
- `option_is_visible: Callable` — `(option, index) -> bool`; hides rows via `set_option_visible`

```gdscript
list.option_scene = preload("res://ui/option_row.tscn")
list.list_layout = ZDataList.LIST_LAYOUT_VERTICAL
list.sections = 2
list.is_scrollable = true
list.option_add_list([item_a, item_b, item_c], true)

list.option_select(0)                 # → Z.players[list.player_index].selected_data_option
list.option_hover(1)
list.option_highlight(2, true)
list.option_clear_all()
```

Signals: `on_option_selected`, `on_option_hovered`, `on_option_highlighted`,
`on_option_added`, `on_option_removed` — each pass `(option, index)` (highlight also has
`is_highlighted`).

PascalCase aliases: `Option_Add`, `Option_Add_List`, `Option_Clear_All`, `Option_Select`,
`Option_Hover`, `Option_Highlight`.

## ZDataOption (Control)

One list **row** as a `Control`. Optional widget links fill from
`source_object.get_z_data() -> ZGeneralData`, with optional manual overrides. Give option
scenes a sensible `custom_minimum_size` so the list can lay them out.

| Property | Type | Notes |
| --- | --- | --- |
| `source_object` | `Object` | Optional. Weak. |
| `select_button` | `Button` | When set: press → `select()`, also drives hover. When unset: the Control itself is clickable / hoverable. |
| `label_title` | `Label` | Title text. |
| `img_icon` | `TextureRect` | Icon (unless `use_button_icon`). |
| `label_description` | `RichTextLabel` | Description. |
| `use_button_icon` | `bool` | Put icon on `select_button` instead of (or in addition to) `img_icon`. |
| `use_button_text` | `bool` | Put title on `select_button.text` (from override / `get_z_data().title`). |
| `override_title` | `String` | Non-empty wins over `get_z_data().title`. |
| `override_icon` | `Texture2D` | Valid texture wins over `get_z_data().icon`. |
| `is_highlighted` | `bool` | Multi-select style mark; independent of player selection. |

```gdscript
# Non-overridable
option.select()
option.override_title = "Custom"
option.override_icon = preload("res://icon.png")

# Overridable in a script on the option:
func on_added_to_list(list: ZDataList, index: int, src_object: Object) -> void: ...
func on_source_object_changed(new_object: Object) -> void: ...
func on_list_owner_changed(list: ZDataList, new_owner: Object) -> void: ...
func _get_is_selectable() -> bool: return true
```

Signals: `on_selected`, `on_hovered(is_hovered)`, `on_highlighted(is_highlighted)`.

## ZGeneralData (RefCounted)

Lightweight presentational blob — **not** a `Resource`. Create with `ZGeneralData.new()` (or
from the inspector where a `ZGeneralData` property is expected).

| Property | Type | Default |
| --- | --- | --- |
| `title` | `String` | `""` |
| `icon` | `Texture2D` | — |
| `description` | `String` | `""` |
| `tags` | `Array[StringName]` | `[]` |
| `color` | `Color` | white |

**Convention:** any object may expose one by implementing:

```gdscript
func get_z_data() -> ZGeneralData
```

UI systems (e.g. option lists) call that method when present and fall back to manual fields when
not. `ZResource` implements it from its own params.

## ZResource (Resource)

Base for presentable data resources. Params live on the resource itself; `get_z_data()` snapshots
them into a `ZGeneralData`. The ZanDot resource preview generator uses `icon` in the FileSystem dock.

| Property | Type | Notes |
| --- | --- | --- |
| `title` | `String` | |
| `icon` | `Texture2D` | |
| `description` | `String` | |
| `tags` | `Array[StringName]` | |
| `color` | `Color` | |
| `meta` | `Dictionary` | Free-form game payload (not part of `get_z_data()`). |

```gdscript
var data: ZGeneralData = some_zresource.get_z_data()
print(data.title, data.tags)
some_zresource.meta = { "cost": 3, "slot": "weapon" }
```

Create via **Create New → Resource → ZResource**, or `extends ZResource` in GDScript.

## ZGlobalCondition / ZGlobalScript (Resource)

Tiny scriptable resources for data-driven checks and effects. They're what flow nodes, quest data,
and interactables reference instead of hardcoding logic.

```gdscript
extends ZGlobalCondition
class_name cond_has_key

@export var key_id: StringName

func _check() -> bool:
    return Z.save_game.params.get("keys", []).has(key_id)
```

```gdscript
extends ZGlobalScript
class_name gs_give_gold

@export var amount: int = 10

func _run() -> void:
    Z.save_game.params["gold"] = Z.save_game.params.get("gold", 0) + amount
```

Run them through `Z`:

```gdscript
Z.global_condition_check(cond) -> bool
Z.global_condition_check_list(conds: Array[ZGlobalCondition], if_one_true := false) -> bool
Z.global_script_run(script)
Z.global_script_run_list(scripts)
```

`if_one_true = false` (default) means ALL conditions must pass; `true` means ANY.

## ZDebugRoot2D / ZDebugRoot3D / ZDebugRootUI

Organizational parents for debug-only content — test geometry, spawners, dev overlays. Behaviour
depends on where they run:

| Context | Behaviour |
| --- | --- |
| Editor | Left completely alone, so you can place and see debug content. |
| Runtime from the editor | Children hidden and their collision disabled. |
| Exported game | `queue_free()`s itself and its children, **unless** launched with `--z-debug`. |

Pick the variant matching the content: `ZDebugRoot2D` (Node2D), `ZDebugRoot3D` (Node3D),
`ZDebugRootUI` (Control).

## ZCanvas3D (Node3D)

World-anchored container for **2D / Control** UI. Place `Control` or `Node2D` nodes as
children; at runtime they are made top-level and drawn at the screen position that maps to
this node's 3D world origin (via the viewport's active `Camera3D.unproject_position`).

Typical uses: nameplates, world markers, interaction prompts, floating damage numbers.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `clamp_to_screen` | `bool` | `false` | Keep content inside the viewport (with margins). |
| `screen_clamp_margins` | `Rect2` | `(0,0,0,0)` | Insets: `x`=left, `y`=top, `size.x`=right, `size.y`=bottom (pixels). |
| `screen_offset` | `Vector2` | `(0,0)` | Extra pixel offset after projection. |
| `center_content` | `bool` | `false` | Subtract half the combined Control content size so content is centered on the projected point. |
| `hide_when_behind_camera` | `bool` | `true` | Hide canvas when the world point is behind the camera. |
| `scale_by_distance` | `bool` | `false` | Scale = `distance_scale_reference / distance`, clamped. |
| `distance_scale_reference` | `float` | `10` | Distance at which scale is `1.0`. |
| `distance_scale_min` / `max` | `float` | `0.25` / `2` | Clamp for distance scaling. |
| `use_distance_fade` | `bool` | `false` | Fade out (and then hide) with distance. |
| `distance_fade_begin` | `float` | `20` | Distance where alpha is still full. |
| `distance_fade_end` | `float` | `10` | **Additional** distance after begin until fully faded (not an absolute end). |

```gdscript
# ZCanvas3D
# └── Label (or any Control / Node2D tree)
var plate: ZCanvas3D = $NamePlate
plate.center_content = true
plate.scale_by_distance = true
plate.distance_scale_reference = 8.0
plate.use_distance_fade = true
plate.distance_fade_begin = 15.0
plate.distance_fade_end = 10.0   # fully gone at 25 units
print(plate.get_screen_position())
```

Child local positions are treated as pixel offsets from the projected anchor. Give Controls a
`custom_minimum_size` (or explicit size) if you use `center_content` or screen clamp.
