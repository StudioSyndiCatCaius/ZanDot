#pragma once

#include "world_query_utility.h"

namespace godot {

// Scores by the angle between the querier's facing and the direction to the
// candidate position (0 deg = facing it directly = score 1).
class WorldQueryUtility_Angle : public WorldQueryUtility {
	GDCLASS(WorldQueryUtility_Angle, WorldQueryUtility)

private:
	float max_angle_degrees = 180.0f;

protected:
	static void _bind_methods();

	virtual float on_score_position(Node *p_querier, const Vector3 &p_position) override;

public:
	void set_max_angle_degrees(float p_degrees);
	float get_max_angle_degrees() const;

	WorldQueryUtility_Angle();
	~WorldQueryUtility_Angle();
};

} // namespace godot
