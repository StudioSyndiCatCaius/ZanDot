#pragma once

#include "res/z_player.h"
#include "res/z_save_game.h"
#include "res/z_save_global.h"
#include "z_game_mode.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Global game state manager (Engine singleton "Z").
// Handles saves, players, and per-level game mode resolution.
class Z : public Object {
	GDCLASS(Z, Object)

private:
	static Z *singleton;

	Ref<ZSaveGame> save_game;
	Ref<ZSaveGlobal> save_global;
	TypedArray<ZPlayer> players;
	ObjectID current_game_mode_id;

	bool listening_tree = false;
	bool bootstrapped = false;

	void _ensure_tree_hooks();
	void _on_tree_changed_frame();
	void _on_scene_changed();
	void _resolve_game_mode_for_current_scene();
	ZGameMode *_find_game_mode_in(Node *p_root, int &r_count) const;
	String _slot_path(int p_slot) const;
	String _global_path() const;
	String _settings_save_prefix() const;
	String _settings_global_name() const;
	String _settings_default_game_mode() const;
	String _settings_savegame_script_class() const;
	String _settings_saveglobal_script_class() const;

protected:
	static void _bind_methods();

public:
	static Z *get_singleton();
	static void register_settings();
	static void create_singleton();
	static void destroy_singleton();

	// --- Properties ---
	void set_save_game(const Ref<ZSaveGame> &p_save);
	Ref<ZSaveGame> get_save_game() const;

	void set_save_global(const Ref<ZSaveGlobal> &p_save);
	Ref<ZSaveGlobal> get_save_global() const;

	void set_players(const TypedArray<ZPlayer> &p_players);
	TypedArray<ZPlayer> get_players() const;

	ZGameMode *get_current_game_mode() const;
	void set_current_game_mode(ZGameMode *p_mode);

	// --- Save API ---
	Ref<ZSaveGame> save_game_create();
	Ref<ZSaveGame> save_game_load_from_path(const String &p_path);
	Ref<ZSaveGame> save_game_load_from_slot(int p_slot);
	bool save_game_save_to_path(const Ref<ZSaveGame> &p_save, const String &p_path);
	bool save_game_save_to_slot(const Ref<ZSaveGame> &p_save, int p_slot);

	Ref<ZSaveGlobal> save_global_create();
	Ref<ZSaveGlobal> save_global_load();
	bool save_global_save(const Ref<ZSaveGlobal> &p_save = Ref<ZSaveGlobal>());

	// --- Player API ---
	Ref<ZPlayer> player_create();
	void player_remove(const Ref<ZPlayer> &p_player);
	void player_remove_index(int p_player_index);
	Node *player_get_pawn_scene(int p_player_index = 0) const;

	// Level / mode
	void on_level_start();

	// Call once main loop exists (from init or deferred).
	void bootstrap();

	Z();
	~Z();
};

} // namespace godot
