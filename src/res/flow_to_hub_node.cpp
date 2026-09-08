#include "flow_to_hub_node.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZFlowToHubNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_hub_id", "id"), &ZFlowToHubNode::set_hub_id);
	ClassDB::bind_method(D_METHOD("get_hub_id"), &ZFlowToHubNode::get_hub_id);

	// StringName; flow editor may refine to a hub dropdown later.
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "hub_id"), "set_hub_id", "get_hub_id");
}

ZFlowToHubNode::ZFlowToHubNode() {
	rebuild_ports();
}

ZFlowToHubNode::~ZFlowToHubNode() {
}

void ZFlowToHubNode::define() {
	add_input_port("in");
	// No outputs — ZFlowPlayer jumps to the matching hub on enter.
}

String ZFlowToHubNode::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	if (hub_id != StringName()) {
		return String("To Hub: ") + String(hub_id);
	}
	return "To Hub";
}

String ZFlowToHubNode::get_category() const {
	return "Built-in";
}

Color ZFlowToHubNode::get_color() const {
	return Color(0.70f, 0.45f, 0.90f, 1.0f);
}

PackedStringArray ZFlowToHubNode::get_graph_exposed_params() const {
	PackedStringArray a;
	a.push_back("hub_id");
	return a;
}

void ZFlowToHubNode::set_hub_id(const StringName &p_id) {
	hub_id = p_id;
	emit_changed();
}

StringName ZFlowToHubNode::get_hub_id() const {
	return hub_id;
}
