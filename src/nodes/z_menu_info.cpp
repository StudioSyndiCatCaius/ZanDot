#include "z_menu_info.h"

#include "z_menu_default.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void ZMenu_Info::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_lbl_text", "label"), &ZMenu_Info::set_lbl_text);
	ClassDB::bind_method(D_METHOD("get_lbl_text"), &ZMenu_Info::get_lbl_text);

	ClassDB::bind_method(D_METHOD("set_btn_close", "button"), &ZMenu_Info::set_btn_close);
	ClassDB::bind_method(D_METHOD("get_btn_close"), &ZMenu_Info::get_btn_close);

	ClassDB::bind_method(D_METHOD("set_text", "text"), &ZMenu_Info::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &ZMenu_Info::get_text);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lbl_text", PROPERTY_HINT_NODE_TYPE, "RichTextLabel"), "set_lbl_text", "get_lbl_text");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "btn_close", PROPERTY_HINT_NODE_TYPE, "Button"), "set_btn_close", "get_btn_close");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
}

ZMenu_Info::ZMenu_Info() {
}

ZMenu_Info::~ZMenu_Info() {
}

void ZMenu_Info::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY && !Engine::get_singleton()->is_editor_hint()) {
		set_process_unhandled_input(true);
		_connect_button();
		_apply_text();
		if (btn_close != nullptr) {
			btn_close->grab_focus();
		}
	}
}

void ZMenu_Info::_unhandled_input(const Ref<InputEvent> &p_event) {
	if (consume_action(p_event, get_action_confirm()) || consume_action(p_event, get_action_close())) {
		close_menu();
	}
}

void ZMenu_Info::_on_close_pressed() {
	close_menu();
}

void ZMenu_Info::_connect_button() {
	if (btn_close == nullptr) {
		return;
	}
	const Callable cb = callable_mp(this, &ZMenu_Info::_on_close_pressed);
	if (!btn_close->is_connected("pressed", cb)) {
		btn_close->connect("pressed", cb);
	}
}

void ZMenu_Info::_apply_text() {
	if (lbl_text != nullptr) {
		lbl_text->set_text(text);
	}
}

void ZMenu_Info::set_lbl_text(RichTextLabel *p_label) {
	lbl_text = p_label;
	_apply_text();
}

RichTextLabel *ZMenu_Info::get_lbl_text() const {
	return lbl_text;
}

void ZMenu_Info::set_btn_close(Button *p_button) {
	btn_close = p_button;
	if (is_inside_tree()) {
		_connect_button();
	}
}

Button *ZMenu_Info::get_btn_close() const {
	return btn_close;
}

void ZMenu_Info::set_text(const String &p_text) {
	text = p_text;
	_apply_text();
}

String ZMenu_Info::get_text() const {
	return text;
}

ZMenu_Info *ZMenu_Info::create_default() {
	ZMenu_Info *menu = memnew(ZMenu_Info);
	menu->set_name("ZMenu_Info");

	CenterContainer *center = ZMenuDefault::build_backdrop(menu);
	VBoxContainer *content = ZMenuDefault::build_panel(center, Vector2(420, 0));

	menu->set_lbl_text(ZMenuDefault::build_label(content, Vector2(388, 48)));
	menu->set_btn_close(ZMenuDefault::build_button(content, "OK"));

	return menu;
}
