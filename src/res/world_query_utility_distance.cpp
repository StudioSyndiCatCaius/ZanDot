#include "world_query_utility_distance.h"

#include "world_query_common.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void WorldQueryUtility_Distance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_distance", "distance"), &WorldQueryUtility_Distance::set_max_distance);
	ClassDB::bind_method(D_METHOD("get_max_distance"), &WorldQueryUtility_Distance::get_max_distance);

	ClassDB::bind_method(D_METHOD("set_prefer_far", "enabled"), &WorldQueryUtility_Distance::set_prefer_far);
	ClassDB::bind_method(D_METHOD("get_prefer_far"), &WorldQueryUtility_Distance::get_prefer_far);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_distance", PROPERTY_HINT_RANGE, "0.1,1000,0.1,or_greater"), "set_max_distance", "get_max_distance");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "prefer_far"), "set_prefer_far", "get_prefer_far");
}

WorldQueryUtility_Distance::WorldQueryUtility_Distance() {
}

WorldQueryUtility_Distance::~WorldQueryUtility_Distance() {
}

void WorldQueryUtility_Distance::set_max_distance(float p_distance) {
	max_distance = p_distance;
}

float WorldQueryUtility_Distance::get_max_distance() const {
	return max_distance;
}

void WorldQueryUtility_Distance::set_prefer_far(bool p_enabled) {
	prefer_far = p_enabled;
}

bool WorldQueryUtility_Distance::get_prefer_far() const {
	return prefer_far;
}

float WorldQueryUtility_Distance::on_score_position(Node *p_querier, const Vector3 &p_position) {
	WorldQueryPose pose;
	if (!world_query_get_pose(p_querier, pose) || max_distance <= 0.0f) {
		return 1.0f;
	}
	const float distance = pose.position.distance_to(p_position);
	const float t = Math::clamp(distance / max_distance, 0.0f, 1.0f);
	return prefer_far ? t : 1.0f - t;
}
