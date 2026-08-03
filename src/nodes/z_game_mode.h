#pragma once

#include "z_game_state.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

// Per-level (or per-root-scene) game mode. Spawns players and drives ZGameState.
class ZGameMode : public Node {
	GDCLASS(ZGameMode, Node)

private:
	Ref<PackedScene> default_player_scene;
	Ref<PackedScene> default_state;
	ZGameState *active_state = nullptr;
	bool setup_done = false;

	void _clear_active_state();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_default_player_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_default_player_scene() const;

	void set_default_state(const Ref<PackedScene> &p_state);
	Ref<PackedScene> get_default_state() const;

	ZGameState *get_active_state() const;

	// Called by Z when this mode is selected for the current level.
	void setup();
	// Spawn / switch to a state scene instance (child of this mode).
	void change_state_scene(const Ref<PackedScene> &p_state_scene);
	// Activate an already-instanced ZGameState (reparents under this mode if needed).
	void change_state(ZGameState *p_state);
	// Switch to default_state if set.
	void change_to_default_state();

	// Spawn pawns for Z players at matching ZPlayerStart3D markers.
	void spawn_players();

	ZGameMode();
	~ZGameMode();
};

} // namespace godot
