#include "z_game_mode.h"

#include "z.h"
#include "z_player_start_3d.h"
#include "z_scene_transit.h"
#include "z_settings.h"
#include "res/z_player.h"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
bool packed_scenes_match(const Ref<PackedScene> &p_a, const Ref<PackedScene> &p_b) {
	if (p_a == p_b) {
		return true;
	}
	if (p_a.is_null() || p_b.is_null()) {
		return false;
	}
	const String path_a = p_a->get_path();
	const String path_b = p_b->get_path();
	return !path_a.is_empty() && path_a == path_b;
}

// Prefer a start matching pending transit (linked_map = origin map, link_id), else default start.
ZPlayerStart3D *find_player_start(const TypedArray<Node> &p_starts, int p_player_index, const ZSceneTransitPending &p_pending) {
	// 1) Transit match: linked_map matches origin scene + optional link_id + player_index.
	if (p_pending.valid) {
		ZPlayerStart3D *link_only = nullptr;
		for (int s = 0; s < p_starts.size(); s++) {
			ZPlayerStart3D *candidate = Object::cast_to<ZPlayerStart3D>(p_starts[s]);
			if (candidate == nullptr || candidate->get_player_index() != p_player_index) {
				continue;
			}
			if (p_pending.link_id != StringName() && candidate->get_link_id() != p_pending.link_id) {
				continue;
			}
			if (p_pending.from_scene.is_valid() && candidate->get_linked_map().is_valid()) {
				if (packed_scenes_match(candidate->get_linked_map(), p_pending.from_scene)) {
					return candidate;
				}
				continue;
			}
			// link_id matched (or both empty) but no map pair — keep as fallback for this pass.
			if (p_pending.link_id != StringName() && candidate->get_link_id() == p_pending.link_id) {
				if (link_only == nullptr) {
					link_only = candidate;
				}
			}
		}
		if (link_only != nullptr) {
			return link_only;
		}
	}

	// 2) Default: matching player_index, prefer starts without a linked_map (level entry).
	ZPlayerStart3D *by_index = nullptr;
	ZPlayerStart3D *by_index_unlinked = nullptr;
	for (int s = 0; s < p_starts.size(); s++) {
		ZPlayerStart3D *candidate = Object::cast_to<ZPlayerStart3D>(p_starts[s]);
		if (candidate == nullptr || candidate->get_player_index() != p_player_index) {
			continue;
		}
		if (by_index == nullptr) {
			by_index = candidate;
		}
		if (candidate->get_linked_map().is_null() && by_index_unlinked == nullptr) {
			by_index_unlinked = candidate;
		}
	}
	if (by_index_unlinked != nullptr) {
		return by_index_unlinked;
	}
	if (by_index != nullptr) {
		return by_index;
	}

	// 3) Fallback: first start at index 0, else any start.
	for (int s = 0; s < p_starts.size(); s++) {
		ZPlayerStart3D *candidate = Object::cast_to<ZPlayerStart3D>(p_starts[s]);
		if (candidate != nullptr && candidate->get_player_index() == 0) {
			return candidate;
		}
	}
	if (p_starts.size() > 0) {
		return Object::cast_to<ZPlayerStart3D>(p_starts[0]);
	}
	return nullptr;
}
} // namespace

void ZGameMode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_default_player_scene", "scene"), &ZGameMode::set_default_player_scene);
	ClassDB::bind_method(D_METHOD("get_default_player_scene"), &ZGameMode::get_default_player_scene);

	ClassDB::bind_method(D_METHOD("set_default_state", "state"), &ZGameMode::set_default_state);
	ClassDB::bind_method(D_METHOD("get_default_state"), &ZGameMode::get_default_state);

	ClassDB::bind_method(D_METHOD("set_starting_state", "state"), &ZGameMode::set_starting_state);
	ClassDB::bind_method(D_METHOD("get_starting_state"), &ZGameMode::get_starting_state);

	ClassDB::bind_method(D_METHOD("set_player_spawn_delay", "delay"), &ZGameMode::set_player_spawn_delay);
	ClassDB::bind_method(D_METHOD("get_player_spawn_delay"), &ZGameMode::get_player_spawn_delay);

	ClassDB::bind_method(D_METHOD("get_active_state"), &ZGameMode::get_active_state);

	ClassDB::bind_method(D_METHOD("setup"), &ZGameMode::setup);
	ClassDB::bind_method(D_METHOD("change_state_scene", "state_scene", "context"), &ZGameMode::change_state_scene, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("change_state", "state", "context"), &ZGameMode::change_state, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("change_to_default_state", "context"), &ZGameMode::change_to_default_state, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("change_to_starting_state", "context"), &ZGameMode::change_to_starting_state, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("spawn_players"), &ZGameMode::spawn_players);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_player_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_default_player_scene", "get_default_player_scene");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_state", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_default_state", "get_default_state");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "starting_state", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_starting_state", "get_starting_state");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "player_spawn_delay", PROPERTY_HINT_RANGE, "0,5,0.01,or_greater"), "set_player_spawn_delay", "get_player_spawn_delay");
	// Read-only runtime property (empty setter).
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "active_state", PROPERTY_HINT_NODE_TYPE, "ZGameState", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_active_state");

	ADD_SIGNAL(MethodInfo("on_state_change", PropertyInfo(Variant::OBJECT, "new_state", PROPERTY_HINT_NODE_TYPE, "ZGameState"), PropertyInfo(Variant::OBJECT, "old_state", PROPERTY_HINT_NODE_TYPE, "ZGameState"), PropertyInfo(Variant::NIL, "context", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
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
		// Tear down without auto-reverting to default (mode is leaving the tree).
		if (active_state != nullptr) {
			_disconnect_active_state_watch(active_state);
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

void ZGameMode::set_starting_state(const Ref<PackedScene> &p_state) {
	starting_state = p_state;
}

Ref<PackedScene> ZGameMode::get_starting_state() const {
	return starting_state;
}

void ZGameMode::set_player_spawn_delay(float p_delay) {
	player_spawn_delay = Math::max(p_delay, 0.0f);
}

float ZGameMode::get_player_spawn_delay() const {
	return player_spawn_delay;
}

String ZGameMode::_settings_default_starting_state() const {
	return ZSettings::get(ZSettings::DEFAULT_STARTING_STATE, String(""));
}

ZGameState *ZGameMode::get_active_state() const {
	return active_state;
}

void ZGameMode::_connect_active_state_watch(ZGameState *p_state) {
	if (p_state == nullptr) {
		return;
	}
	const Callable cb = callable_mp(this, &ZGameMode::_on_active_state_tree_exiting);
	if (!p_state->is_connected("tree_exiting", cb)) {
		p_state->connect("tree_exiting", cb);
	}
}

void ZGameMode::_disconnect_active_state_watch(ZGameState *p_state) {
	if (p_state == nullptr) {
		return;
	}
	const Callable cb = callable_mp(this, &ZGameMode::_on_active_state_tree_exiting);
	if (p_state->is_connected("tree_exiting", cb)) {
		p_state->disconnect("tree_exiting", cb);
	}
}

void ZGameMode::_on_active_state_tree_exiting() {
	// Active state left the tree outside of change_state/_clear_active_state
	// (queue_free, remove_child, scene unload of the state only, etc.).
	if (active_state == nullptr) {
		return;
	}

	ZGameState *prev = active_state;
	active_state = nullptr;
	// Connection dies with the node; no need to disconnect.

	// Prefer a proper exit if the node is still valid during tree_exiting.
	if (prev != nullptr) {
		prev->state_exit();
	}

	// Revert to default only while this mode is still an active tree member.
	if (is_inside_tree() && setup_done) {
		call_deferred("change_to_default_state");
	}
}

void ZGameMode::_clear_active_state() {
	if (active_state == nullptr) {
		return;
	}
	ZGameState *prev = active_state;
	// Disconnect before free so tree_exiting does not auto-revert mid switch.
	_disconnect_active_state_watch(prev);
	active_state = nullptr;
	prev->state_exit();
	prev->queue_free();
}

void ZGameMode::_schedule_spawn_players() {
	if (spawn_scheduled) {
		return;
	}
	if (player_spawn_delay <= 0.0f) {
		spawn_players();
		return;
	}
	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		spawn_players();
		return;
	}
	spawn_scheduled = true;
	Ref<SceneTreeTimer> timer = tree->create_timer(player_spawn_delay);
	if (timer.is_valid()) {
		timer->connect("timeout", callable_mp(this, &ZGameMode::spawn_players));
	} else {
		spawn_scheduled = false;
		spawn_players();
	}
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

	// Delay pawn spawn so the level can finish wiring link maps / transit data first.
	_schedule_spawn_players();

	if (active_state == nullptr) {
		// During scene transit, optionally defer starting/default state until fade-in completes.
		if (z != nullptr && z->should_block_default_state()) {
			z->note_deferred_starting_state();
		} else {
			change_to_starting_state();
		}
	}
}

void ZGameMode::change_state_scene(const Ref<PackedScene> &p_state_scene, const Variant &p_context) {
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

	change_state(state, p_context);
}

void ZGameMode::change_state(ZGameState *p_state, const Variant &p_context) {
	if (p_state == nullptr) {
		UtilityFunctions::push_warning("ZGameMode.change_state: state is null.");
		return;
	}

	if (active_state == p_state) {
		return;
	}

	ZGameState *old_state = active_state;
	_clear_active_state();

	// Stash context before add_child so _ready / early hooks can read it.
	p_state->set_context(p_context);

	if (p_state->get_parent() != this) {
		if (p_state->get_parent() != nullptr) {
			p_state->get_parent()->remove_child(p_state);
		}
		add_child(p_state);
	}

	active_state = p_state;
	_connect_active_state_watch(active_state);
	active_state->state_enter(p_context);
	emit_signal("on_state_change", active_state, old_state, p_context);
}

void ZGameMode::change_to_default_state(const Variant &p_context) {
	if (default_state.is_null()) {
		return;
	}
	change_state_scene(default_state, p_context);
}

void ZGameMode::change_to_starting_state(const Variant &p_context) {
	Ref<PackedScene> scene_to_use = starting_state;

	if (scene_to_use.is_null()) {
		const String path = _settings_default_starting_state();
		if (!path.is_empty() && ResourceLoader::get_singleton()->exists(path)) {
			scene_to_use = ResourceLoader::get_singleton()->load(path);
		}
	}

	if (scene_to_use.is_valid()) {
		change_state_scene(scene_to_use, p_context);
		return;
	}

	change_to_default_state(p_context);
}

void ZGameMode::spawn_players() {
	spawn_scheduled = false;

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

	// Snapshot + clear pending transit so delayed spawn still sees it, and finish won't race-clear.
	const ZSceneTransitPending pending = z->get_pending_scene_transit();
	if (pending.valid) {
		z->clear_pending_scene_transit();
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
		ZPlayerStart3D *start = find_player_start(starts, pindex, pending);

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
