#include "flow_basic_node.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FlowBasicNode::_bind_methods() {
}

FlowBasicNode::FlowBasicNode() {
	rebuild_ports();
}

FlowBasicNode::~FlowBasicNode() {
}

void FlowBasicNode::define() {
	add_input_port("in");
	add_output_port("out");
}

String FlowBasicNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Basic";
}

String FlowBasicNode::get_category() const {
	return "Built-in";
}

Color FlowBasicNode::get_color() const {
	return Color(0.50f, 0.55f, 0.65f, 1.0f);
}

void FlowBasicNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (is_active()) {
		trigger_output(0);
	}
}
