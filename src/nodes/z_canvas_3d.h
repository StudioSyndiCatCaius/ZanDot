#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {

class Camera3D;
class CanvasItem;
class Control;
class Node2D;
class Viewport;

// Anchors 2D/Control children to this Node3D's world position projected onto the
// active Camera3D's viewport (screen-space nameplates, world markers, etc.).
// CanvasItem children are made top-level at runtime and driven each frame.
class ZCanvas3D : public Node3D {
	GDCLASS(ZCanvas3D, Node3D)

private:
	bool clamp_to_screen = false;
	// x = left, y = top, size.x = right, size.y = bottom (pixels inset from viewport).
	Rect2 screen_clamp_margins = Rect2(0, 0, 0, 0);
	Vector2 screen_offset = Vector2(0, 0);
	bool center_content = false;

	bool scale_by_distance = false;
	// Distance at which canvas scale is 1.0 (scale = reference / distance).
	float distance_scale_reference = 10.0f;
	float distance_scale_min = 0.25f;
	float distance_scale_max = 2.0f;

	bool use_distance_fade = false;
	// Distance where fade begins (alpha = 1).
	float distance_fade_begin = 20.0f;
	// Extra distance beyond begin until fully faded (alpha = 0, then hidden).
	float distance_fade_end = 10.0f;

	bool hide_when_behind_camera = true;

	struct ChildCache {
		Vector2 base_offset;
		Vector2 base_scale = Vector2(1, 1);
		Color base_modulate = Color(1, 1, 1, 1);
		bool prepared = false;
	};
	HashMap<ObjectID, ChildCache> child_cache;

	void _prepare_canvas_children();
	void _prepare_child(CanvasItem *p_item);
	void _update_canvas();
	Vector2 _content_size() const;
	CanvasItem *_first_canvas_child() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_clamp_to_screen(bool p_enabled);
	bool get_clamp_to_screen() const;

	void set_screen_clamp_margins(const Rect2 &p_margins);
	Rect2 get_screen_clamp_margins() const;

	void set_screen_offset(const Vector2 &p_offset);
	Vector2 get_screen_offset() const;

	void set_center_content(bool p_enabled);
	bool get_center_content() const;

	void set_scale_by_distance(bool p_enabled);
	bool get_scale_by_distance() const;

	void set_distance_scale_reference(float p_distance);
	float get_distance_scale_reference() const;

	void set_distance_scale_min(float p_min);
	float get_distance_scale_min() const;

	void set_distance_scale_max(float p_max);
	float get_distance_scale_max() const;

	void set_use_distance_fade(bool p_enabled);
	bool get_use_distance_fade() const;

	void set_distance_fade_begin(float p_distance);
	float get_distance_fade_begin() const;

	void set_distance_fade_end(float p_distance);
	float get_distance_fade_end() const;

	void set_hide_when_behind_camera(bool p_enabled);
	bool get_hide_when_behind_camera() const;

	// Projected screen position this frame (viewport coords), or zero if unavailable.
	Vector2 get_screen_position() const;

	ZCanvas3D();
	~ZCanvas3D();
};

} // namespace godot
