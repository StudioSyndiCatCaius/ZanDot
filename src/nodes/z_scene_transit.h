#pragma once

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Shared transit payload stored on Z while changing scenes.
struct ZSceneTransitPending {
	Ref<PackedScene> from_scene;
	Ref<PackedScene> to_scene;
	StringName link_id;
	bool is_3d = true;
	bool valid = false;
};

// 3D door/portal: transitions root scene via Z.Scene_Transit only.
// Destination spawn is handled by ZPlayerStart3D (linked_map + link_id), not this node.
// Area3D so overlapping the player pawn can fire the transit; if no collision shape
// child is present a default 1x2x1 box (matching the editor gizmo) is added at runtime.
class ZSceneTransit3D : public Area3D {
	GDCLASS(ZSceneTransit3D, Area3D)

private:
	Ref<PackedScene> linked_scene;
	StringName link_id;
	bool transit_on_player_enter = true;
	Node *editor_visual = nullptr;
	Node *auto_shape = nullptr;

	void _ensure_editor_visual();
	void _clear_editor_visual();
	void _ensure_collision_shape();
	void _on_body_entered(Node3D *p_body);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_linked_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_linked_scene() const;

	void set_link_id(const StringName &p_id);
	StringName get_link_id() const;

	void set_transit_on_player_enter(bool p_enabled);
	bool get_transit_on_player_enter() const;

	// Trigger transit to linked_scene (stores match info so ZPlayerStart can pick spawn).
	void transit(bool p_skip_fade_out = false, bool p_skip_fade_in = false);

	ZSceneTransit3D();
	~ZSceneTransit3D();
};

// 2D counterpart.
class ZSceneTransit2D : public Area2D {
	GDCLASS(ZSceneTransit2D, Area2D)

private:
	Ref<PackedScene> linked_scene;
	StringName link_id;
	bool transit_on_player_enter = true;
	Node *auto_shape = nullptr;

	void _ensure_collision_shape();
	void _on_body_entered(Node2D *p_body);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_linked_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_linked_scene() const;

	void set_link_id(const StringName &p_id);
	StringName get_link_id() const;

	void set_transit_on_player_enter(bool p_enabled);
	bool get_transit_on_player_enter() const;

	void transit(bool p_skip_fade_out = false, bool p_skip_fade_in = false);

	virtual void _draw() override;

	ZSceneTransit2D();
	~ZSceneTransit2D();
};

} // namespace godot
