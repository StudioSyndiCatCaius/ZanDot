#pragma once

#include "res/world_query_result.h"
#include "res/world_query_shape.h"
#include "res/world_query_utility.h"
#include "z_world_query_debug_canvas2d.h"

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// EQS-like world query for 2D queriers. Debug view draws colored circles
// (red = low score, green = high score) via a child ZWorldQueryDebugCanvas2D,
// and previews live in the editor (using this node itself as the querier)
// when debug_draw is on. See ZWorldQuery3D for the 3D counterpart.
class ZWorldQuery2D : public Node2D {
	GDCLASS(ZWorldQuery2D, Node2D)

private:
	TypedArray<WorldQueryShape> shapes;
	TypedArray<WorldQueryUtility> utilities;

	bool debug_draw = false;
	// false (default): debug circles stay fixed at the positions from the last
	// query, regardless of where this node moves afterward. true: circles are
	// parented under this node's transform and move with it.
	bool debug_follow_node = false;
	float debug_circle_radius = 8.0f;
	Color debug_low_color = Color(1.0f, 0.0f, 0.0f);
	Color debug_high_color = Color(0.0f, 1.0f, 0.0f);

	ZWorldQueryDebugCanvas2D *debug_canvas = nullptr;

	void _ensure_debug_canvas();
	void _update_debug_draw(const PackedVector3Array &p_positions, const PackedFloat32Array &p_scores);
	void _clear_debug_draw();
	void _connect_resource_signals();
	void _disconnect_resource_signals();
	void _editor_preview_update();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_shapes(const TypedArray<WorldQueryShape> &p_shapes);
	TypedArray<WorldQueryShape> get_shapes() const;

	void set_utilities(const TypedArray<WorldQueryUtility> &p_utilities);
	TypedArray<WorldQueryUtility> get_utilities() const;

	void set_debug_draw(bool p_enabled);
	bool get_debug_draw() const;

	void set_debug_follow_node(bool p_enabled);
	bool get_debug_follow_node() const;

	void set_debug_circle_radius(float p_radius);
	float get_debug_circle_radius() const;

	void set_debug_low_color(const Color &p_color);
	Color get_debug_low_color() const;

	void set_debug_high_color(const Color &p_color);
	Color get_debug_high_color() const;

	// Gathers positions from all shapes, scores each by multiplying every
	// utility's 0..1 score, sorts descending, emits on_query_finish, and
	// returns the same WorldQueryResult.
	Ref<WorldQueryResult> run_query(Node *p_querier);

	ZWorldQuery2D();
	~ZWorldQuery2D();
};

} // namespace godot
