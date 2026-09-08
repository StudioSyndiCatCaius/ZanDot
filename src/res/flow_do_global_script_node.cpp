#include "flow_do_global_script_node.h"

#include "nodes/z.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZFlowDoGlobalScriptNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_scripts", "scripts"), &ZFlowDoGlobalScriptNode::set_scripts);
	ClassDB::bind_method(D_METHOD("get_scripts"), &ZFlowDoGlobalScriptNode::get_scripts);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "scripts", PROPERTY_HINT_ARRAY_TYPE, "ZGlobalScript"), "set_scripts", "get_scripts");
}

ZFlowDoGlobalScriptNode::ZFlowDoGlobalScriptNode() {
	rebuild_ports();
}

ZFlowDoGlobalScriptNode::~ZFlowDoGlobalScriptNode() {
}

void ZFlowDoGlobalScriptNode::define() {
	add_input_port("in");
	add_output_port("out");
}

String ZFlowDoGlobalScriptNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Do Global Script";
}

String ZFlowDoGlobalScriptNode::get_category() const {
	return "Built-in";
}

Color ZFlowDoGlobalScriptNode::get_color() const {
	return Color(0.85f, 0.55f, 0.25f, 1.0f);
}

void ZFlowDoGlobalScriptNode::set_scripts(const TypedArray<ZGlobalScript> &p_scripts) {
	scripts = p_scripts;
	emit_changed();
}

TypedArray<ZGlobalScript> ZFlowDoGlobalScriptNode::get_scripts() const {
	return scripts;
}

void ZFlowDoGlobalScriptNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	Z *z = Z::get_singleton();
	if (z != nullptr) {
		z->global_script_run_list(scripts);
	}
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (is_active()) {
		trigger_output(0);
	}
}
