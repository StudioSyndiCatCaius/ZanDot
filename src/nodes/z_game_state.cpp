#include "z_game_state.h"

#include "z_game_mode.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZGameState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("state_enter"), &ZGameState::state_enter);
	ClassDB::bind_method(D_METHOD("state_exit"), &ZGameState::state_exit);
	ClassDB::bind_method(D_METHOD("get_game_mode"), &ZGameState::get_game_mode);

	GDVIRTUAL_BIND(_on_state_enter);
	GDVIRTUAL_BIND(_on_state_exit);
}

ZGameState::ZGameState() {
}

ZGameState::~ZGameState() {
}

void ZGameState::on_state_enter() {
}

void ZGameState::on_state_exit() {
}

void ZGameState::state_enter() {
	// Prefer GDVIRTUAL, then GDScript method name, then C++ virtual.
	if (GDVIRTUAL_IS_OVERRIDDEN(_on_state_enter)) {
		GDVIRTUAL_CALL(_on_state_enter);
	} else if (has_method("_on_state_enter")) {
		call("_on_state_enter");
	} else {
		on_state_enter();
	}
}

void ZGameState::state_exit() {
	if (GDVIRTUAL_IS_OVERRIDDEN(_on_state_exit)) {
		GDVIRTUAL_CALL(_on_state_exit);
	} else if (has_method("_on_state_exit")) {
		call("_on_state_exit");
	} else {
		on_state_exit();
	}
}

ZGameMode *ZGameState::get_game_mode() const {
	Node *p = get_parent();
	while (p != nullptr) {
		ZGameMode *mode = Object::cast_to<ZGameMode>(p);
		if (mode != nullptr) {
			return mode;
		}
		p = p->get_parent();
	}
	return nullptr;
}
