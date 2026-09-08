#include "world_query_common.h"

#include <algorithm>
#include <utility>
#include <vector>

using namespace godot;

Ref<WorldQueryResult> godot::world_query_run(const TypedArray<WorldQueryShape> &p_shapes, const TypedArray<WorldQueryUtility> &p_utilities, Node *p_querier) {
	Ref<WorldQueryResult> result;
	result.instantiate();

	PackedVector3Array all_positions;
	for (int i = 0; i < p_shapes.size(); i++) {
		Ref<WorldQueryShape> shape = p_shapes[i];
		if (shape.is_null()) {
			continue;
		}
		const PackedVector3Array positions = shape->get_query_positions(p_querier);
		for (int p = 0; p < positions.size(); p++) {
			all_positions.push_back(positions[p]);
		}
	}

	std::vector<std::pair<float, Vector3>> scored;
	scored.reserve(all_positions.size());
	for (int i = 0; i < all_positions.size(); i++) {
		const Vector3 &pos = all_positions[i];
		float score = 1.0f;
		for (int u = 0; u < p_utilities.size(); u++) {
			Ref<WorldQueryUtility> utility = p_utilities[u];
			if (utility.is_null()) {
				continue;
			}
			score *= utility->score_position(p_querier, pos);
		}
		scored.emplace_back(score, pos);
	}

	std::sort(scored.begin(), scored.end(), [](const std::pair<float, Vector3> &a, const std::pair<float, Vector3> &b) {
		return a.first > b.first;
	});

	PackedVector3Array ordered_positions;
	PackedFloat32Array ordered_scores;
	ordered_positions.resize((int)scored.size());
	ordered_scores.resize((int)scored.size());
	for (size_t i = 0; i < scored.size(); i++) {
		ordered_positions[(int)i] = scored[i].second;
		ordered_scores[(int)i] = scored[i].first;
	}

	result->set_ordered_positions(ordered_positions);
	result->set_ordered_scores(ordered_scores);
	return result;
}
