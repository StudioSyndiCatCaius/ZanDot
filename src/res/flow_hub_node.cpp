#include "flow_hub_node.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZFlowHubNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_hub_id", "id"), &ZFlowHubNode::set_hub_id);
	ClassDB::bind_method(D_METHOD("get_hub_id"), &ZFlowHubNode::get_hub_id);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "hub_id"), "set_hub_id", "get_hub_id");
}

ZFlowHubNode::ZFlowHubNode() {
	rebuild_ports();
}

ZFlowHubNode::~ZFlowHubNode() {
}

void ZFlowHubNode::define() {
	// No inputs — entered only via ToHub jump (or as an explicit start target).
	add_output_port("out");
}

String ZFlowHubNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	if (hub_id != StringName()) {
		return String("Hub: ") + String(hub_id);
	}
	return "Hub";
}

String ZFlowHubNode::get_category() const {
	return "Built-in";
}

Color ZFlowHubNode::get_color() const {
	return Color(0.55f, 0.40f, 0.85f, 1.0f);
}

void ZFlowHubNode::set_hub_id(const StringName &p_id) {
	hub_id = p_id;
	emit_changed();
}

StringName ZFlowHubNode::get_hub_id() const {
	return hub_id;
}

void ZFlowHubNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (is_active()) {
		trigger_output(0);
	}
}
