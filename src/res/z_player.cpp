#include "z_player.h"

#include "../nodes/z_data_option.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

const char *INPUT_TRIGGERED_METHOD = "input_triggered";
const char *INPUT_IS_ENABLED_METHOD = "input_is_enabled";

} // namespace

void ZPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZPlayer::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZPlayer::get_player_index);

	ClassDB::bind_method(D_METHOD("set_control_rotation", "rotation"), &ZPlayer::set_control_rotation);
	ClassDB::bind_method(D_METHOD("get_control_rotation"), &ZPlayer::get_control_rotation);

	ClassDB::bind_method(D_METHOD("possess", "scene"), &ZPlayer::possess);
	ClassDB::bind_method(D_METHOD("unpossess"), &ZPlayer::unpossess);
	ClassDB::bind_method(D_METHOD("get_possessed_scene"), &ZPlayer::get_possessed_scene);
	ClassDB::bind_method(D_METHOD("set_possessed_scene", "scene"), &ZPlayer::set_possessed_scene);

	ClassDB::bind_method(D_METHOD("set_input_targets", "targets"), &ZPlayer::set_input_targets);
	ClassDB::bind_method(D_METHOD("get_input_targets"), &ZPlayer::get_input_targets);
	ClassDB::bind_method(D_METHOD("input_target_add", "target"), &ZPlayer::input_target_add);
	ClassDB::bind_method(D_METHOD("input_target_remove", "target"), &ZPlayer::input_target_remove);
	ClassDB::bind_method(D_METHOD("input_targets_clear"), &ZPlayer::input_targets_clear);
	ClassDB::bind_method(D_METHOD("has_input_target", "target"), &ZPlayer::has_input_target);

	ClassDB::bind_method(D_METHOD("input_action", "action", "state", "dt"), &ZPlayer::input_action, DEFVAL(0.0));
	ClassDB::bind_method(D_METHOD("input_begin", "action"), &ZPlayer::input_begin);
	ClassDB::bind_method(D_METHOD("input_update", "action", "dt"), &ZPlayer::input_update);
	ClassDB::bind_method(D_METHOD("input_end", "action"), &ZPlayer::input_end);

	ClassDB::bind_method(D_METHOD("set_selected_data_option", "option"), &ZPlayer::set_selected_data_option);
	ClassDB::bind_method(D_METHOD("get_selected_data_option"), &ZPlayer::get_selected_data_option);
	ClassDB::bind_method(D_METHOD("clear_selected_data_option"), &ZPlayer::clear_selected_data_option);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index"), "set_player_index", "get_player_index");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "control_rotation"), "set_control_rotation", "get_control_rotation");
	// possessed_scene is runtime-only; expose for scripts but do not serialize.
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "possessed_scene", PROPERTY_HINT_NODE_TYPE, "Node", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE), "set_possessed_scene", "get_possessed_scene");
	// input_targets is runtime-only too (objects, not resource paths).
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "input_targets", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE), "set_input_targets", "get_input_targets");
	// selected_data_option is runtime UI state; weak ref, not serialized.
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "selected_data_option", PROPERTY_HINT_NODE_TYPE, "ZDataOption", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE), "set_selected_data_option", "get_selected_data_option");

	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(PRESSED);
	BIND_ENUM_CONSTANT(DOWN);
	BIND_ENUM_CONSTANT(RELEASED);
}

ZPlayer::ZPlayer() {
}

ZPlayer::~ZPlayer() {
}

void ZPlayer::set_player_index(int p_index) {
	if (player_index == p_index) {
		return;
	}
	player_index = p_index;
	emit_changed();
}

int ZPlayer::get_player_index() const {
	return player_index;
}

void ZPlayer::set_control_rotation(const Vector3 &p_rotation) {
	if (control_rotation == p_rotation) {
		return;
	}
	control_rotation = p_rotation;
	emit_changed();
}

Vector3 ZPlayer::get_control_rotation() const {
	return control_rotation;
}

void ZPlayer::possess(Node *p_scene) {
	set_possessed_scene(p_scene);
}

void ZPlayer::unpossess() {
	set_possessed_scene(nullptr);
}

void ZPlayer::set_possessed_scene(Node *p_scene) {
	if (p_scene == nullptr) {
		possessed_object_id = ObjectID();
	} else {
		possessed_object_id = p_scene->get_instance_id();
	}
	emit_changed();
}

Node *ZPlayer::get_possessed_scene() const {
	if (possessed_object_id.is_null()) {
		return nullptr;
	}
	Object *obj = ObjectDB::get_instance(possessed_object_id);
	return Object::cast_to<Node>(obj);
}

// --- Input routing ---

void ZPlayer::_prune_input_targets() {
	for (int i = input_target_ids.size() - 1; i >= 0; i--) {
		if (ObjectDB::get_instance(input_target_ids[i]) == nullptr) {
			input_target_ids.remove_at(i);
		}
	}
}

void ZPlayer::set_input_targets(const Array &p_targets) {
	input_target_ids.clear();
	for (int i = 0; i < p_targets.size(); i++) {
		input_target_add(Object::cast_to<Object>(p_targets[i]));
	}
	emit_changed();
}

Array ZPlayer::get_input_targets() const {
	Array out;
	for (int i = 0; i < input_target_ids.size(); i++) {
		Object *obj = ObjectDB::get_instance(input_target_ids[i]);
		if (obj != nullptr) {
			out.push_back(obj);
		}
	}
	return out;
}

bool ZPlayer::input_target_add(Object *p_target) {
	if (p_target == nullptr) {
		return false;
	}
	if (!p_target->has_method(INPUT_TRIGGERED_METHOD)) {
		UtilityFunctions::push_warning(
				String("ZPlayer.input_target_add: ") + p_target->get_class() +
				" has no input_triggered(player, action, state, dt) method — not added.");
		return false;
	}
	if (has_input_target(p_target)) {
		return false;
	}
	input_target_ids.push_back(ObjectID(p_target->get_instance_id()));
	emit_changed();
	return true;
}

void ZPlayer::input_target_remove(Object *p_target) {
	if (p_target == nullptr) {
		return;
	}
	const int idx = input_target_ids.find(ObjectID(p_target->get_instance_id()));
	if (idx < 0) {
		return;
	}
	input_target_ids.remove_at(idx);
	emit_changed();
}

void ZPlayer::input_targets_clear() {
	if (input_target_ids.is_empty()) {
		return;
	}
	input_target_ids.clear();
	emit_changed();
}

bool ZPlayer::has_input_target(Object *p_target) const {
	return p_target != nullptr && input_target_ids.find(ObjectID(p_target->get_instance_id())) >= 0;
}

void ZPlayer::input_action(const StringName &p_action, EInputState p_state, double p_delta) {
	if (input_target_ids.is_empty()) {
		return;
	}
	_prune_input_targets();

	// Snapshot: a target may add/remove targets (or free itself) while handling input.
	const Vector<ObjectID> targets = input_target_ids;
	for (int i = 0; i < targets.size(); i++) {
		Object *obj = ObjectDB::get_instance(targets[i]);
		if (obj == nullptr) {
			continue;
		}
		// No input_is_enabled() means "always enabled".
		if (obj->has_method(INPUT_IS_ENABLED_METHOD) && !(bool)obj->call(INPUT_IS_ENABLED_METHOD)) {
			continue;
		}
		obj->call(INPUT_TRIGGERED_METHOD, this, p_action, (int)p_state, p_delta);
	}
}

void ZPlayer::input_begin(const StringName &p_action) {
	input_action(p_action, PRESSED, 0.0);
}

void ZPlayer::input_update(const StringName &p_action, double p_delta) {
	input_action(p_action, DOWN, p_delta);
}

void ZPlayer::input_end(const StringName &p_action) {
	input_action(p_action, RELEASED, 0.0);
}

void ZPlayer::set_selected_data_option(ZDataOption *p_option) {
	if (p_option == nullptr) {
		selected_data_option_id = ObjectID();
	} else {
		selected_data_option_id = ObjectID(p_option->get_instance_id());
	}
	emit_changed();
}

ZDataOption *ZPlayer::get_selected_data_option() const {
	if (selected_data_option_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<ZDataOption>(ObjectDB::get_instance(selected_data_option_id));
}

void ZPlayer::clear_selected_data_option() {
	set_selected_data_option(nullptr);
}
