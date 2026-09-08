#include "world_query_utility_trace.h"

#include <godot_cpp/classes/collision_object2d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state2d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters2d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world2d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldQueryUtility_Trace::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &WorldQueryUtility_Trace::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &WorldQueryUtility_Trace::get_collision_mask);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
}

WorldQueryUtility_Trace::WorldQueryUtility_Trace() {
}

WorldQueryUtility_Trace::~WorldQueryUtility_Trace() {
}

void WorldQueryUtility_Trace::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;
}

uint32_t WorldQueryUtility_Trace::get_collision_mask() const {
	return collision_mask;
}

float WorldQueryUtility_Trace::on_score_position(Node *p_querier, const Vector3 &p_position) {
	Node3D *n3 = Object::cast_to<Node3D>(p_querier);
	if (n3 != nullptr) {
		Ref<World3D> world = n3->get_world_3d();
		if (world.is_null()) {
			return 1.0f;
		}
		PhysicsDirectSpaceState3D *space = world->get_direct_space_state();
		if (space == nullptr) {
			return 1.0f;
		}
		TypedArray<RID> exclude;
		CollisionObject3D *co = Object::cast_to<CollisionObject3D>(p_querier);
		if (co != nullptr) {
			exclude.push_back(co->get_rid());
		}
		Ref<PhysicsRayQueryParameters3D> params = PhysicsRayQueryParameters3D::create(n3->get_global_transform().origin, p_position, collision_mask, exclude);
		const Dictionary hit = space->intersect_ray(params);
		return hit.is_empty() ? 1.0f : 0.0f;
	}

	Node2D *n2 = Object::cast_to<Node2D>(p_querier);
	if (n2 != nullptr) {
		Ref<World2D> world = n2->get_world_2d();
		if (world.is_null()) {
			return 1.0f;
		}
		PhysicsDirectSpaceState2D *space = world->get_direct_space_state();
		if (space == nullptr) {
			return 1.0f;
		}
		TypedArray<RID> exclude;
		CollisionObject2D *co = Object::cast_to<CollisionObject2D>(p_querier);
		if (co != nullptr) {
			exclude.push_back(co->get_rid());
		}
		Ref<PhysicsRayQueryParameters2D> params = PhysicsRayQueryParameters2D::create(n2->get_global_position(), Vector2(p_position.x, p_position.y), collision_mask, exclude);
		const Dictionary hit = space->intersect_ray(params);
		return hit.is_empty() ? 1.0f : 0.0f;
	}

	return 1.0f;
}
