#pragma once

#include "flow_node.h"
#include "flow_node_choice.h"

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Opens a ZMenu_Choice for every ZFlowNode_Choice wired to its "out" port.
//
// Choices whose conditions fail are left out of the menu. Picking one enters
// that choice node, so the graph continues along the branch behind it. With no
// available choice the hub simply stops.
class ZFlowNode_ChoiceHUB : public ZFlowNode {
	GDCLASS(ZFlowNode_ChoiceHUB, ZFlowNode)

private:
	// Scene whose root is a ZMenu_Choice; overrides Project Settings →
	// ZanDot → Menus → menu_choice for this hub only.
	Ref<PackedScene> override_menu;

	// Choices handed to the open menu, in the order it shows them (runtime only).
	TypedArray<ZFlowNode_Choice> pending_choices;

	void _on_choice_selected(int p_index);

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	void set_override_menu(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_override_menu() const;

	// Available choices connected to "out", in port order.
	TypedArray<ZFlowNode_Choice> collect_choices() const;

	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;
	void on_exit(int32_t p_pin, ZFlowPlayer *p_player) override;

	ZFlowNode_ChoiceHUB();
	~ZFlowNode_ChoiceHUB();
};

} // namespace godot
