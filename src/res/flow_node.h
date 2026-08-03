#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {

class FlowPlayer;
class Script;

// Base flow-graph node. Subclass (C++ or script) and override define() / _define
// to declare ports. Instantiable concrete types: FlowBasicNode, FlowStartNode, FlowEndNode, …
// FlowNode itself is instantiable so GDScript can `extends FlowNode` + set_script.
//
// Editor note: GDScript FlowNode types used in the Flow editor should be @tool scripts.
// Without @tool, Godot uses a placeholder instance and methods cannot run in the editor.
// We still recover ports/caption/category/color from source when possible.
class FlowNode : public Resource {
	GDCLASS(FlowNode, Resource)

private:
	String node_id;
	String display_name;
	Vector2 position;
	PackedStringArray inputs;
	PackedStringArray outputs;
	Dictionary params;

protected:
	static void _bind_methods();

	// C++ override points.
	virtual void define();
	virtual Color get_color() const;
	virtual String get_caption() const;
	virtual String get_category() const;
	virtual PackedStringArray get_graph_exposed_params() const;
	virtual Ref<Texture2D> get_thumbnail() const;

	// Script hooks (GDScript / C#).
	GDVIRTUAL0(_define)
	GDVIRTUAL0RC(Color, _get_color)
	GDVIRTUAL0RC(String, _get_caption)
	GDVIRTUAL0RC(String, _get_category)
	GDVIRTUAL0RC(PackedStringArray, _get_graph_exposed_params)
	// Thumbnail is optional Texture2D; use call()/C++ override only (avoid Object* GDVIRTUAL,
	// which registers an invalid RESOURCE_TYPE hint for plain Object and can crash on load).
	GDVIRTUAL1(_on_enter, Object *)
	GDVIRTUAL1(_on_exit, Object *)
	GDVIRTUAL1R(int64_t, _choose_output, Object *)

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

	bool _script_has_method(const StringName &p_method) const;
	bool _call_script_void(const StringName &p_method);
	bool _call_script_ret(const StringName &p_method, Variant &r_ret) const;
	static bool _is_bad_meta_string(const String &p_s);

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
	// Editor thumbnail; null Ref if none / invalid (caller should only show when valid).
	Ref<Texture2D> get_thumbnail_bind() const;

	// Script global class name, path basename, or native get_class().
	String get_type_name() const;

	// Runtime
	virtual void on_enter(FlowPlayer *p_player);
	virtual void on_exit(FlowPlayer *p_player);
	virtual int64_t choose_output(FlowPlayer *p_player);

	virtual bool is_start_node() const { return false; }
	virtual bool is_end_node() const { return false; }

	FlowNode();
	~FlowNode();
};

} // namespace godot
