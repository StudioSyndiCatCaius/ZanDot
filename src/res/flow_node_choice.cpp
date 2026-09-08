#include "flow_node_choice.h"

#include "nodes/z.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZFlowNode_Choice::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_text", "text"), &ZFlowNode_Choice::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &ZFlowNode_Choice::get_text);

	ClassDB::bind_method(D_METHOD("set_conditions", "conditions"), &ZFlowNode_Choice::set_conditions);
	ClassDB::bind_method(D_METHOD("get_conditions"), &ZFlowNode_Choice::get_conditions);

	ClassDB::bind_method(D_METHOD("is_available"), &ZFlowNode_Choice::is_available);
	ClassDB::bind_method(D_METHOD("get_z_data"), &ZFlowNode_Choice::get_z_data);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "conditions", PROPERTY_HINT_ARRAY_TYPE, "ZGlobalCondition"), "set_conditions", "get_conditions");
}

ZFlowNode_Choice::ZFlowNode_Choice() {
	rebuild_ports();
}

ZFlowNode_Choice::~ZFlowNode_Choice() {
}

void ZFlowNode_Choice::define() {
	add_input_port("in");
	add_output_port("out");
}

String ZFlowNode_Choice::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Choice";
}

String ZFlowNode_Choice::get_category() const {
	return "Built-in";
}

Color ZFlowNode_Choice::get_color() const {
	return Color(0.34f, 0.76f, 0.0f, 1.0f);
}

PackedStringArray ZFlowNode_Choice::get_graph_exposed_params() const {
	PackedStringArray params;
	params.push_back("text");
	return params;
}

void ZFlowNode_Choice::set_text(const String &p_text) {
	text = p_text;
	emit_changed();
}

String ZFlowNode_Choice::get_text() const {
	return text;
}

void ZFlowNode_Choice::set_conditions(const TypedArray<ZGlobalCondition> &p_conditions) {
	conditions = p_conditions;
	emit_changed();
}

TypedArray<ZGlobalCondition> ZFlowNode_Choice::get_conditions() const {
	return conditions;
}

bool ZFlowNode_Choice::is_available() const {
	if (conditions.is_empty()) {
		return true;
	}
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return true;
	}
	return z->global_condition_check_list(conditions, false);
}

Ref<ZGeneralData> ZFlowNode_Choice::get_z_data() const {
	Ref<ZGeneralData> data;
	data.instantiate();
	data->set_title(text);
	data->set_color(get_color());
	return data;
}

void ZFlowNode_Choice::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (!is_active()) {
		return;
	}
	// Picking a choice is the whole action — carry straight on down "out".
	trigger_output(0);
}
