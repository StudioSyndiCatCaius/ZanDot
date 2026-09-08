#include "z_world_query_debug_canvas2d.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZWorldQueryDebugCanvas2D::_bind_methods() {
}

ZWorldQueryDebugCanvas2D::ZWorldQueryDebugCanvas2D() {
}

ZWorldQueryDebugCanvas2D::~ZWorldQueryDebugCanvas2D() {
}

void ZWorldQueryDebugCanvas2D::set_points(const PackedVector3Array &p_positions, const PackedFloat32Array &p_scores, float p_radius, const Color &p_low_color, const Color &p_high_color) {
	positions = p_positions;
	scores = p_scores;
	radius = p_radius;
	low_color = p_low_color;
	high_color = p_high_color;
	queue_redraw();
}

void ZWorldQueryDebugCanvas2D::_draw() {
	const int count = positions.size();
	for (int i = 0; i < count; i++) {
		const Vector3 &pos = positions[i];
		const float score = i < scores.size() ? scores[i] : 0.0f;
		draw_circle(Vector2(pos.x, pos.y), radius, low_color.lerp(high_color, score));
	}
}
