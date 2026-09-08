#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

namespace godot {

// Output of ZWorldQuery.run_query: positions ordered best (highest score) first,
// with a parallel array of their scores.
class WorldQueryResult : public Resource {
	GDCLASS(WorldQueryResult, Resource)

private:
	PackedVector3Array ordered_positions;
	PackedFloat32Array ordered_scores;

protected:
	static void _bind_methods();

public:
	void set_ordered_positions(const PackedVector3Array &p_positions);
	PackedVector3Array get_ordered_positions() const;

	void set_ordered_scores(const PackedFloat32Array &p_scores);
	PackedFloat32Array get_ordered_scores() const;

	// Positions whose score is >= p_score (or <= p_score if p_invert is true).
	PackedVector3Array get_positions_over_score(float p_score, bool p_invert = false) const;

	WorldQueryResult();
	~WorldQueryResult();
};

} // namespace godot
