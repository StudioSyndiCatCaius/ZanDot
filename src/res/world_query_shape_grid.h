#pragma once

#include "world_query_shape.h"

#include <godot_cpp/variant/vector2i.hpp>

namespace godot {

// Flat grid of points centered on the querier, axis-aligned on the movement plane
// (3D: XZ; 2D: XY) — not rotated by the querier's facing.
class WorldQueryShape_Grid : public WorldQueryShape {
	GDCLASS(WorldQueryShape_Grid, WorldQueryShape)

private:
	Vector2i grid_size = Vector2i(5, 5);
	float cell_spacing = 2.0f;

protected:
	static void _bind_methods();

	virtual PackedVector3Array on_get_query_positions(Node *p_querier) override;

public:
	void set_grid_size(const Vector2i &p_size);
	Vector2i get_grid_size() const;

	void set_cell_spacing(float p_spacing);
	float get_cell_spacing() const;

	WorldQueryShape_Grid();
	~WorldQueryShape_Grid();
};

} // namespace godot
