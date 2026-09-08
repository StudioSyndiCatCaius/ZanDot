#include "world_query_shape_grid.h"

#include "world_query_common.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldQueryShape_Grid::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_grid_size", "size"), &WorldQueryShape_Grid::set_grid_size);
	ClassDB::bind_method(D_METHOD("get_grid_size"), &WorldQueryShape_Grid::get_grid_size);

	ClassDB::bind_method(D_METHOD("set_cell_spacing", "spacing"), &WorldQueryShape_Grid::set_cell_spacing);
	ClassDB::bind_method(D_METHOD("get_cell_spacing"), &WorldQueryShape_Grid::get_cell_spacing);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "grid_size"), "set_grid_size", "get_grid_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cell_spacing", PROPERTY_HINT_RANGE, "0.1,50,0.1,or_greater"), "set_cell_spacing", "get_cell_spacing");
}

WorldQueryShape_Grid::WorldQueryShape_Grid() {
}

WorldQueryShape_Grid::~WorldQueryShape_Grid() {
}

void WorldQueryShape_Grid::set_grid_size(const Vector2i &p_size) {
	grid_size = p_size;
}

Vector2i WorldQueryShape_Grid::get_grid_size() const {
	return grid_size;
}

void WorldQueryShape_Grid::set_cell_spacing(float p_spacing) {
	cell_spacing = p_spacing;
}

float WorldQueryShape_Grid::get_cell_spacing() const {
	return cell_spacing;
}

PackedVector3Array WorldQueryShape_Grid::on_get_query_positions(Node *p_querier) {
	PackedVector3Array result;
	WorldQueryPose pose;
	if (!world_query_get_pose(p_querier, pose)) {
		return result;
	}
	if (grid_size.x <= 0 || grid_size.y <= 0) {
		return result;
	}

	const float half_x = (float)(grid_size.x - 1) * 0.5f * cell_spacing;
	const float half_y = (float)(grid_size.y - 1) * 0.5f * cell_spacing;

	for (int y = 0; y < grid_size.y; y++) {
		for (int x = 0; x < grid_size.x; x++) {
			const float ox = (float)x * cell_spacing - half_x;
			const float oy = (float)y * cell_spacing - half_y;
			const Vector3 offset = pose.is_3d ? Vector3(ox, 0.0f, oy) : Vector3(ox, oy, 0.0f);
			result.push_back(pose.position + offset);
		}
	}
	return result;
}
