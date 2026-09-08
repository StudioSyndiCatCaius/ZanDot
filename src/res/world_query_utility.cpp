#include "world_query_utility.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldQueryUtility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("score_position", "querier", "position"), &WorldQueryUtility::score_position);

	GDVIRTUAL_BIND(_score_position, "querier", "position");
}

WorldQueryUtility::WorldQueryUtility() {
}

WorldQueryUtility::~WorldQueryUtility() {
}

float WorldQueryUtility::on_score_position(Node *p_querier, const Vector3 &p_position) {
	(void)p_querier;
	(void)p_position;
	return 1.0f;
}

float WorldQueryUtility::score_position(Node *p_querier, const Vector3 &p_position) {
	float ret = 1.0f;
	if (GDVIRTUAL_IS_OVERRIDDEN(_score_position)) {
		GDVIRTUAL_CALL(_score_position, p_querier, p_position, ret);
	} else if (has_method("_score_position")) {
		ret = call("_score_position", p_querier, p_position);
	} else {
		ret = on_score_position(p_querier, p_position);
	}
	return ret;
}
