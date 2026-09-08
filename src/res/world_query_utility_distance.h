#pragma once

#include "world_query_utility.h"

namespace godot {

// Scores by distance from the querier to the candidate position.
class WorldQueryUtility_Distance : public WorldQueryUtility {
	GDCLASS(WorldQueryUtility_Distance, WorldQueryUtility)

private:
	float max_distance = 20.0f;
	// false (default): closer scores higher. true: farther scores higher.
	bool prefer_far = false;

protected:
	static void _bind_methods();

	virtual float on_score_position(Node *p_querier, const Vector3 &p_position) override;

public:
	void set_max_distance(float p_distance);
	float get_max_distance() const;

	void set_prefer_far(bool p_enabled);
	bool get_prefer_far() const;

	WorldQueryUtility_Distance();
	~WorldQueryUtility_Distance();
};

} // namespace godot
