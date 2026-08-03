#include "z_player.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>

using namespace godot;

void ZPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZPlayer::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZPlayer::get_player_index);

	ClassDB::bind_method(D_METHOD("set_control_rotation", "rotation"), &ZPlayer::set_control_rotation);
	ClassDB::bind_method(D_METHOD("get_control_rotation"), &ZPlayer::get_control_rotation);

	ClassDB::bind_method(D_METHOD("possess", "scene"), &ZPlayer::possess);
	ClassDB::bind_method(D_METHOD("unpossess"), &ZPlayer::unpossess);
	ClassDB::bind_method(D_METHOD("get_possessed_scene"), &ZPlayer::get_possessed_scene);
	ClassDB::bind_method(D_METHOD("set_possessed_scene", "scene"), &ZPlayer::set_possessed_scene);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index"), "set_player_index", "get_player_index");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "control_rotation"), "set_control_rotation", "get_control_rotation");
	// possessed_scene is runtime-only; expose for scripts but do not serialize.
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "possessed_scene", PROPERTY_HINT_NODE_TYPE, "Node", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE), "set_possessed_scene", "get_possessed_scene");
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
