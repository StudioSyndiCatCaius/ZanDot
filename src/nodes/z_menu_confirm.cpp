#include "z_menu_confirm.h"

#include "z_menu_default.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void ZMenu_Confirm::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_lbl_text", "label"), &ZMenu_Confirm::set_lbl_text);
	ClassDB::bind_method(D_METHOD("get_lbl_text"), &ZMenu_Confirm::get_lbl_text);

	ClassDB::bind_method(D_METHOD("set_btn_yes", "button"), &ZMenu_Confirm::set_btn_yes);
	ClassDB::bind_method(D_METHOD("get_btn_yes"), &ZMenu_Confirm::get_btn_yes);

	ClassDB::bind_method(D_METHOD("set_btn_no", "button"), &ZMenu_Confirm::set_btn_no);
	ClassDB::bind_method(D_METHOD("get_btn_no"), &ZMenu_Confirm::get_btn_no);

	ClassDB::bind_method(D_METHOD("set_text", "text"), &ZMenu_Confirm::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &ZMenu_Confirm::get_text);

	ClassDB::bind_method(D_METHOD("set_yes_text", "text"), &ZMenu_Confirm::set_yes_text);
	ClassDB::bind_method(D_METHOD("get_yes_text"), &ZMenu_Confirm::get_yes_text);

	ClassDB::bind_method(D_METHOD("set_no_text", "text"), &ZMenu_Confirm::set_no_text);
	ClassDB::bind_method(D_METHOD("get_no_text"), &ZMenu_Confirm::get_no_text);

	ClassDB::bind_method(D_METHOD("Confirm", "confirmed"), &ZMenu_Confirm::confirm);
	ClassDB::bind_method(D_METHOD("confirm", "confirmed"), &ZMenu_Confirm::confirm);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lbl_text", PROPERTY_HINT_NODE_TYPE, "RichTextLabel"), "set_lbl_text", "get_lbl_text");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "btn_yes", PROPERTY_HINT_NODE_TYPE, "Button"), "set_btn_yes", "get_btn_yes");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "btn_no", PROPERTY_HINT_NODE_TYPE, "Button"), "set_btn_no", "get_btn_no");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "yes_text"), "set_yes_text", "get_yes_text");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "no_text"), "set_no_text", "get_no_text");

	ADD_SIGNAL(MethodInfo("on_confirm", PropertyInfo(Variant::BOOL, "confirmed")));
}

ZMenu_Confirm::ZMenu_Confirm() {
}

ZMenu_Confirm::~ZMenu_Confirm() {
}

void ZMenu_Confirm::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY && !Engine::get_singleton()->is_editor_hint()) {
		set_process_unhandled_input(true);
		_connect_buttons();
		_apply_text();
		_apply_button_text();
		if (btn_yes != nullptr) {
			btn_yes->grab_focus();
		}
	}
}

void ZMenu_Confirm::_unhandled_input(const Ref<InputEvent> &p_event) {
	if (answered) {
		return;
	}
	if (consume_action(p_event, get_action_confirm())) {
		confirm(true);
	} else if (consume_action(p_event, get_action_close())) {
		confirm(false);
	}
}

void ZMenu_Confirm::confirm(bool p_confirmed) {
	if (answered) {
		return;
	}
	answered = true;
	emit_signal("on_confirm", p_confirmed);
	close_menu();
}

void ZMenu_Confirm::_on_yes_pressed() {
	confirm(true);
}

void ZMenu_Confirm::_on_no_pressed() {
	confirm(false);
}

void ZMenu_Confirm::_connect_buttons() {
	if (btn_yes != nullptr) {
		const Callable cb = callable_mp(this, &ZMenu_Confirm::_on_yes_pressed);
		if (!btn_yes->is_connected("pressed", cb)) {
			btn_yes->connect("pressed", cb);
		}
	}
	if (btn_no != nullptr) {
		const Callable cb = callable_mp(this, &ZMenu_Confirm::_on_no_pressed);
		if (!btn_no->is_connected("pressed", cb)) {
			btn_no->connect("pressed", cb);
		}
	}
}

void ZMenu_Confirm::_apply_text() {
	if (lbl_text != nullptr) {
		lbl_text->set_text(text);
	}
}

void ZMenu_Confirm::_apply_button_text() {
	if (btn_yes != nullptr) {
		btn_yes->set_text(yes_text);
	}
	if (btn_no != nullptr) {
		btn_no->set_text(no_text);
	}
}

void ZMenu_Confirm::set_lbl_text(RichTextLabel *p_label) {
	lbl_text = p_label;
	_apply_text();
}

RichTextLabel *ZMenu_Confirm::get_lbl_text() const {
	return lbl_text;
}

void ZMenu_Confirm::set_btn_yes(Button *p_button) {
	btn_yes = p_button;
	_apply_button_text();
	if (is_inside_tree()) {
		_connect_buttons();
	}
}

Button *ZMenu_Confirm::get_btn_yes() const {
	return btn_yes;
}

void ZMenu_Confirm::set_btn_no(Button *p_button) {
	btn_no = p_button;
	_apply_button_text();
	if (is_inside_tree()) {
		_connect_buttons();
	}
}

Button *ZMenu_Confirm::get_btn_no() const {
	return btn_no;
}

void ZMenu_Confirm::set_text(const String &p_text) {
	text = p_text;
	_apply_text();
}

String ZMenu_Confirm::get_text() const {
	return text;
}

void ZMenu_Confirm::set_yes_text(const String &p_text) {
	yes_text = p_text;
	_apply_button_text();
}

String ZMenu_Confirm::get_yes_text() const {
	return yes_text;
}

void ZMenu_Confirm::set_no_text(const String &p_text) {
	no_text = p_text;
	_apply_button_text();
}

String ZMenu_Confirm::get_no_text() const {
	return no_text;
}

ZMenu_Confirm *ZMenu_Confirm::create_default() {
	ZMenu_Confirm *menu = memnew(ZMenu_Confirm);
	menu->set_name("ZMenu_Confirm");

	CenterContainer *center = ZMenuDefault::build_backdrop(menu);
	VBoxContainer *content = ZMenuDefault::build_panel(center, Vector2(420, 0));

	menu->set_lbl_text(ZMenuDefault::build_label(content, Vector2(388, 48)));

	HBoxContainer *row = memnew(HBoxContainer);
	row->set_name("Buttons");
	row->add_theme_constant_override("separation", 12);
	content->add_child(row);

	menu->set_btn_yes(ZMenuDefault::build_button(row, menu->get_yes_text()));
	menu->set_btn_no(ZMenuDefault::build_button(row, menu->get_no_text()));

	return menu;
}
