#include "z_save_game.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZSaveGame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_current_scene", "scene"), &ZSaveGame::set_current_scene);
	ClassDB::bind_method(D_METHOD("get_current_scene"), &ZSaveGame::get_current_scene);

	ClassDB::bind_method(D_METHOD("set_current_transform", "transform"), &ZSaveGame::set_current_transform);
	ClassDB::bind_method(D_METHOD("get_current_transform"), &ZSaveGame::get_current_transform);

	ClassDB::bind_method(D_METHOD("set_bgm_track", "track"), &ZSaveGame::set_bgm_track);
	ClassDB::bind_method(D_METHOD("get_bgm_track"), &ZSaveGame::get_bgm_track);

	ClassDB::bind_method(D_METHOD("set_bgm_variant", "variant"), &ZSaveGame::set_bgm_variant);
	ClassDB::bind_method(D_METHOD("get_bgm_variant"), &ZSaveGame::get_bgm_variant);

	ClassDB::bind_method(D_METHOD("set_bgm_locked", "locked"), &ZSaveGame::set_bgm_locked);
	ClassDB::bind_method(D_METHOD("get_bgm_locked"), &ZSaveGame::get_bgm_locked);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_scene"), "set_current_scene", "get_current_scene");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "current_transform"), "set_current_transform", "get_current_transform");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bgm_track", PROPERTY_HINT_RESOURCE_TYPE, "ZBgmTrack"), "set_bgm_track", "get_bgm_track");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bgm_variant"), "set_bgm_variant", "get_bgm_variant");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bgm_locked"), "set_bgm_locked", "get_bgm_locked");
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

void ZSaveGame::set_bgm_track(const Ref<ZBgmTrack> &p_track) {
	if (bgm_track == p_track) {
		return;
	}
	bgm_track = p_track;
	emit_changed();
}

Ref<ZBgmTrack> ZSaveGame::get_bgm_track() const {
	return bgm_track;
}

void ZSaveGame::set_bgm_variant(const StringName &p_variant) {
	if (bgm_variant == p_variant) {
		return;
	}
	bgm_variant = p_variant;
	emit_changed();
}

StringName ZSaveGame::get_bgm_variant() const {
	return bgm_variant;
}

void ZSaveGame::set_bgm_locked(bool p_locked) {
	if (bgm_locked == p_locked) {
		return;
	}
	bgm_locked = p_locked;
	emit_changed();
}

bool ZSaveGame::get_bgm_locked() const {
	return bgm_locked;
}
