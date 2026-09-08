#include "world_query_shape.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldQueryShape::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_query_positions", "querier"), &WorldQueryShape::get_query_positions);

	GDVIRTUAL_BIND(_get_query_positions, "querier");
}

WorldQueryShape::WorldQueryShape() {
}

WorldQueryShape::~WorldQueryShape() {
}

PackedVector3Array WorldQueryShape::on_get_query_positions(Node *p_querier) {
	(void)p_querier;
	return PackedVector3Array();
}

PackedVector3Array WorldQueryShape::get_query_positions(Node *p_querier) {
	PackedVector3Array ret;
	if (GDVIRTUAL_IS_OVERRIDDEN(_get_query_positions)) {
		GDVIRTUAL_CALL(_get_query_positions, p_querier, ret);
	} else if (has_method("_get_query_positions")) {
		ret = call("_get_query_positions", p_querier);
	} else {
		ret = on_get_query_positions(p_querier);
	}
	return ret;
}
