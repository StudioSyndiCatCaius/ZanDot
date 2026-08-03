#include "z_game_mode.h"

#include "z.h"
#include "z_player_start_3d.h"
#include "res/z_player.h"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZGameMode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_default_player_scene", "scene"), &ZGameMode::set_default_player_scene);
	ClassDB::bind_method(D_METHOD("get_default_player_scene"), &ZGameMode::get_default_player_scene);

	ClassDB::bind_method(D_METHOD("set_default_state", "state"), &ZGameMode::set_default_state);
	ClassDB::bind_method(D_METHOD("get_default_state"), &ZGameMode::get_default_state);

	ClassDB::bind_method(D_METHOD("get_active_state"), &ZGameMode::get_active_state);

	ClassDB::bind_method(D_METHOD("setup"), &ZGameMode::setup);
	ClassDB::bind_method(D_METHOD("change_state_scene", "state_scene"), &ZGameMode::change_state_scene);
	ClassDB::bind_method(D_METHOD("change_state", "state"), &ZGameMode::change_state);
	ClassDB::bind_method(D_METHOD("change_to_default_state"), &ZGameMode::change_to_default_state);
	ClassDB::bind_method(D_METHOD("spawn_players"), &ZGameMode::spawn_players);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_player_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_default_player_scene", "get_default_player_scene");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_state", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_default_state", "get_default_state");
	// Read-only runtime property (empty setter).
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "active_state", PROPERTY_HINT_NODE_TYPE, "ZGameState", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_active_state");
}

ZGameMode::ZGameMode() {
}

ZGameMode::~ZGameMode() {
}

void ZGameMode::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		add_to_group("z_game_mode");
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		remove_from_group("z_game_mode");
		if (active_state != nullptr && !active_state->is_inside_tree()) {
			active_state = nullptr;
		}
	}
}

void ZGameMode::set_default_player_scene(const Ref<PackedScene> &p_scene) {
	default_player_scene = p_scene;
}

Ref<PackedScene> ZGameMode::get_default_player_scene() const {
	return default_player_scene;
}

void ZGameMode::set_default_state(const Ref<PackedScene> &p_state) {
	default_state = p_state;
}

Ref<PackedScene> ZGameMode::get_default_state() const {
	return default_state;
}

ZGameState *ZGameMode::get_active_state() const {
	return active_state;
}

void ZGameMode::_clear_active_state() {
	if (active_state == nullptr) {
		return;
	}
	ZGameState *prev = active_state;
	active_state = nullptr;
	prev->state_exit();
	prev->queue_free();
}

void ZGameMode::setup() {
	if (setup_done) {
		return;
	}
	setup_done = true;

	Z *z = Z::get_singleton();
	if (z != nullptr && z->get_players().size() == 0) {
		z->player_create();
	}

	spawn_players();

	if (active_state == nullptr) {
		change_to_default_state();
	}
}

void ZGameMode::change_state_scene(const Ref<PackedScene> &p_state_scene) {
	if (p_state_scene.is_null()) {
		UtilityFunctions::push_warning("ZGameMode.change_state_scene: state scene is null.");
		return;
	}

	Node *inst = p_state_scene->instantiate();
	ZGameState *state = Object::cast_to<ZGameState>(inst);
	if (state == nullptr) {
		UtilityFunctions::push_error("ZGameMode.change_state_scene: root must be a ZGameState (got " + (inst ? inst->get_class() : String("null")) + ").");
		if (inst != nullptr) {
			inst->queue_free();
		}
		return;
	}

	change_state(state);
}

void ZGameMode::change_state(ZGameState *p_state) {
	if (p_state == nullptr) {
		UtilityFunctions::push_warning("ZGameMode.change_state: state is null.");
		return;
	}

	if (active_state == p_state) {
		return;
	}

	_clear_active_state();

	if (p_state->get_parent() != this) {
		if (p_state->get_parent() != nullptr) {
			p_state->get_parent()->remove_child(p_state);
		}
		add_child(p_state);
	}

	active_state = p_state;
	active_state->state_enter();
}

void ZGameMode::change_to_default_state() {
	if (default_state.is_null()) {
		return;
	}
	change_state_scene(default_state);
}

void ZGameMode::spawn_players() {
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return;
	}
	if (default_player_scene.is_null()) {
		return;
	}

	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		return;
	}

	TypedArray<Node> starts = tree->get_nodes_in_group("z_player_start");
	const TypedArray<ZPlayer> players = z->get_players();

	for (int i = 0; i < players.size(); i++) {
		Ref<ZPlayer> player = players[i];
		if (player.is_null()) {
			continue;
		}
		if (player->get_possessed_scene() != nullptr) {
			continue;
		}

		const int pindex = player->get_player_index();
		ZPlayerStart3D *start = nullptr;
		for (int s = 0; s < starts.size(); s++) {
			ZPlayerStart3D *candidate = Object::cast_to<ZPlayerStart3D>(starts[s]);
			if (candidate != nullptr && candidate->get_player_index() == pindex) {
				start = candidate;
				break;
			}
		}
		// Fallback: first start with index 0, or any start if only one player.
		if (start == nullptr && starts.size() > 0) {
			for (int s = 0; s < starts.size(); s++) {
				ZPlayerStart3D *candidate = Object::cast_to<ZPlayerStart3D>(starts[s]);
				if (candidate != nullptr && candidate->get_player_index() == 0) {
					start = candidate;
					break;
				}
			}
			if (start == nullptr) {
				start = Object::cast_to<ZPlayerStart3D>(starts[0]);
			}
		}

		Node *pawn = default_player_scene->instantiate();
		if (pawn == nullptr) {
			UtilityFunctions::push_error("ZGameMode.spawn_players: failed to instantiate default_player_scene.");
			continue;
		}

		Node *parent = get_parent() != nullptr ? get_parent() : this;
		parent->add_child(pawn);

		if (start != nullptr) {
			Node3D *pawn3d = Object::cast_to<Node3D>(pawn);
			if (pawn3d != nullptr) {
				pawn3d->set_global_transform(start->get_global_transform());
			}
		}

		player->possess(pawn);
	}
}
