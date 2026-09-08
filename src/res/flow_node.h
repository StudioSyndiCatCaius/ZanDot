#pragma once

// ZFlowPlayer must be a complete type here: the _on_enter / _on_exit script
// hooks below are typed against it, and GDVIRTUAL needs get_class_static().
// flow_player.h only forward-declares ZFlowNode, so this does not cycle.
#include "nodes/flow_player.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {

class ZFlowGraph;
class Script;

// Base flow-graph node. Default ports: 1 input ("in") + 1 output ("out").
// Subclass (C++ or script) and override define() / _define to declare custom ports.
// Instantiable concrete types: FlowBasicNode, FlowStartNode, FlowEndNode, …
// ZFlowNode itself is instantiable so GDScript can `extends ZFlowNode` + set_script.
//
// Editor note: GDScript ZFlowNode types used in the Flow editor should be @tool scripts.
// Without @tool, Godot uses a placeholder instance and methods cannot run in the editor.
// We still recover ports/caption/category/color from source when possible.
//
// Runtime: a node is active while a ZFlowPlayer has entered it and it has not yet
// stop_node()'d / triggered an output that stops it. Multiple nodes may be active.
class ZFlowNode : public Resource {
	GDCLASS(ZFlowNode, Resource)

private:
	String node_id;
	String display_name;
	Vector2 position;
	PackedStringArray inputs;
	PackedStringArray outputs;
	Dictionary params;

	// Runtime (not serialized). Owning graph is a raw back-pointer (graph owns node Refs).
	ZFlowGraph *owning_graph = nullptr;
	ZFlowPlayer *runtime_player = nullptr;
	bool active = false;

protected:
	static void _bind_methods();

	// C++ override points.
	virtual void define();
	virtual Color get_color() const;
	virtual String get_caption() const;
	virtual String get_category() const;
	virtual PackedStringArray get_graph_exposed_params() const;
	// Large preview drawn in the graph node's body.
	virtual Ref<Texture2D> get_thumbnail() const;
	// Small icon drawn in the graph node's titlebar, next to the caption.
	virtual Ref<Texture2D> get_node_icon() const;
	// Custom graph node size in the editor. Zero on an axis = let the node size itself.
	virtual Vector2 get_node_size() const;

	// Script hooks (GDScript / C#).
	GDVIRTUAL0(_define)
	GDVIRTUAL0RC(Color, _get_color)
	GDVIRTUAL0RC(String, _get_caption)
	GDVIRTUAL0RC(String, _get_category)
	GDVIRTUAL0RC(PackedStringArray, _get_graph_exposed_params)
	GDVIRTUAL0RC(Vector2, _get_node_size)
	// Thumbnail / icon are optional Texture2D; use call()/C++ override only (avoid Object* GDVIRTUAL,
	// which registers an invalid RESOURCE_TYPE hint for plain Object and can crash on load).
	// _on_enter(pin: int, player: ZFlowPlayer, previous_node: ZFlowNode, previous_out_pin: int)
	GDVIRTUAL4(_on_enter, int64_t, ZFlowPlayer *, Ref<ZFlowNode>, int64_t)
	// _on_exit(pin: int, player: ZFlowPlayer) — pin is the output used when leaving, or -1 if stop_node()
	GDVIRTUAL2(_on_exit, int64_t, ZFlowPlayer *)

	// Port helpers for use inside define() / _define().
	void clear_ports();
	void add_input_port(const String &p_name);
	void add_output_port(const String &p_name);

	// True when script methods may safely run (runtime, or editor + @tool).
	bool _script_methods_safe() const;
	Ref<Script> _get_attached_script() const;
	String _get_script_source() const;

	// Static GDScript source recovery (no execution — works for non-@tool in editor).
	bool _apply_ports_from_script_source();
	String _parse_script_string_return(const String &p_func_name) const;
	Color _parse_script_color_return(const String &p_func_name) const;
	PackedStringArray _parse_script_string_array_return(const String &p_func_name) const;

	// Result of a script `_get_*` returning a Texture2D; null Ref if absent / wrong type.
	Ref<Texture2D> _script_texture(const StringName &p_method) const;

	bool _script_has_method(const StringName &p_method) const;
	bool _call_script_void(const StringName &p_method);
	bool _call_script_ret(const StringName &p_method, Variant &r_ret) const;
	static bool _is_bad_meta_string(const String &p_s);

	// Player-only lifecycle (not for game scripts to call directly).
	friend class ZFlowPlayer;
	void _set_runtime_player(ZFlowPlayer *p_player);
	void _set_active_flag(bool p_active);

public:
	void set_node_id(const String &p_id);
	String get_node_id() const;

	void set_display_name(const String &p_name);
	String get_display_name() const;

	void set_position(const Vector2 &p_position);
	Vector2 get_position() const;

	// Ports are owned by define(); exposed for serialization / tools only (hidden in inspector).
	void set_inputs(const PackedStringArray &p_inputs);
	PackedStringArray get_inputs() const;

	void set_outputs(const PackedStringArray &p_outputs);
	PackedStringArray get_outputs() const;

	void set_params(const Dictionary &p_params);
	Dictionary get_params() const;

	// Rebuild inputs/outputs from define() / _define(). Call after construction or script attach.
	void rebuild_ports();

	// Bound accessors that dispatch to virtuals / GDVIRTUAL / script call / source parse.
	String get_caption_bind() const;
	String get_category_bind() const;
	Color get_color_bind() const;
	PackedStringArray get_graph_exposed_params_bind() const;
	// Editor body thumbnail; null Ref if none / invalid (caller should only show when valid).
	Ref<Texture2D> get_thumbnail_bind() const;
	// Editor titlebar icon; null Ref if none / invalid.
	Ref<Texture2D> get_node_icon_bind() const;
	// Editor node size override; (0, 0) = auto.
	Vector2 get_node_size_bind() const;

	// Script global class name, path basename, or native get_class().
	String get_type_name() const;

	// --- Graph ownership (set by ZFlowGraph; not serialized) ---
	void set_owning_graph(ZFlowGraph *p_graph);
	ZFlowGraph *get_owning_graph() const;
	ZFlowPlayer *get_runtime_player() const;
	// Script-facing versions of the two above; the graph falls back to the one
	// the runtime player is playing when this node has no owner yet.
	Ref<ZFlowGraph> get_graph_bind() const;

	// --- Runtime state ---
	bool is_active() const;

	// Runtime enter/exit. Prefer overriding _on_enter / _on_exit from GDScript.
	// pin: input pin that was entered (or -1). previous_node / previous_out_pin describe the source.
	// To continue the flow, call trigger_output(...) (or stay active until UI does).
	virtual void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin);
	// pin: output pin used when leaving via trigger_output, or -1 for stop_node().
	virtual void on_exit(int32_t p_pin, ZFlowPlayer *p_player);

	// Connected neighbors. If input/output pin is < 0, include all pins of that direction.
	TypedArray<ZFlowNode> get_connected_nodes(bool p_input, bool p_output, int32_t p_input_pin = -1, int32_t p_output_pin = -1) const;

	// Activate nodes linked from this output. connection < 0 = all connections on that port.
	// stop_node: if true, deactivates this node first (exit pin = output).
	bool trigger_output(int32_t p_output, int32_t p_connection = -1, bool p_stop_node = true);
	// Activate a specific node (optionally requiring a real connection from this node).
	bool trigger_output_to_node(const Ref<ZFlowNode> &p_node, int32_t p_input = 0, bool p_only_if_connected = true, bool p_stop_node = true);

	// Ends this node instance; it is no longer active.
	void stop_node();

	virtual bool is_start_node() const { return false; }
	virtual bool is_end_node() const { return false; }

	ZFlowNode();
	~ZFlowNode();
};

} // namespace godot
