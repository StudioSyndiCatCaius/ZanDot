#include "z_save_game.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZSaveGame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_current_scene", "scene"), &ZSaveGame::set_current_scene);
	ClassDB::bind_method(D_METHOD("get_current_scene"), &ZSaveGame::get_current_scene);

	ClassDB::bind_method(D_METHOD("set_current_transform", "transform"), &ZSaveGame::set_current_transform);
	ClassDB::bind_method(D_METHOD("get_current_transform"), &ZSaveGame::get_current_transform);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_scene"), "set_current_scene", "get_current_scene");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "current_transform"), "set_current_transform", "get_current_transform");
}

ZSaveGame::ZSaveGame() {
}

ZSaveGame::~ZSaveGame() {
}

void ZSaveGame::set_current_scene(const String &p_scene) {
	if (current_scene == p_scene) {
		return;
	}
	current_scene = p_scene;
	emit_changed();
}

String ZSaveGame::get_current_scene() const {
	return current_scene;
}

void ZSaveGame::set_current_transform(const Transform3D &p_transform) {
	current_transform = p_transform;
	emit_changed();
}

Transform3D ZSaveGame::get_current_transform() const {
	return current_transform;
}
