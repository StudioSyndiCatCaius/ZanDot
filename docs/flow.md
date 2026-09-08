# Flow Graphs

Flow graphs are node-graph assets for branching logic — dialogue, cutscenes, quest steps. They are
plain Godot resources (`.tres`), edited in a dedicated **Flow** main-screen tab next to
Scene/Script, and played back at runtime by a `ZFlowPlayer` node.

## The editor

- Open it from the main-screen tabs, or double-click a `ZFlowGraph` `.tres` in the FileSystem dock.
- **Left**: open graphs list + a filterable palette of node types (drag or double-click to add).
  Row icons come from `_get_node_icon()`, else `_get_thumbnail()`, else a `_get_color()` swatch.
- **Center**: the GraphEdit canvas. New graphs get a Start node pre-added.
- **Right**: inspectors for the selected `ZFlowNode` and for the graph asset.
- Legacy `.gdflow` (JSON) files can be opened read-only; re-save them as `.tres`.

## ZFlowGraph (Resource)

The asset itself. **`node_id` values are unique within a graph** — `add_node` replaces on collision,
and `set_nodes` reassigns duplicates.

| Property | Notes |
| --- | --- |
| `nodes` | Nested `ZFlowNode` sub-resources. Graph-owned; hidden in the inspector. |
| `connections` | Graph-owned; hidden in the inspector. |
| `params` | Free-form graph-level `Dictionary` for your own data. |

Editing API (mostly used by the editor plugin, but available at runtime):

```gdscript
graph.add_node(node) / remove_node(id) / get_node(id) / has_node(id)
graph.get_start_node() / find_hub_node(hub_id)
graph.add_connection(...) / remove_connection(...) / clear_connections_for_node(id)
graph.get_connections_from(id, port) / get_connections_to(id, port := -1)
graph.get_next_node_id(...)
graph.generate_node_id() / ensure_defaults()
graph.get_nodes_from_ids(ids: PackedInt64Array) -> Array[ZFlowNode]  # Object instance ids
graph.get_ids_from_nodes(nodes: Array[ZFlowNode]) -> PackedInt64Array
graph.to_dict() / from_dict(d)
```

Because nodes are real sub-resources, any `@export` on a GDScript flow node serializes with the
graph — no custom JSON layer.

## ZFlowNode (Resource)

The base class for graph nodes. Subclass it (GDScript or C++) to make a node type.
Default ports: one input `"in"`, one output `"out"`.

| Property | Notes |
| --- | --- |
| `node_id` | Unique string id within the graph. |
| `display_name` | Editable label. |
| `params` | Free-form `Dictionary`. |
| `position`, `inputs`, `outputs` | Graph-owned; hidden in the inspector. |

### Runtime

Nodes control the flow. Multiple may be **active** at once (fan-out via `trigger_output`).

| Method | Purpose |
| --- | --- |
| `is_active() -> bool` | Entered and not yet stopped. |
| `get_graph() -> ZFlowGraph` | Owning graph, falling back to the one the runtime player is playing. |
| `get_player() -> ZFlowPlayer` | Player that entered this node; `null` when not active. |
| `get_connected_nodes(input, output, input_pin := -1, output_pin := -1) -> Array[ZFlowNode]` | Neighbors. Pin `< 0` = all pins of that direction. |
| `trigger_output(output, connection := -1, stop_node := true) -> bool` | Activate connected targets. `connection < 0` = all links on that output. |
| `trigger_output_to_node(node, input := 0, only_if_connected := true, stop_node := true) -> bool` | Activate a specific node. |
| `stop_node()` | Deactivate without firing outputs. |

### Overridables

| Method | Purpose |
| --- | --- |
| `_define()` | Declare ports with `add_input_port(name)` / `add_output_port(name)`. |
| `_get_caption() -> String` | Title on the graph node. |
| `_get_category() -> String` | Palette grouping. |
| `_get_color() -> Color` | Node chrome color. |
| `_get_thumbnail() -> Texture2D` | Large preview drawn in the node's body, below the ports (return `null` for none). |
| `_get_node_icon() -> Texture2D` | Small icon in the titlebar, left of the caption (return `null` for none). |
| `_get_node_size() -> Vector2` | Editor size override. `0` on an axis = size to content. |
| `_get_graph_exposed_params() -> PackedStringArray` | Property names to edit inline on the graph node. |
| `_on_enter(pin: int, player: ZFlowPlayer, previous_node: ZFlowNode, previous_out_pin: int)` | Became active. Call `trigger_output` to continue, or stay active (e.g. wait for UI). |
| `_on_exit(pin: int, player: ZFlowPlayer)` | Leaving (`pin` = output used, or `-1` for `stop_node()`). |

### Built-in node types

| Class | Purpose |
| --- | --- |
| `FlowStartNode` / `FlowEndNode` | Entry and exit. Start auto-triggers `out`; End finishes the whole player. |
| `FlowBasicNode` | Pass-through (triggers `out` on enter). |
| `ZFlowHubNode` | Named jump target (`hub_id`); triggers `out` on enter. |
| `ZFlowToHubNode` | Jumps to the hub with a matching `hub_id`. |
| `ZFlowCheckGlobalConditionsNode` | Runs conditions; triggers `true` (0) or `false` (1). |
| `ZFlowDoGlobalScriptNode` | Runs scripts, then triggers `out`. |
| `ZFlowNode_ChoiceHUB` | Opens a `ZMenu_Choice` for the choices on `out`; enters the picked one. See [menus.md](menus.md#choices-in-a-flow-graph). |
| `ZFlowNode_Choice` | One selectable option (`text` + `conditions`); triggers `out` when picked. |

## ZFlowPlayer (Node)

Plays a graph from Start. **Multiple nodes can be active** at once. Recorded nodes are those
entered at least once during this playback.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `flow_graph` | `ZFlowGraph` | — | Graph to play. |
| `end_flow_on_no_active_nodes` | `bool` | `true` | When the last active node stops with no successors, emit `flow_end`. |
| `variables` | `Dictionary` | `{}` | Per-playback scratch data. |

```gdscript
player.play()                       # start from the Start node
player.stop()                       # deactivates all; emits flow_end if it was playing
player.is_playing()
player.nodes_get_active() -> Array[ZFlowNode]
player.nodes_get_recorded() -> Array[ZFlowNode]
player.activate_node(node, pin := 0, previous := null, previous_out_pin := -1)
player.deactivate_node(node, exit_pin := -1)
player.set_variable(name, value) / get_variable(name, default = null)

signal flow_begin()
signal flow_end()
signal node_enter(node: ZFlowNode)
signal node_exit(node: ZFlowNode)
```

**Pausing for dialogue:** a message node’s `_on_enter` shows UI and does **not** call
`trigger_output`. It stays active until the UI calls `node.trigger_output(port)`.

## Writing a custom node type

```gdscript
@tool                                    # required — the editor instantiates nodes
extends ZFlowNode
class_name FN_Message

@export var speaker: Resource
@export_multiline var text := ""

func _get_caption() -> String:
    return "Message"

func _get_category() -> String:
    return "Dialogue"

func _define() -> void:
    add_input_port("in")
    add_output_port("out")

func _get_color() -> Color:
    return Color(0.4, 0.65, 1.0)

func _get_graph_exposed_params() -> PackedStringArray:
    return PackedStringArray(["speaker", "text"])

func _on_enter(pin: int, player: ZFlowPlayer, previous_node: ZFlowNode, previous_out_pin: int) -> void:
    DialogueUI.show_line(speaker, text)
    # stay active — UI later: trigger_output(0)
```

**Always mark flow node scripts `@tool`.** Without it Godot uses a placeholder instance in the
editor and your methods can't run — ZanDot recovers ports/caption/category/color by parsing the
script source as a fallback, but it's a fallback.

For a multiple-choice branch, declare one output port per choice in `_define()` and call
`trigger_output(chosen_index)` from UI.

## Playback pattern

Play graphs from a game state so entering/leaving dialogue is a state transition:

```gdscript
extends ZGameState

@onready var flow: ZFlowPlayer = $ZFlowPlayer

func _on_state_enter(context) -> void:
    flow.flow_graph = context as ZFlowGraph
    flow.flow_end.connect(_on_flow_end, CONNECT_ONE_SHOT)
    flow.play()

func _on_flow_end() -> void:
    get_game_mode().change_to_default_state()
```
