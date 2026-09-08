#include "z_global_condition.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZGlobalCondition::_bind_methods() {
	ClassDB::bind_method(D_METHOD("check"), &ZGlobalCondition::check_bind);

	GDVIRTUAL_BIND(_check);
}

ZGlobalCondition::ZGlobalCondition() {
}

ZGlobalCondition::~ZGlobalCondition() {
}

bool ZGlobalCondition::check() {
	return false;
}

bool ZGlobalCondition::check_bind() {
	bool result = false;
	if (GDVIRTUAL_CALL(_check, result)) {
		return result;
	}
	return check();
}
