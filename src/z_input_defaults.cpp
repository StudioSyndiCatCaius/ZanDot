#include "z_input_defaults.h"

#include "z_settings.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

// Triggers rest near 0 and rise to 1, so they need a real deadzone; sticks use
// Godot's usual 0.2 so a resting stick does not creep.
const float DEFAULT_DEADZONE = 0.2f;

bool wrote_any = false;

Ref<InputEvent> key_event(Key p_key) {
	Ref<InputEventKey> ev;
	ev.instantiate();
	ev->set_physical_keycode(p_key);
	return ev;
}

Ref<InputEvent> joy_button_event(JoyButton p_button) {
	Ref<InputEventJoypadButton> ev;
	ev.instantiate();
	ev->set_button_index(p_button);
	return ev;
}

// p_value is the axis end this action listens to: -1 or +1 (triggers use +1).
Ref<InputEvent> joy_axis_event(JoyAxis p_axis, float p_value) {
	Ref<InputEventJoypadMotion> ev;
	ev.instantiate();
	ev->set_axis(p_axis);
	ev->set_axis_value(p_value);
	return ev;
}

Ref<InputEvent> mouse_button_event(MouseButton p_button) {
	Ref<InputEventMouseButton> ev;
	ev.instantiate();
	ev->set_button_index(p_button);
	return ev;
}

// Adds p_action with p_events when it does not exist yet. The live InputMap and
// (in the editor) ProjectSettings are kept in step, so a binding edited in
// Project Settings wins on the next launch.
void def_action(const char *p_action, const Array &p_events) {
	InputMap *map = InputMap::get_singleton();
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (map == nullptr) {
		return;
	}

	const StringName action(p_action);
	const String setting_key = String("input/") + String(p_action);

	if (!map->has_action(action)) {
		map->add_action(action, DEFAULT_DEADZONE);
		for (int i = 0; i < p_events.size(); i++) {
			Ref<InputEvent> ev = p_events[i];
			if (ev.is_valid()) {
				map->action_add_event(action, ev);
			}
		}
	}

	if (ps == nullptr || ps->has_setting(setting_key)) {
		return;
	}

	Dictionary info;
	info["deadzone"] = DEFAULT_DEADZONE;
	info["events"] = p_events;
	ps->set_setting(setting_key, info);
	wrote_any = true;
}

Array events(const Ref<InputEvent> &p_a, const Ref<InputEvent> &p_b = Ref<InputEvent>()) {
	Array a;
	a.push_back(p_a);
	if (p_b.is_valid()) {
		a.push_back(p_b);
	}
	return a;
}

} // namespace

void ZInputDefaults::register_all() {
	if (!(bool)ZSettings::get(ZSettings::REGISTER_DEFAULT_INPUT_ACTIONS, true)) {
		return;
	}

	wrote_any = false;

	// Movement — WASD + left stick. Godot's forward is -Z, so "up" is the
	// negative-Y end of the stick (see ZMover::move_by_input).
	def_action("z_move_up", events(key_event(KEY_W), joy_axis_event(JOY_AXIS_LEFT_Y, -1.0f)));
	def_action("z_move_down", events(key_event(KEY_S), joy_axis_event(JOY_AXIS_LEFT_Y, 1.0f)));
	def_action("z_move_left", events(key_event(KEY_A), joy_axis_event(JOY_AXIS_LEFT_X, -1.0f)));
	def_action("z_move_right", events(key_event(KEY_D), joy_axis_event(JOY_AXIS_LEFT_X, 1.0f)));

	// Look — right stick. Mouse look is handled by ZInputCommon directly, since
	// relative motion is not an InputMap action.
	def_action("z_look_up", events(joy_axis_event(JOY_AXIS_RIGHT_Y, -1.0f)));
	def_action("z_look_down", events(joy_axis_event(JOY_AXIS_RIGHT_Y, 1.0f)));
	def_action("z_look_left", events(joy_axis_event(JOY_AXIS_RIGHT_X, -1.0f)));
	def_action("z_look_right", events(joy_axis_event(JOY_AXIS_RIGHT_X, 1.0f)));

	// Menu navigation — arrows + dpad.
	def_action("z_nav_up", events(key_event(KEY_UP), joy_button_event(JOY_BUTTON_DPAD_UP)));
	def_action("z_nav_down", events(key_event(KEY_DOWN), joy_button_event(JOY_BUTTON_DPAD_DOWN)));
	def_action("z_nav_left", events(key_event(KEY_LEFT), joy_button_event(JOY_BUTTON_DPAD_LEFT)));
	def_action("z_nav_right", events(key_event(KEY_RIGHT), joy_button_event(JOY_BUTTON_DPAD_RIGHT)));

	// Face buttons. Confirm/cancel default to Godot's own ui_* actions, so only
	// the other two need registering (Xbox layout: top = Y, left = X).
	def_action("z_face_top", events(key_event(KEY_R), joy_button_event(JOY_BUTTON_Y)));
	def_action("z_face_left", events(key_event(KEY_F), joy_button_event(JOY_BUTTON_X)));

	// Stick clicks.
	def_action("z_special_l", events(key_event(KEY_Z), joy_button_event(JOY_BUTTON_LEFT_STICK)));
	def_action("z_special_r", events(key_event(KEY_C), joy_button_event(JOY_BUTTON_RIGHT_STICK)));

	// Bumpers.
	def_action("z_shoulder_l", events(key_event(KEY_Q), joy_button_event(JOY_BUTTON_LEFT_SHOULDER)));
	def_action("z_shoulder_r", events(key_event(KEY_E), joy_button_event(JOY_BUTTON_RIGHT_SHOULDER)));

	// Triggers — analog on a pad, mouse buttons on keyboard.
	def_action("z_trigger_l", events(mouse_button_event(MOUSE_BUTTON_RIGHT), joy_axis_event(JOY_AXIS_TRIGGER_LEFT, 1.0f)));
	def_action("z_trigger_r", events(mouse_button_event(MOUSE_BUTTON_LEFT), joy_axis_event(JOY_AXIS_TRIGGER_RIGHT, 1.0f)));

	// Only the editor can write the project file; an exported game keeps the
	// runtime-only InputMap entries added above.
	if (wrote_any && OS::get_singleton()->has_feature("editor")) {
		ProjectSettings *ps = ProjectSettings::get_singleton();
		if (ps != nullptr && ps->save() == OK) {
			UtilityFunctions::print("ZanDot: added default z_* input actions to the Input Map.");
		}
	}
}
