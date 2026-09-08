#include "world_query_utility_angle.h"

#include "world_query_common.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void WorldQueryUtility_Angle::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_angle_degrees", "degrees"), &WorldQueryUtility_Angle::set_max_angle_degrees);
	ClassDB::bind_method(D_METHOD("get_max_angle_degrees"), &WorldQueryUtility_Angle::get_max_angle_degrees);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_angle_degrees", PROPERTY_HINT_RANGE, "1,180,1"), "set_max_angle_degrees", "get_max_angle_degrees");
}

WorldQueryUtility_Angle::WorldQueryUtility_Angle() {
}

WorldQueryUtility_Angle::~WorldQueryUtility_Angle() {
}

void WorldQueryUtility_Angle::set_max_angle_degrees(float p_degrees) {
	max_angle_degrees = p_degrees;
}

float WorldQueryUtility_Angle::get_max_angle_degrees() const {
	return max_angle_degrees;
}

float WorldQueryUtility_Angle::on_score_position(Node *p_querier, const Vector3 &p_position) {
	WorldQueryPose pose;
	if (!world_query_get_pose(p_querier, pose) || max_angle_degrees <= 0.0f) {
		return 1.0f;
	}

	const Vector3 forward = world_query_project_planar(pose, world_query_forward(pose));
	Vector3 to_point = world_query_project_planar(pose, p_position - pose.position);
	if (forward.length_squared() < 1e-12f || to_point.length_squared() < 1e-12f) {
		return 1.0f;
	}

	const float dot = Math::clamp(forward.normalized().dot(to_point.normalized()), -1.0f, 1.0f);
	const float angle_deg = Math::rad_to_deg(Math::acos(dot));
	const float t = Math::clamp(angle_deg / max_angle_degrees, 0.0f, 1.0f);
	return 1.0f - t;
}
