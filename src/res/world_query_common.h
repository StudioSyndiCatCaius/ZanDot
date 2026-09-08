#pragma once

#include "world_query_result.h"
#include "world_query_shape.h"
#include "world_query_utility.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Shared 2D/3D querier resolution for WorldQueryShape/WorldQueryUtility subclasses.
// Positions are always Vector3 (2D uses XY, z stays 0). Basis facing follows the
// same convention as ZMover: 3D forward is -Z; 2D forward is rotation around Z,
// with angle 0 = +X.
struct WorldQueryPose {
	Vector3 position;
	Basis basis;
	bool is_3d = true;
};

inline bool world_query_get_pose(Node *p_node, WorldQueryPose &r_pose) {
	if (p_node == nullptr) {
		return false;
	}
	Node3D *n3 = Object::cast_to<Node3D>(p_node);
	if (n3 != nullptr) {
		const Transform3D xf = n3->get_global_transform();
		r_pose.position = xf.origin;
		r_pose.basis = xf.basis;
		r_pose.is_3d = true;
		return true;
	}
	Node2D *n2 = Object::cast_to<Node2D>(p_node);
	if (n2 != nullptr) {
		const Vector2 pos = n2->get_global_position();
		r_pose.position = Vector3(pos.x, pos.y, 0.0f);
		r_pose.basis = Basis(Vector3(0, 0, 1), n2->get_global_rotation());
		r_pose.is_3d = false;
		return true;
	}
	return false;
}

inline Vector3 world_query_forward(const WorldQueryPose &p_pose) {
	if (p_pose.is_3d) {
		return -p_pose.basis.get_column(2);
	}
	return p_pose.basis.get_column(0);
}

// Drops the axis irrelevant to this pose's movement plane (3D: XZ, up = Y; 2D: XY).
inline Vector3 world_query_project_planar(const WorldQueryPose &p_pose, const Vector3 &p_vec) {
	if (p_pose.is_3d) {
		return Vector3(p_vec.x, 0.0f, p_vec.z);
	}
	return Vector3(p_vec.x, p_vec.y, 0.0f);
}

// Shared by ZWorldQuery2D/ZWorldQuery3D: gathers positions from every shape,
// scores each by multiplying every utility's 0..1 result, and returns them
// sorted descending (best first) as a WorldQueryResult.
Ref<WorldQueryResult> world_query_run(const TypedArray<WorldQueryShape> &p_shapes, const TypedArray<WorldQueryUtility> &p_utilities, Node *p_querier);

} // namespace godot
