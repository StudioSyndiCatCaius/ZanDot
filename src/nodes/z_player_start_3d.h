#pragma once

#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Spawn point for a player pawn. player_index maps to Z.players[i].
// linked_map + link_id select this start when arriving via ZSceneTransit from that map.
// Editor-only: draws a cylinder, forward arrow, and "Player Start" label.
class ZPlayerStart3D : public Marker3D {
	GDCLASS(ZPlayerStart3D, Marker3D)

private:
	int player_index = 0;
	// When set, preferred spawn when transit arrived from this map (origin).
	Ref<PackedScene> linked_map;
	// Optional id to pair with ZSceneTransit.link_id (and peer starts).
	StringName link_id;
	Node *editor_visual = nullptr;

	void _ensure_editor_visual();
	void _clear_editor_visual();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_player_index(int p_index);
	int get_player_index() const;

	void set_linked_map(const Ref<PackedScene> &p_map);
	Ref<PackedScene> get_linked_map() const;

	void set_link_id(const StringName &p_id);
	StringName get_link_id() const;

	ZPlayerStart3D();
	~ZPlayerStart3D();
};

} // namespace godot
