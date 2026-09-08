#pragma once

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

namespace godot {

// Internal helper used by ZWorldQuery2D to draw debug circles. Kept as its
// own node (instead of drawing directly on ZWorldQuery2D) so it can be made
// top-level and rendered at fixed world positions independent of whatever
// transform the owning ZWorldQuery2D node has (see debug_follow_node).
class ZWorldQueryDebugCanvas2D : public Node2D {
	GDCLASS(ZWorldQueryDebugCanvas2D, Node2D)

private:
	PackedVector3Array positions;
	PackedFloat32Array scores;
	float radius = 8.0f;
	Color low_color = Color(1.0f, 0.0f, 0.0f);
	Color high_color = Color(0.0f, 1.0f, 0.0f);

protected:
	static void _bind_methods();

public:
	virtual void _draw() override;

	void set_points(const PackedVector3Array &p_positions, const PackedFloat32Array &p_scores, float p_radius, const Color &p_low_color, const Color &p_high_color);

	ZWorldQueryDebugCanvas2D();
	~ZWorldQueryDebugCanvas2D();
};

} // namespace godot
