#include "z_menu_process.h"

#include "z_menu_default.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZMenu_Process::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_lbl_text", "label"), &ZMenu_Process::set_lbl_text);
	ClassDB::bind_method(D_METHOD("get_lbl_text"), &ZMenu_Process::get_lbl_text);

	ClassDB::bind_method(D_METHOD("set_process_progress_bar", "bar"), &ZMenu_Process::set_process_progress_bar);
	ClassDB::bind_method(D_METHOD("get_process_progress_bar"), &ZMenu_Process::get_process_progress_bar);

	ClassDB::bind_method(D_METHOD("set_text", "text"), &ZMenu_Process::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &ZMenu_Process::get_text);

	ClassDB::bind_method(D_METHOD("set_update", "update"), &ZMenu_Process::set_update);
	ClassDB::bind_method(D_METHOD("get_update"), &ZMenu_Process::get_update);

	ClassDB::bind_method(D_METHOD("get_process_percent"), &ZMenu_Process::get_process_percent);
	ClassDB::bind_method(D_METHOD("get_percent"), &ZMenu_Process::get_percent);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lbl_text", PROPERTY_HINT_NODE_TYPE, "RichTextLabel"), "set_lbl_text", "get_lbl_text");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "process_progress_bar", PROPERTY_HINT_NODE_TYPE, "ProgressBar"), "set_process_progress_bar", "get_process_progress_bar");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	// `update` stays script-only — a Callable has no useful inspector editor.

	ADD_SIGNAL(MethodInfo("on_progress", PropertyInfo(Variant::FLOAT, "percent")));

	GDVIRTUAL_BIND(_get_process_percent);
}

ZMenu_Process::ZMenu_Process() {
}

ZMenu_Process::~ZMenu_Process() {
}

void ZMenu_Process::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY && !Engine::get_singleton()->is_editor_hint()) {
		_apply_text();
		_apply_percent(get_process_percent());
		set_process(true);
	}
}

void ZMenu_Process::_process(double p_delta) {
	if (finished || Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	const float pct = get_process_percent();
	_apply_percent(pct);
	emit_signal("on_progress", pct);

	if (pct >= 1.0f) {
		finished = true;
		set_process(false);
		close_menu();
	}
}

float ZMenu_Process::get_process_percent() {
	float result = 0.0f;
	if (has_method("_get_process_percent")) {
		// Guarded: an unimplemented virtual answers with null, which would read
		// as 0% forever and never let the menu close.
		const Variant ret = call("_get_process_percent");
		if (ret.get_type() == Variant::FLOAT || ret.get_type() == Variant::INT) {
			return (float)ret;
		}
	}
	if (GDVIRTUAL_CALL(_get_process_percent, result)) {
		return result;
	}
	if (update.is_valid()) {
		const Variant ret = update.call();
		if (ret.get_type() == Variant::FLOAT || ret.get_type() == Variant::INT) {
			return (float)ret;
		}
		UtilityFunctions::push_warning("ZMenu_Process: update callable did not return a number.");
	}
	return percent;
}

float ZMenu_Process::get_percent() const {
	return percent;
}

void ZMenu_Process::_apply_percent(float p_percent) {
	percent = Math::clamp(p_percent, 0.0f, 1.0f);
	if (process_progress_bar == nullptr) {
		return;
	}
	// Respect whatever min/max the bar was authored with.
	const double min_value = process_progress_bar->get_min();
	const double max_value = process_progress_bar->get_max();
	process_progress_bar->set_value(min_value + (max_value - min_value) * (double)percent);
}

void ZMenu_Process::_apply_text() {
	if (lbl_text != nullptr) {
		lbl_text->set_text(text);
	}
}

void ZMenu_Process::set_lbl_text(RichTextLabel *p_label) {
	lbl_text = p_label;
	_apply_text();
}

RichTextLabel *ZMenu_Process::get_lbl_text() const {
	return lbl_text;
}

void ZMenu_Process::set_process_progress_bar(ProgressBar *p_bar) {
	process_progress_bar = p_bar;
	_apply_percent(percent);
}

ProgressBar *ZMenu_Process::get_process_progress_bar() const {
	return process_progress_bar;
}

void ZMenu_Process::set_text(const String &p_text) {
	text = p_text;
	_apply_text();
}

String ZMenu_Process::get_text() const {
	return text;
}

void ZMenu_Process::set_update(const Callable &p_update) {
	update = p_update;
}

Callable ZMenu_Process::get_update() const {
	return update;
}

ZMenu_Process *ZMenu_Process::create_default() {
	ZMenu_Process *menu = memnew(ZMenu_Process);
	menu->set_name("ZMenu_Process");

	CenterContainer *center = ZMenuDefault::build_backdrop(menu);
	VBoxContainer *content = ZMenuDefault::build_panel(center, Vector2(420, 0));

	menu->set_lbl_text(ZMenuDefault::build_label(content, Vector2(388, 32)));

	ProgressBar *bar = memnew(ProgressBar);
	bar->set_name("Progress");
	bar->set_min(0.0);
	bar->set_max(1.0);
	bar->set_step(0.0);
	bar->set_show_percentage(true);
	bar->set_custom_minimum_size(Vector2(388, 20));
	content->add_child(bar);
	menu->set_process_progress_bar(bar);

	return menu;
}
