#include "z_input_listener.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

void ZInputListener::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_visibility_root", "path"), &ZInputListener::set_visibility_root);
	ClassDB::bind_method(D_METHOD("get_visibility_root"), &ZInputListener::get_visibility_root);

	ClassDB::bind_method(D_METHOD("set_actions", "actions"), &ZInputListener::set_actions);
	ClassDB::bind_method(D_METHOD("get_actions"), &ZInputListener::get_actions);

	ClassDB::bind_method(D_METHOD("set_process_unhandled", "enabled"), &ZInputListener::set_process_unhandled);
	ClassDB::bind_method(D_METHOD("get_process_unhandled"), &ZInputListener::get_process_unhandled);

	ClassDB::bind_method(D_METHOD("poll_input", "delta"), &ZInputListener::poll_input);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "visibility_root", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node"), "set_visibility_root", "get_visibility_root");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "actions"), "set_actions", "get_actions");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "process_unhandled"), "set_process_unhandled", "get_process_unhandled");

	ADD_SIGNAL(MethodInfo("on_input_begin", PropertyInfo(Variant::STRING, "action")));
	ADD_SIGNAL(MethodInfo("on_input_update", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::FLOAT, "dt")));
	ADD_SIGNAL(MethodInfo("on_input_end", PropertyInfo(Variant::STRING, "action")));
}

ZInputListener::ZInputListener() {
	set_process(true);
}

ZInputListener::~ZInputListener() {
}

void ZInputListener::_notification(int p_what) {
	if (p_what == NOTIFICATION_PROCESS) {
		poll_input(get_process_delta_time());
	}
}

void ZInputListener::set_visibility_root(const NodePath &p_path) {
	visibility_root = p_path;
}

NodePath ZInputListener::get_visibility_root() const {
	return visibility_root;
}

void ZInputListener::set_actions(const PackedStringArray &p_actions) {
	actions = p_actions;
}

PackedStringArray ZInputListener::get_actions() const {
	return actions;
}

void ZInputListener::set_process_unhandled(bool p_enabled) {
	process_unhandled = p_enabled;
}

bool ZInputListener::get_process_unhandled() const {
	return process_unhandled;
}

Node *ZInputListener::_resolve_visibility_root() const {
	if (visibility_root.is_empty()) {
		return nullptr;
	}
	return get_node_or_null(visibility_root);
}

bool ZInputListener::_is_input_enabled() const {
	Node *root = _resolve_visibility_root();
	if (root == nullptr) {
		// No visibility gate configured → always allow input.
		return true;
	}

	// Prefer CanvasItem / Node3D visibility; fall back to tree presence.
	CanvasItem *ci = Object::cast_to<CanvasItem>(root);
	if (ci != nullptr) {
		return ci->is_visible_in_tree();
	}
	Node3D *n3 = Object::cast_to<Node3D>(root);
	if (n3 != nullptr) {
		return n3->is_visible_in_tree();
	}
	return root->is_inside_tree();
}

void ZInputListener::poll_input(double p_delta) {
	if (!_is_input_enabled()) {
		// Release any held actions when gated off.
		if (!held_actions.is_empty()) {
			Vector<StringName> copy;
			for (const StringName &a : held_actions) {
				copy.push_back(a);
			}
			held_actions.clear();
			for (int i = 0; i < copy.size(); i++) {
				emit_signal("on_input_end", String(copy[i]));
			}
		}
		return;
	}

	Input *input = Input::get_singleton();
	if (input == nullptr) {
		return;
	}

	for (int i = 0; i < actions.size(); i++) {
		const String action_str = actions[i];
		if (action_str.is_empty()) {
			continue;
		}
		const StringName action(action_str);
		const bool pressed = input->is_action_pressed(action);

		if (pressed) {
			if (!held_actions.has(action)) {
				held_actions.insert(action);
				emit_signal("on_input_begin", action_str);
			}
			emit_signal("on_input_update", action_str, p_delta);
		} else if (held_actions.has(action)) {
			held_actions.erase(action);
			emit_signal("on_input_end", action_str);
		}
	}
}
