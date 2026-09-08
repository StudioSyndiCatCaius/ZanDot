#include "z_global_script.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZGlobalScript::_bind_methods() {
	ClassDB::bind_method(D_METHOD("run"), &ZGlobalScript::run_bind);

	GDVIRTUAL_BIND(_run);
}

ZGlobalScript::ZGlobalScript() {
}

ZGlobalScript::~ZGlobalScript() {
}

void ZGlobalScript::run() {
}

void ZGlobalScript::run_bind() {
	if (GDVIRTUAL_CALL(_run)) {
		return;
	}
	run();
}
