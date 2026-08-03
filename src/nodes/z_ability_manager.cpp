#include "z_ability_manager.h"
#include "z_ability.h"
#include "z.h"
#include "../res/z_player.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void ZAbilityManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_root_owner", "root_owner"), &ZAbilityManager::set_root_owner);
	ClassDB::bind_method(D_METHOD("get_root_owner"), &ZAbilityManager::get_root_owner);
	ClassDB::bind_method(D_METHOD("set_auto_find_root_owner", "auto_find_root_owner"), &ZAbilityManager::set_auto_find_root_owner);
	ClassDB::bind_method(D_METHOD("get_auto_find_root_owner"), &ZAbilityManager::get_auto_find_root_owner);
	ClassDB::bind_method(D_METHOD("set_autorun_utility", "autorun_utility"), &ZAbilityManager::set_autorun_utility);
	ClassDB::bind_method(D_METHOD("get_autorun_utility"), &ZAbilityManager::get_autorun_utility);
	ClassDB::bind_method(D_METHOD("set_utility_frequency", "utility_frequency"), &ZAbilityManager::set_utility_frequency);
	ClassDB::bind_method(D_METHOD("get_utility_frequency"), &ZAbilityManager::get_utility_frequency);
	ClassDB::bind_method(D_METHOD("set_utility_variance", "utility_variance"), &ZAbilityManager::set_utility_variance);
	ClassDB::bind_method(D_METHOD("get_utility_variance"), &ZAbilityManager::get_utility_variance);

	ClassDB::bind_method(D_METHOD("get_abilities"), &ZAbilityManager::get_abilities);
	ClassDB::bind_method(D_METHOD("find_ability", "name"), &ZAbilityManager::find_ability);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_owner", PROPERTY_HINT_NODE_TYPE, "Node"), "set_root_owner", "get_root_owner");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_find_root_owner"), "set_auto_find_root_owner", "get_auto_find_root_owner");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autorun_utility"), "set_autorun_utility", "get_autorun_utility");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "utility_frequency", PROPERTY_HINT_RANGE, "0.01,10,0.01,or_greater"), "set_utility_frequency", "get_utility_frequency");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "utility_variance", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_utility_variance", "get_utility_variance");

	ADD_SIGNAL(MethodInfo("on_ability_activated", PropertyInfo(Variant::OBJECT, "ability", PROPERTY_HINT_NODE_TYPE, "ZAbility"), PropertyInfo(Variant::NIL, "context", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
	ADD_SIGNAL(MethodInfo("on_ability_finished", PropertyInfo(Variant::OBJECT, "ability", PROPERTY_HINT_NODE_TYPE, "ZAbility"), PropertyInfo(Variant::BOOL, "cancelled")));
}

void ZAbilityManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_resolve_root_owner_if_needed();
		} break;
		case NOTIFICATION_READY: {
			_resolve_root_owner_if_needed();
		} break;
		case NOTIFICATION_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}
			double delta = get_process_delta_time();
			_poll_ability_input(delta);
			_tick_utility(delta);
		} break;
	}
}

void ZAbilityManager::_resolve_root_owner_if_needed() {
	if (root_owner_id.is_null() && auto_find_root_owner) {
		Node *parent = get_parent();
		if (parent != nullptr) {
			root_owner_id = parent->get_instance_id();
		}
	}
}

bool ZAbilityManager::_is_player_input_enabled() const {
	Node *owner = get_root_owner();
	if (owner == nullptr) {
		return false;
	}

	Z *z_singleton = Z::get_singleton();
	if (z_singleton == nullptr) {
		return false;
	}

	TypedArray<ZPlayer> players = z_singleton->get_players();
	for (int i = 0; i < players.size(); i++) {
		ZPlayer *player = Object::cast_to<ZPlayer>(players[i]);
		if (player != nullptr && player->get_possessed_scene() == owner) {
			return true;
		}
	}

	return false;
}

void ZAbilityManager::_poll_ability_input(double p_delta) {
	Node *owner = get_root_owner();
	if (owner == nullptr) {
		return;
	}

	bool input_enabled = _is_player_input_enabled();
	if (!input_enabled) {
		_release_all_held();
		return;
	}

	// Collect all unique input actions from child abilities
	HashSet<StringName> current_actions;
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		ZAbility *ability = Object::cast_to<ZAbility>(child);
		if (ability != nullptr && !ability->get_input_action().is_empty()) {
			current_actions.insert(StringName(ability->get_input_action()));
		}
	}

	// Poll each action
	Input *input = Input::get_singleton();
	for (const StringName &action : current_actions) {
		bool is_pressed = input->is_action_pressed(action);
		bool was_held = held_actions.has(action);

		if (is_pressed && !was_held) {
			// Press edge
			held_actions.insert(action);
			for (int i = 0; i < get_child_count(); i++) {
				Node *child = get_child(i);
				ZAbility *ability = Object::cast_to<ZAbility>(child);
				if (ability != nullptr && StringName(ability->get_input_action()) == action) {
					ability->ability_activate(owner);
				}
			}
		} else if (!is_pressed && was_held) {
			// Release edge
			held_actions.erase(action);
			for (int i = 0; i < get_child_count(); i++) {
				Node *child = get_child(i);
				ZAbility *ability = Object::cast_to<ZAbility>(child);
				if (ability != nullptr && StringName(ability->get_input_action()) == action) {
					if (ability->get_is_active() && ability->get_cancel_ability_on_release()) {
						ability->ability_stop(true);
					}
				}
			}
		}
	}
}

void ZAbilityManager::_release_all_held() {
	for (const StringName &action : held_actions) {
		for (int i = 0; i < get_child_count(); i++) {
			Node *child = get_child(i);
			ZAbility *ability = Object::cast_to<ZAbility>(child);
			if (ability != nullptr && StringName(ability->get_input_action()) == action) {
				if (ability->get_is_active() && ability->get_cancel_ability_on_release()) {
					ability->ability_stop(true);
				}
			}
		}
	}
	held_actions.clear();
}

void ZAbilityManager::_tick_utility(double p_delta) {
	if (!autorun_utility) {
		return;
	}

	Node *owner = get_root_owner();
	if (owner == nullptr) {
		return;
	}

	utility_timer -= p_delta;
	if (utility_timer <= 0.0) {
		_reset_utility_timer();

		float best_score = 0.0f;
		ZAbility *best_ability = nullptr;

		for (int i = 0; i < get_child_count(); i++) {
			Node *child = get_child(i);
			ZAbility *ability = Object::cast_to<ZAbility>(child);
			if (ability == nullptr || ability->get_is_active()) {
				continue;
			}

			float score = ability->get_utility_score(owner);
			if (score > best_score) {
				best_score = score;
				best_ability = ability;
			}
		}

		if (best_ability != nullptr) {
			best_ability->ability_activate(owner);
		}
	}
}

void ZAbilityManager::_reset_utility_timer() {
	double variance = UtilityFunctions::randf_range(-utility_variance, utility_variance);
	utility_timer = utility_frequency + variance;
	if (utility_timer < 0.01) {
		utility_timer = 0.01;
	}
}

void ZAbilityManager::set_root_owner(Node *p_owner) {
	if (p_owner != nullptr) {
		root_owner_id = p_owner->get_instance_id();
	} else {
		root_owner_id = ObjectID();
	}
}

Node *ZAbilityManager::get_root_owner() const {
	if (!root_owner_id.is_null()) {
		return Object::cast_to<Node>(ObjectDB::get_instance(root_owner_id));
	}
	return nullptr;
}

void ZAbilityManager::set_auto_find_root_owner(bool p_enabled) {
	auto_find_root_owner = p_enabled;
}

bool ZAbilityManager::get_auto_find_root_owner() const {
	return auto_find_root_owner;
}

void ZAbilityManager::set_autorun_utility(bool p_enabled) {
	autorun_utility = p_enabled;
}

bool ZAbilityManager::get_autorun_utility() const {
	return autorun_utility;
}

void ZAbilityManager::set_utility_frequency(double p_frequency) {
	utility_frequency = p_frequency;
}

double ZAbilityManager::get_utility_frequency() const {
	return utility_frequency;
}

void ZAbilityManager::set_utility_variance(double p_variance) {
	utility_variance = p_variance;
}

double ZAbilityManager::get_utility_variance() const {
	return utility_variance;
}

TypedArray<ZAbility> ZAbilityManager::get_abilities() const {
	TypedArray<ZAbility> result;
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		ZAbility *ability = Object::cast_to<ZAbility>(child);
		if (ability != nullptr) {
			result.push_back(ability);
		}
	}
	return result;
}

ZAbility *ZAbilityManager::find_ability(const String &p_name) const {
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		if (child->get_name() == StringName(p_name)) {
			ZAbility *ability = Object::cast_to<ZAbility>(child);
			if (ability != nullptr) {
				return ability;
			}
		}
	}
	return nullptr;
}

void ZAbilityManager::_notify_ability_activated(ZAbility *p_ability, const Variant &p_context) {
	emit_signal("on_ability_activated", p_ability, p_context);
}

void ZAbilityManager::_notify_ability_finished(ZAbility *p_ability, bool p_cancelled) {
	emit_signal("on_ability_finished", p_ability, p_cancelled);
}

ZAbilityManager::ZAbilityManager() {
	set_process(true);
}

ZAbilityManager::~ZAbilityManager() {
}

} // namespace godot
