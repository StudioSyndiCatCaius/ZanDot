#include "z_game_state.h"

#include "z_game_mode.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZGameState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("state_enter", "context"), &ZGameState::state_enter, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("state_exit"), &ZGameState::state_exit);
	ClassDB::bind_method(D_METHOD("get_game_mode"), &ZGameState::get_game_mode);
	ClassDB::bind_method(D_METHOD("set_context", "context"), &ZGameState::set_context);
	ClassDB::bind_method(D_METHOD("get_context"), &ZGameState::get_context);

	ADD_PROPERTY(PropertyInfo(Variant::NIL, "context", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_NIL_IS_VARIANT), "set_context", "get_context");

	GDVIRTUAL_BIND(_on_state_enter, "context");
	GDVIRTUAL_BIND(_on_state_exit);
}

ZGameState::ZGameState() {
}

ZGameState::~ZGameState() {
}

void ZGameState::on_state_enter(const Variant &p_context) {
	(void)p_context;
}

void ZGameState::on_state_exit() {
}

void ZGameState::set_context(const Variant &p_context) {
	context = p_context;
}

Variant ZGameState::get_context() const {
	return context;
}

void ZGameState::state_enter(const Variant &p_context) {
	context = p_context;
	// Prefer explicit script method + call() so Variant/Dictionary context is
	// delivered reliably to GDScript. Fall back to GDVIRTUAL, then C++ virtual.
	if (has_method("_on_state_enter")) {
		call("_on_state_enter", p_context);
	} else if (GDVIRTUAL_IS_OVERRIDDEN(_on_state_enter)) {
		GDVIRTUAL_CALL(_on_state_enter, p_context);
	} else {
		on_state_enter(p_context);
	}
}

void ZGameState::state_exit() {
	if (has_method("_on_state_exit")) {
		call("_on_state_exit");
	} else if (GDVIRTUAL_IS_OVERRIDDEN(_on_state_exit)) {
		GDVIRTUAL_CALL(_on_state_exit);
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
