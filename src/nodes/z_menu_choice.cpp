#include "z_menu_choice.h"

#include "z_menu_default.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

// Minimal row scene for the fallback menu: a ZDataOption whose Button shows the
// choice title (ZDataOption fills it from ZFlowNode_Choice.get_z_data()).
Ref<PackedScene> make_default_option_scene() {
	ZDataOption *option = memnew(ZDataOption);
	option->set_name("ChoiceOption");
	option->set_custom_minimum_size(Vector2(320, 34));

	Button *button = memnew(Button);
	button->set_name("SelectButton");
	button->set_anchors_preset(Control::PRESET_FULL_RECT);
	option->add_child(button);
	// pack() only keeps children that the scene root owns.
	button->set_owner(option);

	option->set_select_button(button);
	option->set_use_button_text(true);

	Ref<PackedScene> scene;
	scene.instantiate();
	if (scene->pack(option) != OK) {
		scene.unref();
	}
	memdelete(option);
	return scene;
}

} // namespace

void ZMenu_Choice::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_choice_list", "list"), &ZMenu_Choice::set_choice_list);
	ClassDB::bind_method(D_METHOD("get_choice_list"), &ZMenu_Choice::get_choice_list);

	ClassDB::bind_method(D_METHOD("set_choices", "choices"), &ZMenu_Choice::set_choices);
	ClassDB::bind_method(D_METHOD("get_choices"), &ZMenu_Choice::get_choices);

	ClassDB::bind_method(D_METHOD("select_choice", "index"), &ZMenu_Choice::select_choice);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "choice_list", PROPERTY_HINT_NODE_TYPE, "ZDataList"), "set_choice_list", "get_choice_list");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "choices", PROPERTY_HINT_ARRAY_TYPE, "ZFlowNode_Choice", PROPERTY_USAGE_NONE), "set_choices", "get_choices");

	ADD_SIGNAL(MethodInfo("on_choice_selected", PropertyInfo(Variant::INT, "index")));
}

ZMenu_Choice::ZMenu_Choice() {
}

ZMenu_Choice::~ZMenu_Choice() {
}

void ZMenu_Choice::_notification(int p_what) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (p_what == NOTIFICATION_READY) {
		_connect_list();
		_populate();
		if (row_to_choice.is_empty()) {
			// Nothing to pick — close rather than leaving an empty menu that
			// whoever is awaiting it can never dismiss.
			close_menu();
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		_disconnect_list();
	}
}

void ZMenu_Choice::_connect_list() {
	if (list_connected || choice_list == nullptr) {
		return;
	}
	const Callable cb = callable_mp(this, &ZMenu_Choice::_on_option_selected);
	if (!choice_list->is_connected("on_option_selected", cb)) {
		choice_list->connect("on_option_selected", cb);
	}
	list_connected = true;
}

void ZMenu_Choice::_disconnect_list() {
	if (!list_connected || choice_list == nullptr) {
		list_connected = false;
		return;
	}
	const Callable cb = callable_mp(this, &ZMenu_Choice::_on_option_selected);
	if (choice_list->is_connected("on_option_selected", cb)) {
		choice_list->disconnect("on_option_selected", cb);
	}
	list_connected = false;
}

void ZMenu_Choice::_populate() {
	if (populated || choice_list == nullptr) {
		return;
	}
	populated = true;

	choice_list->option_clear_all();
	row_to_choice.clear();

	for (int i = 0; i < choices.size(); i++) {
		Ref<ZFlowNode_Choice> choice = choices[i];
		if (choice.is_null() || !choice->is_available()) {
			continue;
		}
		if (choice_list->option_add(choice.ptr()) != nullptr) {
			row_to_choice.push_back(i);
		}
	}

	if (row_to_choice.is_empty()) {
		UtilityFunctions::push_warning("ZMenu_Choice: no available choices to show.");
	}
}

void ZMenu_Choice::_on_option_selected(ZDataOption *p_option, int p_index) {
	(void)p_option;
	if (p_index < 0 || p_index >= row_to_choice.size()) {
		return;
	}
	select_choice(row_to_choice[p_index]);
}

void ZMenu_Choice::select_choice(int p_index) {
	if (answered) {
		return;
	}
	answered = true;
	emit_signal("on_choice_selected", p_index);
	close_menu();
}

void ZMenu_Choice::set_choice_list(ZDataList *p_list) {
	if (choice_list == p_list) {
		return;
	}
	_disconnect_list();
	choice_list = p_list;
	populated = false;
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
		_connect_list();
		_populate();
	}
}

ZDataList *ZMenu_Choice::get_choice_list() const {
	return choice_list;
}

void ZMenu_Choice::set_choices(const TypedArray<ZFlowNode_Choice> &p_choices) {
	choices = p_choices;
	populated = false;
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
		_populate();
	}
}

TypedArray<ZFlowNode_Choice> ZMenu_Choice::get_choices() const {
	return choices;
}

ZMenu_Choice *ZMenu_Choice::create_default() {
	ZMenu_Choice *menu = memnew(ZMenu_Choice);
	menu->set_name("ZMenu_Choice");

	CenterContainer *center = ZMenuDefault::build_backdrop(menu);
	VBoxContainer *content = ZMenuDefault::build_panel(center, Vector2(420, 0));

	ZDataList *list = memnew(ZDataList);
	list->set_name("ChoiceList");
	list->set_custom_minimum_size(Vector2(388, 0));
	list->set_list_layout(ZDataList::LIST_LAYOUT_VERTICAL);
	list->set_expand_horizontal(true);
	list->set_option_scene(make_default_option_scene());
	content->add_child(list);

	menu->set_choice_list(list);
	return menu;
}
