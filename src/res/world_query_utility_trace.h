#pragma once

#include "world_query_utility.h"

namespace godot {

// Traces from the querier to the candidate position; 0 if blocked, 1 if clear.
class WorldQueryUtility_Trace : public WorldQueryUtility {
	GDCLASS(WorldQueryUtility_Trace, WorldQueryUtility)

private:
	uint32_t collision_mask = 1;

protected:
	static void _bind_methods();

	virtual float on_score_position(Node *p_querier, const Vector3 &p_position) override;

public:
	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	WorldQueryUtility_Trace();
	~WorldQueryUtility_Trace();
};

} // namespace godot
