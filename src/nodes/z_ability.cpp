#include "z_ability.h"
#include "z_ability_manager.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

void ZAbility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_input_action", "input_action"), &ZAbility::set_input_action);
	ClassDB::bind_method(D_METHOD("get_input_action"), &ZAbility::get_input_action);
	ClassDB::bind_method(D_METHOD("set_cancel_ability_on_release", "cancel_ability_on_release"), &ZAbility::set_cancel_ability_on_release);
	ClassDB::bind_method(D_METHOD("get_cancel_ability_on_release"), &ZAbility::get_cancel_ability_on_release);
	ClassDB::bind_method(D_METHOD("get_is_active"), &ZAbility::get_is_active);

	ClassDB::bind_method(D_METHOD("ability_activate", "context"), &ZAbility::ability_activate, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("Ability_Activate", "context"), &ZAbility::ability_activate, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("ability_stop", "cancelled"), &ZAbility::ability_stop, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("Ability_Stop", "cancelled"), &ZAbility::ability_stop, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("ability_can_activate", "context"), &ZAbility::ability_can_activate, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("Ability_CanActivate", "context"), &ZAbility::ability_can_activate, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("get_utility_score", "context"), &ZAbility::get_utility_score, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("Ability_GetUtilityScore", "context"), &ZAbility::get_utility_score, DEFVAL(Variant()));

	ClassDB::bind_method(D_METHOD("get_ability_manager"), &ZAbility::get_ability_manager);
	ClassDB::bind_method(D_METHOD("get_root_owner"), &ZAbility::get_root_owner);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "input_action"), "set_input_action", "get_input_action");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cancel_ability_on_release"), "set_cancel_ability_on_release", "get_cancel_ability_on_release");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_active", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_is_active");

	ADD_SIGNAL(MethodInfo("on_ability_activated", PropertyInfo(Variant::NIL, "context", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
	ADD_SIGNAL(MethodInfo("on_ability_finished", PropertyInfo(Variant::BOOL, "cancelled")));

	GDVIRTUAL_BIND(_on_ability_activated, "context");
	GDVIRTUAL_BIND(_on_ability_finished, "cancelled");
	GDVIRTUAL_BIND(_process_activated_ability, "delta");
	GDVIRTUAL_BIND(_get_utility_score, "context");
	GDVIRTUAL_BIND(_ability_can_activate, "context");
}

void ZAbility::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}
			if (is_active) {
				float delta = get_process_delta_time();
				if (GDVIRTUAL_IS_OVERRIDDEN(_process_activated_ability)) {
					GDVIRTUAL_CALL(_process_activated_ability, (double)delta);
				} else if (has_method("_process_activated_ability")) {
					call("_process_activated_ability", delta);
				} else {
					on_process_activated_ability(delta);
				}
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (is_active) {
				ability_stop(true);
			}
		} break;
	}
}

void ZAbility::set_input_action(const String &p_action) {
	input_action = p_action;
}

String ZAbility::get_input_action() const {
	return input_action;
}

void ZAbility::set_cancel_ability_on_release(bool p_enabled) {
	cancel_ability_on_release = p_enabled;
}

bool ZAbility::get_cancel_ability_on_release() const {
	return cancel_ability_on_release;
}

bool ZAbility::get_is_active() const {
	return is_active;
}

bool ZAbility::ability_activate(const Variant &p_context) {
	if (is_active) {
		return false;
	}

	if (!ability_can_activate(p_context)) {
		return false;
	}

	is_active = true;
	set_process(true);

	emit_signal("on_ability_activated", p_context);

	ZAbilityManager *manager = get_ability_manager();
	if (manager != nullptr) {
		manager->_notify_ability_activated(this, p_context);
	}

	if (GDVIRTUAL_IS_OVERRIDDEN(_on_ability_activated)) {
		GDVIRTUAL_CALL(_on_ability_activated, p_context);
	} else if (has_method("_on_ability_activated")) {
		call("_on_ability_activated", p_context);
	} else {
		on_ability_activated(p_context);
	}

	return true;
}

void ZAbility::ability_stop(bool p_cancelled) {
	if (!is_active) {
		return;
	}

	is_active = false;
	set_process(false);

	emit_signal("on_ability_finished", p_cancelled);

	ZAbilityManager *manager = get_ability_manager();
	if (manager != nullptr) {
		manager->_notify_ability_finished(this, p_cancelled);
	}

	if (GDVIRTUAL_IS_OVERRIDDEN(_on_ability_finished)) {
		GDVIRTUAL_CALL(_on_ability_finished, p_cancelled);
	} else if (has_method("_on_ability_finished")) {
		call("_on_ability_finished", p_cancelled);
	} else {
		on_ability_finished(p_cancelled);
	}
}

bool ZAbility::ability_can_activate(const Variant &p_context) {
	bool ret = true;
	if (GDVIRTUAL_IS_OVERRIDDEN(_ability_can_activate)) {
		GDVIRTUAL_CALL(_ability_can_activate, p_context, ret);
	} else if (has_method("_ability_can_activate")) {
		ret = call("_ability_can_activate", p_context);
	} else {
		ret = on_ability_can_activate(p_context);
	}
	return ret;
}

float ZAbility::get_utility_score(const Variant &p_context) {
	float ret = 0.0f;
	if (GDVIRTUAL_IS_OVERRIDDEN(_get_utility_score)) {
		GDVIRTUAL_CALL(_get_utility_score, p_context, ret);
	} else if (has_method("_get_utility_score")) {
		ret = call("_get_utility_score", p_context);
	} else {
		ret = on_get_utility_score(p_context);
	}
	return ret;
}

ZAbilityManager *ZAbility::get_ability_manager() const {
	Node *parent = get_parent();
	while (parent != nullptr) {
		ZAbilityManager *manager = Object::cast_to<ZAbilityManager>(parent);
		if (manager != nullptr) {
			return manager;
		}
		parent = parent->get_parent();
	}
	return nullptr;
}

Node *ZAbility::get_root_owner() const {
	ZAbilityManager *manager = get_ability_manager();
	if (manager != nullptr) {
		return manager->get_root_owner();
	}
	return nullptr;
}

void ZAbility::on_ability_activated(const Variant &p_context) {
	// no-op default
}

void ZAbility::on_ability_finished(bool p_cancelled) {
	// no-op default
}

void ZAbility::on_process_activated_ability(double p_delta) {
	// no-op default
}

float ZAbility::on_get_utility_score(const Variant &p_context) {
	return 0.0f;
}

bool ZAbility::on_ability_can_activate(const Variant &p_context) {
	return true;
}

ZAbility::ZAbility() {
	set_process(false);
}

ZAbility::~ZAbility() {
}

} // namespace godot
