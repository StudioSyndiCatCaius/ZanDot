#include "flow_end_node.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FlowEndNode::_bind_methods() {
}

FlowEndNode::FlowEndNode() {
	if (get_node_id().is_empty()) {
		set_node_id("end");
	}
	rebuild_ports();
}

FlowEndNode::~FlowEndNode() {
}

void FlowEndNode::define() {
	// Terminal: single flow input, no outputs.
	add_input_port("in");
}

String FlowEndNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "End";
}

String FlowEndNode::get_category() const {
	return "Built-in";
}

Color FlowEndNode::get_color() const {
	return Color(0.85f, 0.32f, 0.32f, 1.0f);
}
