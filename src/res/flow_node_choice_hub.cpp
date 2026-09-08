#include "flow_node_choice_hub.h"

#include "nodes/z_event.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZFlowNode_ChoiceHUB::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_override_menu", "scene"), &ZFlowNode_ChoiceHUB::set_override_menu);
	ClassDB::bind_method(D_METHOD("get_override_menu"), &ZFlowNode_ChoiceHUB::get_override_menu);

	ClassDB::bind_method(D_METHOD("collect_choices"), &ZFlowNode_ChoiceHUB::collect_choices);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "override_menu", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_override_menu", "get_override_menu");
}

ZFlowNode_ChoiceHUB::ZFlowNode_ChoiceHUB() {
	rebuild_ports();
}

ZFlowNode_ChoiceHUB::~ZFlowNode_ChoiceHUB() {
}

void ZFlowNode_ChoiceHUB::define() {
	add_input_port("in");
	add_output_port("out");
}

String ZFlowNode_ChoiceHUB::get_caption() const {
	if (!get_display_name().is_empty()) {
		return get_display_name();
	}
	return "Choice Hub";
}

String ZFlowNode_ChoiceHUB::get_category() const {
	return "Built-in";
}

Color ZFlowNode_ChoiceHUB::get_color() const {
	return Color(0.0f, 0.62f, 0.37f, 1.0f);
}

void ZFlowNode_ChoiceHUB::set_override_menu(const Ref<PackedScene> &p_scene) {
	override_menu = p_scene;
	emit_changed();
}

Ref<PackedScene> ZFlowNode_ChoiceHUB::get_override_menu() const {
	return override_menu;
}

TypedArray<ZFlowNode_Choice> ZFlowNode_ChoiceHUB::collect_choices() const {
	TypedArray<ZFlowNode_Choice> result;
	const TypedArray<ZFlowNode> connected = get_connected_nodes(false, true, -1, 0);
	for (int i = 0; i < connected.size(); i++) {
		Ref<ZFlowNode_Choice> choice = connected[i];
		if (choice.is_valid() && choice->is_available()) {
			result.push_back(choice);
		}
	}
	return result;
}

void ZFlowNode_ChoiceHUB::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	ZFlowNode::on_enter(p_pin, p_player, p_previous_node, p_previous_out_pin);
	if (!is_active()) {
		return;
	}

	pending_choices = collect_choices();
	if (pending_choices.is_empty()) {
		UtilityFunctions::push_warning("ZFlowNode_ChoiceHUB: no available ZFlowNode_Choice on 'out' — stopping.");
		stop_node();
		return;
	}

	// Stays active until the menu answers; the chosen node is entered from there.
	ZEvent::choice(pending_choices, callable_mp(this, &ZFlowNode_ChoiceHUB::_on_choice_selected), override_menu);
}

void ZFlowNode_ChoiceHUB::on_exit(int32_t p_pin, ZFlowPlayer *p_player) {
	pending_choices.clear();
	ZFlowNode::on_exit(p_pin, p_player);
}

void ZFlowNode_ChoiceHUB::_on_choice_selected(int p_index) {
	if (p_index < 0 || p_index >= pending_choices.size()) {
		return;
	}
	Ref<ZFlowNode_Choice> chosen = pending_choices[p_index];
	pending_choices.clear();
	if (chosen.is_null()) {
		stop_node();
		return;
	}
	trigger_output_to_node(chosen, 0, true, true);
}
