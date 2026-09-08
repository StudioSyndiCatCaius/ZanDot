#include "flow_start_node.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FlowStartNode::_bind_methods() {
}

FlowStartNode::FlowStartNode() {
	if (get_node_id().is_empty()) {
		set_node_id("start");
	}
	rebuild_ports();
}

FlowStartNode::~FlowStartNode() {
}

void FlowStartNode::define() {
	// Entry: no inputs, single flow output.
	add_output_port("out");
}

String FlowStartNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Start";
}

String FlowStartNode::get_category() const {
	return "Built-in";
}

Color FlowStartNode::get_color() const {
	return Color(0.25f, 0.72f, 0.40f, 1.0f);
}

void FlowStartNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (is_active()) {
		trigger_output(0);
	}
}
