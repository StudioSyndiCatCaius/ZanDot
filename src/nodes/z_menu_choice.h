#pragma once

#include "z_data_list.h"
#include "z_data_option.h"
#include "z_menu.h"

#include "res/flow_node_choice.h"

#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Picks one option out of a list of ZFlowNode_Choice.
//
// Opened with `ZEvent.Choice(choices, on_select)`, and by ZFlowNode_ChoiceHUB
// for the choices wired to its output. Choices whose conditions fail are left
// out of the list; the emitted index always refers to the array that was
// passed in, not to the filtered rows.
class ZMenu_Choice : public ZMenu {
	GDCLASS(ZMenu_Choice, ZMenu)

private:
	ZDataList *choice_list = nullptr;
	TypedArray<ZFlowNode_Choice> choices;
	// Row index -> index into `choices`.
	PackedInt32Array row_to_choice;
	bool populated = false;
	bool answered = false;
	bool list_connected = false;

	void _connect_list();
	void _disconnect_list();
	void _populate();
	void _on_option_selected(ZDataOption *p_option, int p_index);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_choice_list(ZDataList *p_list);
	ZDataList *get_choice_list() const;

	void set_choices(const TypedArray<ZFlowNode_Choice> &p_choices);
	TypedArray<ZFlowNode_Choice> get_choices() const;

	// Answers with an index into `choices`; emits on_choice_selected then closes.
	void select_choice(int p_index);

	// Fallback UI used when no scene is set for this menu.
	static ZMenu_Choice *create_default();

	ZMenu_Choice();
	~ZMenu_Choice();
};

} // namespace godot
