#pragma once

#include "world_query_shape.h"

namespace godot {

// Concentric rings of points around the querier, on the movement plane
// (3D: XZ; 2D: XY).
class WorldQueryShape_Donut : public WorldQueryShape {
	GDCLASS(WorldQueryShape_Donut, WorldQueryShape)

private:
	float inner_radius = 2.0f;
	float outer_radius = 6.0f;
	int ring_count = 2;
	int points_per_ring = 8;

protected:
	static void _bind_methods();

	virtual PackedVector3Array on_get_query_positions(Node *p_querier) override;

public:
	void set_inner_radius(float p_radius);
	float get_inner_radius() const;

	void set_outer_radius(float p_radius);
	float get_outer_radius() const;

	void set_ring_count(int p_count);
	int get_ring_count() const;

	void set_points_per_ring(int p_count);
	int get_points_per_ring() const;

	WorldQueryShape_Donut();
	~WorldQueryShape_Donut();
};

} // namespace godot
