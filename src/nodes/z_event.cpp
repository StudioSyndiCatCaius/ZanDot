#include "z_event.h"

#include "z_menu_choice.h"
#include "z_menu_confirm.h"
#include "z_menu_info.h"
#include "z_menu_process.h"
#include "z_settings.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZEvent::_bind_methods() {
	// Spec-style names (PascalCase, as elsewhere in ZanDot) …
	ClassDB::bind_static_method("ZEvent", D_METHOD("Confirm", "text", "on_yes", "on_no", "yes_text", "no_text"),
			&ZEvent::confirm, DEFVAL(Callable()), DEFVAL(Callable()), DEFVAL("Yes"), DEFVAL("No"));
	ClassDB::bind_static_method("ZEvent", D_METHOD("Info", "text"), &ZEvent::info);
	ClassDB::bind_static_method("ZEvent", D_METHOD("Process", "text", "update"), &ZEvent::process);
	ClassDB::bind_static_method("ZEvent", D_METHOD("Choice", "choices", "on_select", "override_menu"),
			&ZEvent::choice, DEFVAL(Callable()), DEFVAL(Ref<PackedScene>()));
	ClassDB::bind_static_method("ZEvent", D_METHOD("OpenMenu", "scene"), &ZEvent::open_menu);
	ClassDB::bind_static_method("ZEvent", D_METHOD("Menus_Get_Open"), &ZEvent::get_open_menus);
	ClassDB::bind_static_method("ZEvent", D_METHOD("Menu_Get_Top"), &ZEvent::get_top_menu);
	ClassDB::bind_static_method("ZEvent", D_METHOD("Menu_Is_Open"), &ZEvent::is_menu_open);

	// … plus snake_case aliases.
	ClassDB::bind_static_method("ZEvent", D_METHOD("confirm", "text", "on_yes", "on_no", "yes_text", "no_text"),
			&ZEvent::confirm, DEFVAL(Callable()), DEFVAL(Callable()), DEFVAL("Yes"), DEFVAL("No"));
	ClassDB::bind_static_method("ZEvent", D_METHOD("info", "text"), &ZEvent::info);
	ClassDB::bind_static_method("ZEvent", D_METHOD("process", "text", "update"), &ZEvent::process);
	ClassDB::bind_static_method("ZEvent", D_METHOD("choice", "choices", "on_select", "override_menu"),
			&ZEvent::choice, DEFVAL(Callable()), DEFVAL(Ref<PackedScene>()));
	ClassDB::bind_static_method("ZEvent", D_METHOD("open_menu", "scene"), &ZEvent::open_menu);
	ClassDB::bind_static_method("ZEvent", D_METHOD("get_open_menus"), &ZEvent::get_open_menus);
	ClassDB::bind_static_method("ZEvent", D_METHOD("get_top_menu"), &ZEvent::get_top_menu);
	ClassDB::bind_static_method("ZEvent", D_METHOD("is_menu_open"), &ZEvent::is_menu_open);
}

ZMenu *ZEvent::_menu_from_setting(const char *p_setting_key, const StringName &p_class_name) {
	const String path = ZSettings::get(p_setting_key, String(""));
	if (path.is_empty()) {
		return nullptr;
	}

	Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
	if (scene.is_null()) {
		UtilityFunctions::push_warning(String("ZEvent: could not load menu scene '") + path + "' (" + p_setting_key + ").");
		return nullptr;
	}

	Node *root = scene->instantiate();
	ZMenu *menu = Object::cast_to<ZMenu>(root);
	if (menu == nullptr) {
		UtilityFunctions::push_warning(String("ZEvent: '") + path + "' root is not a " + String(p_class_name) + "; using the built-in menu.");
		if (root != nullptr) {
			memdelete(root);
		}
		return nullptr;
	}
	if (!menu->is_class(p_class_name)) {
		UtilityFunctions::push_warning(String("ZEvent: '") + path + "' root is a " + menu->get_class() + ", expected " + String(p_class_name) + "; using the built-in menu.");
		memdelete(menu);
		return nullptr;
	}
	return menu;
}

Signal ZEvent::_spawn(ZMenu *p_menu) {
	if (p_menu == nullptr) {
		return Signal();
	}

	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr || tree->get_root() == nullptr) {
		UtilityFunctions::push_warning("ZEvent: no active SceneTree — menu not opened.");
		memdelete(p_menu);
		return Signal();
	}

	// Parented to the root viewport so the menu outlives scene changes.
	tree->get_root()->add_child(p_menu);
	return Signal(p_menu, "tree_exited");
}

Signal ZEvent::confirm(const String &p_text, const Callable &p_on_yes, const Callable &p_on_no,
		const String &p_yes_text, const String &p_no_text) {
	ZMenu_Confirm *menu = Object::cast_to<ZMenu_Confirm>(_menu_from_setting(ZSettings::MENU_CONFIRM, "ZMenu_Confirm"));
	if (menu == nullptr) {
		menu = ZMenu_Confirm::create_default();
	}

	menu->set_text(p_text);
	menu->set_yes_text(p_yes_text);
	menu->set_no_text(p_no_text);

	if (p_on_yes.is_valid() || p_on_no.is_valid()) {
		menu->connect("on_confirm", callable_mp_static(&ZEvent::_on_confirm_result).bind(p_on_yes, p_on_no));
	}
	return _spawn(menu);
}

Signal ZEvent::info(const String &p_text) {
	ZMenu_Info *menu = Object::cast_to<ZMenu_Info>(_menu_from_setting(ZSettings::MENU_INFO, "ZMenu_Info"));
	if (menu == nullptr) {
		menu = ZMenu_Info::create_default();
	}
	menu->set_text(p_text);
	return _spawn(menu);
}

Signal ZEvent::process(const String &p_text, const Callable &p_update) {
	ZMenu_Process *menu = Object::cast_to<ZMenu_Process>(_menu_from_setting(ZSettings::MENU_PROCESS, "ZMenu_Process"));
	if (menu == nullptr) {
		menu = ZMenu_Process::create_default();
	}
	menu->set_text(p_text);
	menu->set_update(p_update);
	return _spawn(menu);
}

Signal ZEvent::choice(const TypedArray<ZFlowNode_Choice> &p_choices, const Callable &p_on_select,
		const Ref<PackedScene> &p_override_menu) {
	ZMenu_Choice *menu = nullptr;

	if (p_override_menu.is_valid()) {
		Node *root = p_override_menu->instantiate();
		menu = Object::cast_to<ZMenu_Choice>(root);
		if (menu == nullptr) {
			UtilityFunctions::push_warning("ZEvent.Choice: override_menu root is not a ZMenu_Choice.");
			if (root != nullptr) {
				memdelete(root);
			}
		}
	}
	if (menu == nullptr) {
		menu = Object::cast_to<ZMenu_Choice>(_menu_from_setting(ZSettings::MENU_CHOICE, "ZMenu_Choice"));
	}
	if (menu == nullptr) {
		menu = ZMenu_Choice::create_default();
	}

	menu->set_choices(p_choices);
	if (p_on_select.is_valid()) {
		menu->connect("on_choice_selected", callable_mp_static(&ZEvent::_on_choice_result).bind(p_on_select));
	}
	return _spawn(menu);
}

ZMenu *ZEvent::open_menu(const Ref<PackedScene> &p_scene) {
	if (p_scene.is_null()) {
		UtilityFunctions::push_warning("ZEvent.OpenMenu: scene is null.");
		return nullptr;
	}

	Node *root = p_scene->instantiate();
	ZMenu *menu = Object::cast_to<ZMenu>(root);
	if (menu == nullptr) {
		UtilityFunctions::push_warning("ZEvent.OpenMenu: scene root is not a ZMenu.");
		if (root != nullptr) {
			memdelete(root);
		}
		return nullptr;
	}

	if (!_spawn(menu).is_null()) {
		return menu;
	}
	return nullptr;
}

void ZEvent::_on_confirm_result(bool p_confirmed, Callable p_on_yes, Callable p_on_no) {
	const Callable &target = p_confirmed ? p_on_yes : p_on_no;
	if (target.is_valid()) {
		target.call();
	}
}

void ZEvent::_on_choice_result(int p_index, Callable p_on_select) {
	if (p_on_select.is_valid()) {
		p_on_select.call(p_index);
	}
}

TypedArray<ZMenu> ZEvent::get_open_menus() {
	return ZMenu::get_open_menus();
}

ZMenu *ZEvent::get_top_menu() {
	return ZMenu::get_top_menu();
}

bool ZEvent::is_menu_open() {
	return ZMenu::get_top_menu() != nullptr;
}
