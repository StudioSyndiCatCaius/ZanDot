#pragma once

#include "z_game_state.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Per-level (or per-root-scene) game mode. Spawns players and drives ZGameState.
class ZGameMode : public Node {
	GDCLASS(ZGameMode, Node)

private:
	Ref<PackedScene> default_player_scene;
	Ref<PackedScene> default_state;
	// State to enter on first setup(), instead of default_state. Falls back to the
	// project setting zandot/default_starting_state, then to default_state, if unset/invalid.
	Ref<PackedScene> starting_state;
	// Seconds to wait after setup before spawning player pawns (lets level assign link maps, etc.).
	float player_spawn_delay = 0.05f;
	ZGameState *active_state = nullptr;
	bool setup_done = false;
	bool spawn_scheduled = false;

	void _clear_active_state();
	void _connect_active_state_watch(ZGameState *p_state);
	void _disconnect_active_state_watch(ZGameState *p_state);
	void _on_active_state_tree_exiting();
	void _schedule_spawn_players();
	String _settings_default_starting_state() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_default_player_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_default_player_scene() const;

	void set_default_state(const Ref<PackedScene> &p_state);
	Ref<PackedScene> get_default_state() const;

	void set_starting_state(const Ref<PackedScene> &p_state);
	Ref<PackedScene> get_starting_state() const;

	void set_player_spawn_delay(float p_delay);
	float get_player_spawn_delay() const;

	ZGameState *get_active_state() const;

	// Called by Z when this mode is selected for the current level.
	void setup();
	// Spawn / switch to a state scene instance (child of this mode). p_context is
	// passed through to the new state's on_state_enter and the on_state_change signal.
	void change_state_scene(const Ref<PackedScene> &p_state_scene, const Variant &p_context = Variant());
	// Activate an already-instanced ZGameState (reparents under this mode if needed).
	void change_state(ZGameState *p_state, const Variant &p_context = Variant());
	// Switch to default_state if set.
	void change_to_default_state(const Variant &p_context = Variant());
	// Switch to starting_state, falling back to zandot/default_starting_state, then default_state.
	void change_to_starting_state(const Variant &p_context = Variant());

	// Spawn pawns for Z players at matching ZPlayerStart3D markers
	// (uses pending ZSceneTransit match: linked_map / link_id when present).
	void spawn_players();

	ZGameMode();
	~ZGameMode();
};

} // namespace godot
