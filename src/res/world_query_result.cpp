#include "world_query_result.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldQueryResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_ordered_positions", "positions"), &WorldQueryResult::set_ordered_positions);
	ClassDB::bind_method(D_METHOD("get_ordered_positions"), &WorldQueryResult::get_ordered_positions);

	ClassDB::bind_method(D_METHOD("set_ordered_scores", "scores"), &WorldQueryResult::set_ordered_scores);
	ClassDB::bind_method(D_METHOD("get_ordered_scores"), &WorldQueryResult::get_ordered_scores);

	ClassDB::bind_method(D_METHOD("get_positions_over_score", "score", "invert"), &WorldQueryResult::get_positions_over_score, DEFVAL(false));

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "ordered_positions"), "set_ordered_positions", "get_ordered_positions");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "ordered_scores"), "set_ordered_scores", "get_ordered_scores");
}

WorldQueryResult::WorldQueryResult() {
}

WorldQueryResult::~WorldQueryResult() {
}

void WorldQueryResult::set_ordered_positions(const PackedVector3Array &p_positions) {
	ordered_positions = p_positions;
}

PackedVector3Array WorldQueryResult::get_ordered_positions() const {
	return ordered_positions;
}

void WorldQueryResult::set_ordered_scores(const PackedFloat32Array &p_scores) {
	ordered_scores = p_scores;
}

PackedFloat32Array WorldQueryResult::get_ordered_scores() const {
	return ordered_scores;
}

PackedVector3Array WorldQueryResult::get_positions_over_score(float p_score, bool p_invert) const {
	PackedVector3Array result;
	const int count = ordered_positions.size() < ordered_scores.size() ? ordered_positions.size() : ordered_scores.size();
	for (int i = 0; i < count; i++) {
		const float s = ordered_scores[i];
		const bool passes = p_invert ? (s <= p_score) : (s >= p_score);
		if (passes) {
			result.push_back(ordered_positions[i]);
		}
	}
	return result;
}
