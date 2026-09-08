#include "flow_check_global_conditions_node.h"

#include "nodes/z.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZFlowCheckGlobalConditionsNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_conditions", "conditions"), &ZFlowCheckGlobalConditionsNode::set_conditions);
	ClassDB::bind_method(D_METHOD("get_conditions"), &ZFlowCheckGlobalConditionsNode::get_conditions);

	ClassDB::bind_method(D_METHOD("set_if_one_true", "value"), &ZFlowCheckGlobalConditionsNode::set_if_one_true);
	ClassDB::bind_method(D_METHOD("get_if_one_true"), &ZFlowCheckGlobalConditionsNode::get_if_one_true);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "conditions", PROPERTY_HINT_ARRAY_TYPE, "ZGlobalCondition"), "set_conditions", "get_conditions");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "if_one_true"), "set_if_one_true", "get_if_one_true");
}

ZFlowCheckGlobalConditionsNode::ZFlowCheckGlobalConditionsNode() {
	rebuild_ports();
}

ZFlowCheckGlobalConditionsNode::~ZFlowCheckGlobalConditionsNode() {
}

void ZFlowCheckGlobalConditionsNode::define() {
	add_input_port("in");
	add_output_port("true");
	add_output_port("false");
}

String ZFlowCheckGlobalConditionsNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Check Global Conditions";
}

String ZFlowCheckGlobalConditionsNode::get_category() const {
	return "Built-in";
}

Color ZFlowCheckGlobalConditionsNode::get_color() const {
	return Color(0.35f, 0.65f, 0.85f, 1.0f);
}

void ZFlowCheckGlobalConditionsNode::set_conditions(const TypedArray<ZGlobalCondition> &p_conditions) {
	conditions = p_conditions;
	emit_changed();
}

TypedArray<ZGlobalCondition> ZFlowCheckGlobalConditionsNode::get_conditions() const {
	return conditions;
}

void ZFlowCheckGlobalConditionsNode::set_if_one_true(bool p_value) {
	if_one_true = p_value;
	emit_changed();
}

bool ZFlowCheckGlobalConditionsNode::get_if_one_true() const {
	return if_one_true;
}

void ZFlowCheckGlobalConditionsNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (!is_active()) {
		return;
	}
	Z *z = Z::get_singleton();
	const bool ok = (z != nullptr) ? z->global_condition_check_list(conditions, if_one_true) : false;
	trigger_output(ok ? 0 : 1);
}
