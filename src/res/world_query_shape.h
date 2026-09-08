#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/packed_vector3_array.hpp>

namespace godot {

// Base for EQS-like query shapes: generates candidate world positions around a querier.
// Works for both 2D and 3D queriers (Node2D/Node3D); positions are always Vector3,
// with 2D callers using x/y and leaving z at 0. See world_query_common.h.
class WorldQueryShape : public Resource {
	GDCLASS(WorldQueryShape, Resource)

protected:
	static void _bind_methods();

	virtual PackedVector3Array on_get_query_positions(Node *p_querier);
	GDVIRTUAL1R(PackedVector3Array, _get_query_positions, Node *)

public:
	PackedVector3Array get_query_positions(Node *p_querier);

	WorldQueryShape();
	~WorldQueryShape();
};

} // namespace godot
