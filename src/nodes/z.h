#pragma once

#include "res/z_global_condition.h"
#include "res/z_global_script.h"
#include "res/z_player.h"
#include "res/z_save_game.h"
#include "res/z_save_global.h"
#include "z_game_mode.h"
#include "z_scene_transit.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/signal.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class ZMasterScene;

// Global game state manager (Engine singleton "Z").
// Handles saves, players, and per-level game mode resolution.
// A Node so it appears in the running game's tree (root/Z) and its state is
// visible/editable from the editor's Remote scene tree + inspector.
class Z : public Node {
	GDCLASS(Z, Node)

public:
	// Where SaveGame/Global files are written; see set_save_location.
	enum SaveLocation {
		SAVE_LOCATION_USER = 0, // OS "Saved Games/Zandot/{game}" (falls back to user:// if unresolvable).
		SAVE_LOCATION_GAMEDIR = 1, // {executable_dir}/save/
	};

	enum SceneTransitState {
		SCENE_TRANSIT_NONE = 0,
		SCENE_TRANSIT_FADE_OUT = 1,
		SCENE_TRANSIT_TRANSITING = 2,
		SCENE_TRANSIT_FADE_IN = 3,
	};

private:
	static Z *singleton;

	Ref<ZSaveGame> save_game;
	Ref<ZSaveGlobal> save_global;
	TypedArray<ZPlayer> players;
	ObjectID current_game_mode_id;
	ObjectID master_scene_id;
	SaveLocation save_location = SAVE_LOCATION_USER;

	bool listening_tree = false;
	bool bootstrapped = false;
	// Re-entrancy guard: loading the global save runs ResourceLoader, and a
	// custom SaveGlobal script's _init() could read Z.save_global right back.
	bool loading_save_global = false;

	// Scene transit state machine.
	SceneTransitState scene_transit_state = SCENE_TRANSIT_NONE;
	Ref<PackedScene> pending_transit_scene;
	bool pending_skip_fade_in = false;
	bool pending_skip_fade_out = false;
	ZSceneTransitPending pending_scene_transit_match;
	bool deferred_starting_state_after_transit = false;

	// Z_Obj NameVars cache: Object instance id (int64) -> PackedStringArray.
	Dictionary object_name_vars_cache;

	void _ensure_tree_hooks();
	// Parents the singleton under the SceneTree root so it shows up in the tree.
	void _attach_to_tree();
	void _on_tree_changed_frame();
	void _on_scene_changed();
	void _resolve_game_mode_for_current_scene();
	ZGameMode *_find_game_mode_in(Node *p_root, int &r_count) const;
	String _game_name() const;
	String _save_root_dir() const;
	String _slot_dir(int p_slot) const;
	String _slot_path(int p_slot) const;
	String _slot_screenshot_path(int p_slot) const;
	String _global_path() const;
	Ref<ZSaveGlobal> _save_global_create_and_persist();
	Ref<ZSaveGlobal> _save_global_load_inner();
	String _settings_save_prefix() const;
	String _settings_global_name() const;
	String _settings_default_game_mode() const;
	String _settings_savegame_script_class() const;
	String _settings_saveglobal_script_class() const;
	bool _settings_block_default_state_during_scene_transit() const;
	void _save_slot_screenshot(int p_slot) const;

	void _set_scene_transit_state(SceneTransitState p_state);
	void _scene_transit_begin_fade_out();
	void _scene_transit_on_fade_out_done();
	void _scene_transit_do_change();
	void _scene_transit_after_scene_ready();
	void _scene_transit_begin_fade_in();
	void _scene_transit_on_fade_in_done();
	void _scene_transit_finish();
	bool _connect_one_shot_signal(const Signal &p_signal, const Callable &p_callable);

protected:
	static void _bind_methods();

public:
	static Z *get_singleton();
	static void register_settings();
	static void create_singleton();
	static void destroy_singleton();

	// --- Properties ---
	// Both save getters are self-healing: a SaveGame/SaveGlobal always exists.
	// save_game is created on first access, save_global is loaded from disk
	// (or created if there is no file yet). Hence non-const.
	void set_save_game(const Ref<ZSaveGame> &p_save);
	Ref<ZSaveGame> get_save_game();

	void set_save_global(const Ref<ZSaveGlobal> &p_save);
	Ref<ZSaveGlobal> get_save_global();

	void set_save_location(SaveLocation p_location);
	SaveLocation get_save_location() const;

	void set_players(const TypedArray<ZPlayer> &p_players);
	TypedArray<ZPlayer> get_players() const;

	ZGameMode *get_current_game_mode() const;
	void set_current_game_mode(ZGameMode *p_mode);

	void set_master_scene(ZMasterScene *p_scene);
	ZMasterScene *get_master_scene() const;

	SceneTransitState get_scene_transit_state() const;
	bool is_scene_transiting() const;

	// Pending match info for ZPlayerStart destination spawn after ZSceneTransit.
	void set_pending_scene_transit(const ZSceneTransitPending &p_pending);
	ZSceneTransitPending get_pending_scene_transit() const;
	void clear_pending_scene_transit();

	// True while transit is in progress and project setting blocks default state.
	bool should_block_default_state() const;
	// Called by ZGameMode when it skipped starting state due to transit.
	void note_deferred_starting_state();

	// --- NameVars cache (Z_Obj) ---
	bool settings_cache_object_name_vars() const;
	bool object_name_vars_cache_get(uint64_t p_instance_id, PackedStringArray &r_out) const;
	void object_name_vars_cache_set(uint64_t p_instance_id, const PackedStringArray &p_vars);
	void object_name_vars_cache_clear();

	// --- Save API ---
	Ref<ZSaveGame> save_game_create();
	Ref<ZSaveGame> save_game_load_from_path(const String &p_path);
	Ref<ZSaveGame> save_game_load_from_slot(int p_slot);
	bool save_game_save_to_path(const Ref<ZSaveGame> &p_save, const String &p_path);
	// Writes to {save_root}/{prefix}{slot,000}/{prefix}{slot,000}.sav and also
	// captures a screenshot alongside it (same folder, .png).
	bool save_game_save_to_slot(const Ref<ZSaveGame> &p_save, int p_slot);
	// Resolved paths for a slot — for UI (existence checks, screenshot previews, deletion).
	String save_game_slot_dir(int p_slot) const;
	String save_game_slot_path(int p_slot) const;
	String save_game_screenshot_path(int p_slot) const;

	Ref<ZSaveGlobal> save_global_create();
	Ref<ZSaveGlobal> save_global_load();
	bool save_global_save(const Ref<ZSaveGlobal> &p_save = Ref<ZSaveGlobal>());

	// --- Player API ---
	Ref<ZPlayer> player_create();
	void player_remove(const Ref<ZPlayer> &p_player);
	void player_remove_index(int p_player_index);
	Node *player_get_pawn(int p_player_index = 0) const;

	// --- Global condition / script ---
	bool global_condition_check(const Ref<ZGlobalCondition> &p_condition);
	bool global_condition_check_list(const TypedArray<ZGlobalCondition> &p_conditions, bool p_if_one_true = false);
	void global_script_run(const Ref<ZGlobalScript> &p_script);
	void global_script_run_list(const TypedArray<ZGlobalScript> &p_scripts);

	// --- Global events ---
	// Broadcast a named event with optional payload. Listeners connect to on_gEvent.
	void g_event(const String &p_event, const Dictionary &p_meta = Dictionary());

	// --- Scene transit ---
	// Forwards to master_scene.hook_play_scene_transit when available.
	Signal hook_play_scene_transit(bool p_fade_out);
	// Async scene change with optional fade hooks. Emits on_scene_transit_state_change.
	void scene_transit(const Ref<PackedScene> &p_new_scene, bool p_skip_fade_out = false, bool p_skip_fade_in = false);

	// Level / mode
	void on_level_start();

	// Call once main loop exists (from init or deferred).
	void bootstrap();

	Z();
	~Z();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::Z::SaveLocation);
VARIANT_ENUM_CAST(godot::Z::SceneTransitState);
