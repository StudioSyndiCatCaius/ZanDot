#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Base for EQS-like scoring filters: scores a single candidate position 0..1.
class WorldQueryUtility : public Resource {
	GDCLASS(WorldQueryUtility, Resource)

protected:
	static void _bind_methods();

	virtual float on_score_position(Node *p_querier, const Vector3 &p_position);
	GDVIRTUAL2R(float, _score_position, Node *, Vector3)

public:
	float score_position(Node *p_querier, const Vector3 &p_position);

	WorldQueryUtility();
	~WorldQueryUtility();
};

} // namespace godot
