#include "world_query_shape_donut.h"

#include "world_query_common.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void WorldQueryShape_Donut::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_inner_radius", "radius"), &WorldQueryShape_Donut::set_inner_radius);
	ClassDB::bind_method(D_METHOD("get_inner_radius"), &WorldQueryShape_Donut::get_inner_radius);

	ClassDB::bind_method(D_METHOD("set_outer_radius", "radius"), &WorldQueryShape_Donut::set_outer_radius);
	ClassDB::bind_method(D_METHOD("get_outer_radius"), &WorldQueryShape_Donut::get_outer_radius);

	ClassDB::bind_method(D_METHOD("set_ring_count", "count"), &WorldQueryShape_Donut::set_ring_count);
	ClassDB::bind_method(D_METHOD("get_ring_count"), &WorldQueryShape_Donut::get_ring_count);

	ClassDB::bind_method(D_METHOD("set_points_per_ring", "count"), &WorldQueryShape_Donut::set_points_per_ring);
	ClassDB::bind_method(D_METHOD("get_points_per_ring"), &WorldQueryShape_Donut::get_points_per_ring);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "inner_radius", PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"), "set_inner_radius", "get_inner_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "outer_radius", PROPERTY_HINT_RANGE, "0.1,1000,0.1,or_greater"), "set_outer_radius", "get_outer_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ring_count", PROPERTY_HINT_RANGE, "1,32,1,or_greater"), "set_ring_count", "get_ring_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "points_per_ring", PROPERTY_HINT_RANGE, "1,64,1,or_greater"), "set_points_per_ring", "get_points_per_ring");
}

WorldQueryShape_Donut::WorldQueryShape_Donut() {
}

WorldQueryShape_Donut::~WorldQueryShape_Donut() {
}

void WorldQueryShape_Donut::set_inner_radius(float p_radius) {
	inner_radius = p_radius;
}

float WorldQueryShape_Donut::get_inner_radius() const {
	return inner_radius;
}

void WorldQueryShape_Donut::set_outer_radius(float p_radius) {
	outer_radius = p_radius;
}

float WorldQueryShape_Donut::get_outer_radius() const {
	return outer_radius;
}

void WorldQueryShape_Donut::set_ring_count(int p_count) {
	ring_count = p_count;
}

int WorldQueryShape_Donut::get_ring_count() const {
	return ring_count;
}

void WorldQueryShape_Donut::set_points_per_ring(int p_count) {
	points_per_ring = p_count;
}

int WorldQueryShape_Donut::get_points_per_ring() const {
	return points_per_ring;
}

PackedVector3Array WorldQueryShape_Donut::on_get_query_positions(Node *p_querier) {
	PackedVector3Array result;
	WorldQueryPose pose;
	if (!world_query_get_pose(p_querier, pose)) {
		return result;
	}
	if (ring_count <= 0 || points_per_ring <= 0) {
		return result;
	}

	for (int ring = 0; ring < ring_count; ring++) {
		const float ring_t = ring_count > 1 ? (float)ring / (float)(ring_count - 1) : 1.0f;
		const float radius = Math::lerp(inner_radius, outer_radius, ring_t);
		for (int i = 0; i < points_per_ring; i++) {
			const float angle = (float)i / (float)points_per_ring * (float)Math_TAU;
			const float ox = Math::cos(angle) * radius;
			const float oy = Math::sin(angle) * radius;
			const Vector3 offset = pose.is_3d ? Vector3(ox, 0.0f, oy) : Vector3(ox, oy, 0.0f);
			result.push_back(pose.position + offset);
		}
	}
	return result;
}
