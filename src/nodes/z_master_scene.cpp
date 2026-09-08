#include "z_master_scene.h"

#include "z.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMasterScene::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_autoset_as_master_scene", "value"), &ZMasterScene::set_autoset_as_master_scene);
	ClassDB::bind_method(D_METHOD("get_autoset_as_master_scene"), &ZMasterScene::get_autoset_as_master_scene);

	ClassDB::bind_method(D_METHOD("hook_play_scene_transit", "fade_out"), &ZMasterScene::hook_play_scene_transit);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autoset_as_master_scene"), "set_autoset_as_master_scene", "get_autoset_as_master_scene");

	GDVIRTUAL_BIND(_hook_play_scene_transit, "fade_out");
}

ZMasterScene::ZMasterScene() {
}

ZMasterScene::~ZMasterScene() {
}

void ZMasterScene::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		if (autoset_as_master_scene && !Engine::get_singleton()->is_editor_hint()) {
			Z *z = Z::get_singleton();
			if (z != nullptr) {
				z->set_master_scene(this);
			}
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		Z *z = Z::get_singleton();
		if (z != nullptr && z->get_master_scene() == this) {
			z->set_master_scene(nullptr);
		}
	}
}

void ZMasterScene::set_autoset_as_master_scene(bool p_value) {
	autoset_as_master_scene = p_value;
}

bool ZMasterScene::get_autoset_as_master_scene() const {
	return autoset_as_master_scene;
}

Signal ZMasterScene::hook_play_scene_transit(bool p_fade_out) {
	Signal result;
	if (GDVIRTUAL_CALL(_hook_play_scene_transit, p_fade_out, result)) {
		return result;
	}
	// Also try a script method that returns a Signal (non-virtual convention).
	if (has_method("_hook_play_scene_transit")) {
		Variant ret = call("_hook_play_scene_transit", p_fade_out);
		if (ret.get_type() == Variant::SIGNAL) {
			return ret;
		}
	}
	return Signal();
}
