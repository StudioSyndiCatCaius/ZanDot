#include "z_creature.h"
#include "z_ability.h"
#include "z.h"
#include "../res/z_player.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void ZCreature::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_root_owner", "root_owner"), &ZCreature::set_root_owner);
	ClassDB::bind_method(D_METHOD("get_root_owner"), &ZCreature::get_root_owner);
	ClassDB::bind_method(D_METHOD("set_auto_find_root_owner", "auto_find_root_owner"), &ZCreature::set_auto_find_root_owner);
	ClassDB::bind_method(D_METHOD("get_auto_find_root_owner"), &ZCreature::get_auto_find_root_owner);
	ClassDB::bind_method(D_METHOD("set_autorun_utility", "autorun_utility"), &ZCreature::set_autorun_utility);
	ClassDB::bind_method(D_METHOD("get_autorun_utility"), &ZCreature::get_autorun_utility);
	ClassDB::bind_method(D_METHOD("set_utility_frequency", "utility_frequency"), &ZCreature::set_utility_frequency);
	ClassDB::bind_method(D_METHOD("get_utility_frequency"), &ZCreature::get_utility_frequency);
	ClassDB::bind_method(D_METHOD("set_utility_variance", "utility_variance"), &ZCreature::set_utility_variance);
	ClassDB::bind_method(D_METHOD("get_utility_variance"), &ZCreature::get_utility_variance);

	ClassDB::bind_method(D_METHOD("set_animation_player", "animation_player"), &ZCreature::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &ZCreature::get_animation_player);
	ClassDB::bind_method(D_METHOD("set_animation_tree", "animation_tree"), &ZCreature::set_animation_tree);
	ClassDB::bind_method(D_METHOD("get_animation_tree"), &ZCreature::get_animation_tree);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &ZCreature::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &ZCreature::get_data);
	ClassDB::bind_method(D_METHOD("set_reset_attributes_on_ready", "enabled"), &ZCreature::set_reset_attributes_on_ready);
	ClassDB::bind_method(D_METHOD("get_reset_attributes_on_ready"), &ZCreature::get_reset_attributes_on_ready);

	ClassDB::bind_method(D_METHOD("get_abilities"), &ZCreature::get_abilities);
	ClassDB::bind_method(D_METHOD("find_ability", "name"), &ZCreature::find_ability);

	ClassDB::bind_method(D_METHOD("fire_notify", "notify"), &ZCreature::fire_notify);

	// Attributes.
	ClassDB::bind_method(D_METHOD("attribute_get_current", "att"), &ZCreature::attribute_get_current);
	ClassDB::bind_method(D_METHOD("attribute_get_max", "att"), &ZCreature::attribute_get_max);
	ClassDB::bind_method(D_METHOD("attribute_get_percent", "att"), &ZCreature::attribute_get_percent);
	ClassDB::bind_method(D_METHOD("attribute_damage", "att", "amount", "instigator"), &ZCreature::attribute_damage, DEFVAL(nullptr));
	ClassDB::bind_method(D_METHOD("attribute_set_current", "att", "value"), &ZCreature::attribute_set_current);
	ClassDB::bind_method(D_METHOD("attribute_reset", "att"), &ZCreature::attribute_reset);
	ClassDB::bind_method(D_METHOD("attribute_reset_all"), &ZCreature::attribute_reset_all);

	ClassDB::bind_method(D_METHOD("Attribute_Get_Current", "att"), &ZCreature::attribute_get_current);
	ClassDB::bind_method(D_METHOD("Attribute_Get_Max", "att"), &ZCreature::attribute_get_max);
	ClassDB::bind_method(D_METHOD("Attribute_Get_Percent", "att"), &ZCreature::attribute_get_percent);
	ClassDB::bind_method(D_METHOD("Attribute_Damage", "att", "amount", "instigator"), &ZCreature::attribute_damage, DEFVAL(nullptr));
	ClassDB::bind_method(D_METHOD("Attribute_Set_Current", "att", "value"), &ZCreature::attribute_set_current);
	ClassDB::bind_method(D_METHOD("Attribute_Reset", "att"), &ZCreature::attribute_reset);
	ClassDB::bind_method(D_METHOD("Attribute_Reset_All"), &ZCreature::attribute_reset_all);

	// Resource slots.
	ClassDB::bind_method(D_METHOD("rslot_equip", "slot", "res"), &ZCreature::rslot_equip);
	ClassDB::bind_method(D_METHOD("rslot_unequip", "slot"), &ZCreature::rslot_unequip);
	ClassDB::bind_method(D_METHOD("rslot_unequip_all"), &ZCreature::rslot_unequip_all);
	ClassDB::bind_method(D_METHOD("rslot_get", "slot"), &ZCreature::rslot_get);

	ClassDB::bind_method(D_METHOD("RSlot_Equip", "slot", "res"), &ZCreature::rslot_equip);
	ClassDB::bind_method(D_METHOD("RSlot_Unequip", "slot"), &ZCreature::rslot_unequip);
	ClassDB::bind_method(D_METHOD("RSlot_Unequip_All"), &ZCreature::rslot_unequip_all);
	ClassDB::bind_method(D_METHOD("RSlot_Get", "slot"), &ZCreature::rslot_get);

	// Inventory.
	ClassDB::bind_method(D_METHOD("inventory_give", "res", "amount"), &ZCreature::inventory_give);
	ClassDB::bind_method(D_METHOD("inventory_give_list", "list"), &ZCreature::inventory_give_list);
	ClassDB::bind_method(D_METHOD("inventory_transfer", "list", "to"), &ZCreature::inventory_transfer);
	ClassDB::bind_method(D_METHOD("inventory_transfer_all", "to"), &ZCreature::inventory_transfer_all);
	ClassDB::bind_method(D_METHOD("inventory_get_amount", "res"), &ZCreature::inventory_get_amount);

	ClassDB::bind_method(D_METHOD("Inventory_Give", "res", "amount"), &ZCreature::inventory_give);
	ClassDB::bind_method(D_METHOD("Inventory_Give_List", "list"), &ZCreature::inventory_give_list);
	ClassDB::bind_method(D_METHOD("Inventory_Transfer", "list", "to"), &ZCreature::inventory_transfer);
	ClassDB::bind_method(D_METHOD("Inventory_Transfer_All", "to"), &ZCreature::inventory_transfer_all);
	ClassDB::bind_method(D_METHOD("Inventory_Get_Amount", "res"), &ZCreature::inventory_get_amount);

	// Levelling.
	ClassDB::bind_method(D_METHOD("leveling_xp_add", "res", "xp"), &ZCreature::leveling_xp_add);
	ClassDB::bind_method(D_METHOD("leveling_xp_set", "res", "xp"), &ZCreature::leveling_xp_set);
	ClassDB::bind_method(D_METHOD("leveling_level_set", "res", "lvl"), &ZCreature::leveling_level_set);
	ClassDB::bind_method(D_METHOD("leveling_level_get", "res"), &ZCreature::leveling_level_get);
	ClassDB::bind_method(D_METHOD("leveling_xp_get_current", "res"), &ZCreature::leveling_xp_get_current);
	ClassDB::bind_method(D_METHOD("leveling_xp_get_percent", "res"), &ZCreature::leveling_xp_get_percent);
	ClassDB::bind_method(D_METHOD("leveling_xp_min", "res"), &ZCreature::leveling_xp_min);
	ClassDB::bind_method(D_METHOD("leveling_xp_max", "res"), &ZCreature::leveling_xp_max);

	ClassDB::bind_method(D_METHOD("Leveling_XP_Add", "res", "xp"), &ZCreature::leveling_xp_add);
	ClassDB::bind_method(D_METHOD("Leveling_XP_Set", "res", "xp"), &ZCreature::leveling_xp_set);
	ClassDB::bind_method(D_METHOD("Leveling_Level_Set", "res", "lvl"), &ZCreature::leveling_level_set);
	ClassDB::bind_method(D_METHOD("Leveling_Level_Get", "res"), &ZCreature::leveling_level_get);
	ClassDB::bind_method(D_METHOD("Leveling_XP_Get_Current", "res"), &ZCreature::leveling_xp_get_current);
	ClassDB::bind_method(D_METHOD("Leveling_XP_Get_Percent", "res"), &ZCreature::leveling_xp_get_percent);
	ClassDB::bind_method(D_METHOD("Leveling_XP_Min", "res"), &ZCreature::leveling_xp_min);
	ClassDB::bind_method(D_METHOD("Leveling_XP_Max", "res"), &ZCreature::leveling_xp_max);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "ZCreatureData"), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_attributes_on_ready"), "set_reset_attributes_on_ready", "get_reset_attributes_on_ready");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_owner", PROPERTY_HINT_NODE_TYPE, "Node"), "set_root_owner", "get_root_owner");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_find_root_owner"), "set_auto_find_root_owner", "get_auto_find_root_owner");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autorun_utility"), "set_autorun_utility", "get_autorun_utility");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "utility_frequency", PROPERTY_HINT_RANGE, "0.01,10,0.01,or_greater"), "set_utility_frequency", "get_utility_frequency");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "utility_variance", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_utility_variance", "get_utility_variance");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "animation_player", PROPERTY_HINT_NODE_TYPE, "AnimationPlayer"), "set_animation_player", "get_animation_player");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "animation_tree", PROPERTY_HINT_NODE_TYPE, "AnimationTree"), "set_animation_tree", "get_animation_tree");

	ADD_SIGNAL(MethodInfo("on_ability_activated", PropertyInfo(Variant::OBJECT, "ability", PROPERTY_HINT_NODE_TYPE, "ZAbility"), PropertyInfo(Variant::NIL, "context", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
	ADD_SIGNAL(MethodInfo("on_ability_finished", PropertyInfo(Variant::OBJECT, "ability", PROPERTY_HINT_NODE_TYPE, "ZAbility"), PropertyInfo(Variant::BOOL, "cancelled")));
	ADD_SIGNAL(MethodInfo("on_creature_notify", PropertyInfo(Variant::STRING, "notify")));

	ADD_SIGNAL(MethodInfo("on_attribute_damaged",
			PropertyInfo(Variant::OBJECT, "att", PROPERTY_HINT_RESOURCE_TYPE, "ZAttribute"),
			PropertyInfo(Variant::FLOAT, "amount"),
			PropertyInfo(Variant::OBJECT, "instigator", PROPERTY_HINT_NODE_TYPE, "ZCreature")));

	ADD_SIGNAL(MethodInfo("on_rslot_equip",
			PropertyInfo(Variant::OBJECT, "creature", PROPERTY_HINT_NODE_TYPE, "ZCreature"),
			PropertyInfo(Variant::OBJECT, "slot", PROPERTY_HINT_RESOURCE_TYPE, "Resource"),
			PropertyInfo(Variant::OBJECT, "res", PROPERTY_HINT_RESOURCE_TYPE, "Resource")));
	ADD_SIGNAL(MethodInfo("on_rslot_unequip",
			PropertyInfo(Variant::OBJECT, "creature", PROPERTY_HINT_NODE_TYPE, "ZCreature"),
			PropertyInfo(Variant::OBJECT, "slot", PROPERTY_HINT_RESOURCE_TYPE, "Resource"),
			PropertyInfo(Variant::OBJECT, "res", PROPERTY_HINT_RESOURCE_TYPE, "Resource")));

	ADD_SIGNAL(MethodInfo("on_inventory_edit",
			PropertyInfo(Variant::OBJECT, "creature", PROPERTY_HINT_NODE_TYPE, "ZCreature"),
			PropertyInfo(Variant::OBJECT, "res", PROPERTY_HINT_RESOURCE_TYPE, "Resource"),
			PropertyInfo(Variant::INT, "amount_changed")));

	ADD_SIGNAL(MethodInfo("on_xp_change",
			PropertyInfo(Variant::OBJECT, "creature", PROPERTY_HINT_NODE_TYPE, "ZCreature"),
			PropertyInfo(Variant::OBJECT, "res", PROPERTY_HINT_RESOURCE_TYPE, "ZLeveler"),
			PropertyInfo(Variant::FLOAT, "old_xp"),
			PropertyInfo(Variant::FLOAT, "new_xp")));
	ADD_SIGNAL(MethodInfo("on_level_change",
			PropertyInfo(Variant::OBJECT, "creature", PROPERTY_HINT_NODE_TYPE, "ZCreature"),
			PropertyInfo(Variant::OBJECT, "res", PROPERTY_HINT_RESOURCE_TYPE, "ZLeveler"),
			PropertyInfo(Variant::INT, "old_lvl"),
			PropertyInfo(Variant::INT, "new_lvl")));
}

void ZCreature::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_resolve_root_owner_if_needed();
		} break;
		case NOTIFICATION_READY: {
			_resolve_root_owner_if_needed();
			if (reset_attributes_on_ready && !Engine::get_singleton()->is_editor_hint()) {
				attribute_reset_all();
			}
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

void ZCreature::_resolve_root_owner_if_needed() {
	if (root_owner_id.is_null() && auto_find_root_owner) {
		Node *parent = get_parent();
		if (parent != nullptr) {
			root_owner_id = parent->get_instance_id();
		}
	}
}

bool ZCreature::_is_player_input_enabled() const {
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

void ZCreature::_poll_ability_input(double p_delta) {
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

void ZCreature::_release_all_held() {
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

void ZCreature::_tick_utility(double p_delta) {
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

void ZCreature::_reset_utility_timer() {
	double variance = UtilityFunctions::randf_range(-utility_variance, utility_variance);
	utility_timer = utility_frequency + variance;
	if (utility_timer < 0.01) {
		utility_timer = 0.01;
	}
}

void ZCreature::set_root_owner(Node *p_owner) {
	if (p_owner != nullptr) {
		root_owner_id = p_owner->get_instance_id();
	} else {
		root_owner_id = ObjectID();
	}
}

Node *ZCreature::get_root_owner() const {
	if (!root_owner_id.is_null()) {
		return Object::cast_to<Node>(ObjectDB::get_instance(root_owner_id));
	}
	return nullptr;
}

void ZCreature::set_auto_find_root_owner(bool p_enabled) {
	auto_find_root_owner = p_enabled;
}

bool ZCreature::get_auto_find_root_owner() const {
	return auto_find_root_owner;
}

void ZCreature::set_autorun_utility(bool p_enabled) {
	autorun_utility = p_enabled;
}

bool ZCreature::get_autorun_utility() const {
	return autorun_utility;
}

void ZCreature::set_utility_frequency(double p_frequency) {
	utility_frequency = p_frequency;
}

double ZCreature::get_utility_frequency() const {
	return utility_frequency;
}

void ZCreature::set_utility_variance(double p_variance) {
	utility_variance = p_variance;
}

double ZCreature::get_utility_variance() const {
	return utility_variance;
}

void ZCreature::set_animation_player(AnimationPlayer *p_animation_player) {
	animation_player = p_animation_player;
}

AnimationPlayer *ZCreature::get_animation_player() const {
	return animation_player;
}

void ZCreature::set_animation_tree(AnimationTree *p_animation_tree) {
	animation_tree = p_animation_tree;
}

AnimationTree *ZCreature::get_animation_tree() const {
	return animation_tree;
}

TypedArray<ZAbility> ZCreature::get_abilities() const {
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

ZAbility *ZCreature::find_ability(const String &p_name) const {
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

void ZCreature::fire_notify(const String &p_notify) {
	emit_signal("on_creature_notify", p_notify);

	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		ZAbility *ability = Object::cast_to<ZAbility>(child);
		if (ability != nullptr) {
			ability->creature_notify(p_notify);
		}
	}
}

void ZCreature::set_data(const Ref<ZCreatureData> &p_data) {
	data = p_data;
}

Ref<ZCreatureData> ZCreature::get_data() const {
	return data;
}

Ref<ZCreatureData> ZCreature::_ensure_data() {
	if (data.is_null()) {
		data.instantiate();
	}
	return data;
}

void ZCreature::set_reset_attributes_on_ready(bool p_enabled) {
	reset_attributes_on_ready = p_enabled;
}

bool ZCreature::get_reset_attributes_on_ready() const {
	return reset_attributes_on_ready;
}

float ZCreature::attribute_get_current(const Ref<ZAttribute> &p_att) const {
	if (p_att.is_null()) {
		return 0.0f;
	}
	const int rank = data.is_valid() ? data->get_rank() : 0;
	if (p_att->get_is_static()) {
		return p_att->get_max_value_at_rank(rank);
	}
	if (data.is_valid()) {
		const Dictionary attributes = data->get_attributes();
		if (attributes.has(p_att)) {
			return (float)(double)attributes[p_att];
		}
	}
	return p_att->get_start_value_at_rank(rank);
}

float ZCreature::attribute_get_max(const Ref<ZAttribute> &p_att) const {
	if (p_att.is_null()) {
		return 0.0f;
	}
	return p_att->get_max_value_at_rank(data.is_valid() ? data->get_rank() : 0);
}

float ZCreature::attribute_get_percent(const Ref<ZAttribute> &p_att) const {
	const float max_value = attribute_get_max(p_att);
	if (max_value <= 0.0f) {
		return 0.0f;
	}
	const float percent = attribute_get_current(p_att) / max_value;
	return percent < 0.0f ? 0.0f : (percent > 1.0f ? 1.0f : percent);
}

void ZCreature::attribute_set_current(const Ref<ZAttribute> &p_att, float p_value) {
	if (p_att.is_null() || p_att->get_is_static()) {
		return;
	}
	const float max_value = attribute_get_max(p_att);
	float value = p_value;
	if (value < 0.0f) {
		value = 0.0f;
	} else if (value > max_value) {
		value = max_value;
	}
	_ensure_data()->get_attributes()[p_att] = p_att->round_value(value);
}

void ZCreature::attribute_damage(const Ref<ZAttribute> &p_att, float p_amount, ZCreature *p_instigator) {
	if (p_att.is_null() || p_att->get_is_static()) {
		return;
	}

	const float current = attribute_get_current(p_att);
	attribute_set_current(p_att, current - p_amount);
	// Report what landed, so overkill and overheal don't produce phantom numbers.
	const float applied = current - attribute_get_current(p_att);
	if (applied == 0.0f) {
		return;
	}
	emit_signal("on_attribute_damaged", p_att, applied, p_instigator);
}

void ZCreature::attribute_reset(const Ref<ZAttribute> &p_att) {
	if (p_att.is_null() || p_att->get_is_static()) {
		return;
	}
	const int rank = data.is_valid() ? data->get_rank() : 0;
	_ensure_data()->get_attributes()[p_att] = p_att->get_start_value_at_rank(rank);
}

void ZCreature::attribute_reset_all() {
	if (data.is_null()) {
		return;
	}
	const Dictionary attributes = data->get_attributes();
	const Array keys = attributes.keys();
	for (int i = 0; i < keys.size(); i++) {
		Ref<ZAttribute> att = keys[i];
		attribute_reset(att);
	}
}

Ref<Resource> ZCreature::rslot_get(const Ref<Resource> &p_slot) const {
	if (p_slot.is_null() || data.is_null()) {
		return Ref<Resource>();
	}
	const Dictionary equipment = data->get_equipment();
	if (!equipment.has(p_slot)) {
		return Ref<Resource>();
	}
	Ref<Resource> res = equipment[p_slot];
	return res;
}

void ZCreature::rslot_equip(const Ref<Resource> &p_slot, const Ref<Resource> &p_res) {
	if (p_slot.is_null()) {
		return;
	}
	if (p_res.is_null()) {
		rslot_unequip(p_slot);
		return;
	}

	// Slots hold one resource; clear the old one so listeners see the swap.
	rslot_unequip(p_slot);
	_ensure_data()->get_equipment()[p_slot] = p_res;
	emit_signal("on_rslot_equip", this, p_slot, p_res);
}

void ZCreature::rslot_unequip(const Ref<Resource> &p_slot) {
	if (p_slot.is_null() || data.is_null()) {
		return;
	}
	Dictionary equipment = data->get_equipment();
	if (!equipment.has(p_slot)) {
		return;
	}
	Ref<Resource> old_res = equipment[p_slot];
	equipment.erase(p_slot);
	emit_signal("on_rslot_unequip", this, p_slot, old_res);
}

void ZCreature::rslot_unequip_all() {
	if (data.is_null()) {
		return;
	}
	const Array slots = data->get_equipment().keys();
	for (int i = 0; i < slots.size(); i++) {
		Ref<Resource> slot = slots[i];
		rslot_unequip(slot);
	}
}

int ZCreature::inventory_get_amount(const Ref<Resource> &p_res) const {
	if (p_res.is_null() || data.is_null()) {
		return 0;
	}
	const Dictionary inventory = data->get_inventory();
	if (!inventory.has(p_res)) {
		return 0;
	}
	return (int)(int64_t)inventory[p_res];
}

void ZCreature::inventory_give(const Ref<Resource> &p_res, int p_amount) {
	if (p_res.is_null() || p_amount == 0) {
		return;
	}

	const int held = inventory_get_amount(p_res);
	int total = held + p_amount;
	if (total < 0) {
		total = 0;
	}
	const int changed = total - held;
	if (changed == 0) {
		return;
	}

	Dictionary inventory = _ensure_data()->get_inventory();
	if (total == 0) {
		inventory.erase(p_res);
	} else {
		inventory[p_res] = total;
	}
	emit_signal("on_inventory_edit", this, p_res, changed);
}

void ZCreature::inventory_give_list(const Dictionary &p_list) {
	const Array keys = p_list.keys();
	for (int i = 0; i < keys.size(); i++) {
		Ref<Resource> res = keys[i];
		inventory_give(res, (int)(int64_t)p_list[keys[i]]);
	}
}

void ZCreature::inventory_transfer(const Dictionary &p_list, ZCreature *p_to) {
	if (p_to == nullptr || p_to == this) {
		return;
	}

	const Array keys = p_list.keys();
	for (int i = 0; i < keys.size(); i++) {
		Ref<Resource> res = keys[i];
		if (res.is_null()) {
			continue;
		}
		int amount = (int)(int64_t)p_list[keys[i]];
		const int held = inventory_get_amount(res);
		if (amount > held) {
			amount = held;
		}
		if (amount <= 0) {
			continue;
		}
		inventory_give(res, -amount);
		p_to->inventory_give(res, amount);
	}
}

void ZCreature::inventory_transfer_all(ZCreature *p_to) {
	if (p_to == nullptr || p_to == this || data.is_null()) {
		return;
	}
	// Snapshot first — inventory_give mutates the dictionary we would iterate.
	inventory_transfer(data->get_inventory().duplicate(false), p_to);
}

float ZCreature::leveling_xp_get_current(const Ref<ZLeveler> &p_res) const {
	if (p_res.is_null() || data.is_null()) {
		return 0.0f;
	}
	const Dictionary xp = data->get_xp();
	if (!xp.has(p_res)) {
		return 0.0f;
	}
	return (float)(double)xp[p_res];
}

int ZCreature::leveling_level_get(const Ref<ZLeveler> &p_res) const {
	if (p_res.is_null()) {
		return 0;
	}
	return p_res->get_level_for_xp(leveling_xp_get_current(p_res));
}

float ZCreature::leveling_xp_min(const Ref<ZLeveler> &p_res) const {
	if (p_res.is_null()) {
		return 0.0f;
	}
	return p_res->get_xp_for_level(leveling_level_get(p_res));
}

float ZCreature::leveling_xp_max(const Ref<ZLeveler> &p_res) const {
	if (p_res.is_null()) {
		return 0.0f;
	}
	const int level = leveling_level_get(p_res);
	if (level >= p_res->get_max_level()) {
		return p_res->get_xp_for_level(level);
	}
	return p_res->get_xp_for_level(level + 1);
}

float ZCreature::leveling_xp_get_percent(const Ref<ZLeveler> &p_res) const {
	if (p_res.is_null()) {
		return 0.0f;
	}
	if (leveling_level_get(p_res) >= p_res->get_max_level()) {
		return 1.0f;
	}
	const float min_xp = leveling_xp_min(p_res);
	const float max_xp = leveling_xp_max(p_res);
	if (max_xp <= min_xp) {
		return 1.0f;
	}
	const float percent = (leveling_xp_get_current(p_res) - min_xp) / (max_xp - min_xp);
	return percent < 0.0f ? 0.0f : (percent > 1.0f ? 1.0f : percent);
}

void ZCreature::leveling_xp_set(const Ref<ZLeveler> &p_res, float p_xp) {
	if (p_res.is_null()) {
		return;
	}

	const float old_xp = leveling_xp_get_current(p_res);
	const int old_lvl = p_res->get_level_for_xp(old_xp);

	const float floor_xp = p_res->get_xp_for_level(0);
	const float cap_xp = p_res->get_xp_for_level(p_res->get_max_level());
	float new_xp = p_res->round_xp(p_xp);
	if (new_xp < floor_xp) {
		new_xp = floor_xp;
	} else if (new_xp > cap_xp) {
		new_xp = cap_xp;
	}
	if (new_xp == old_xp) {
		return;
	}

	_ensure_data()->get_xp()[p_res] = new_xp;
	emit_signal("on_xp_change", this, p_res, old_xp, new_xp);

	const int new_lvl = p_res->get_level_for_xp(new_xp);
	if (new_lvl != old_lvl) {
		emit_signal("on_level_change", this, p_res, old_lvl, new_lvl);
	}
}

void ZCreature::leveling_xp_add(const Ref<ZLeveler> &p_res, float p_xp) {
	if (p_res.is_null()) {
		return;
	}
	leveling_xp_set(p_res, leveling_xp_get_current(p_res) + p_xp);
}

void ZCreature::leveling_level_set(const Ref<ZLeveler> &p_res, int p_lvl) {
	if (p_res.is_null()) {
		return;
	}
	leveling_xp_set(p_res, p_res->get_xp_for_level(p_lvl));
}

void ZCreature::_notify_ability_activated(ZAbility *p_ability, const Variant &p_context) {
	emit_signal("on_ability_activated", p_ability, p_context);
}

void ZCreature::_notify_ability_finished(ZAbility *p_ability, bool p_cancelled) {
	emit_signal("on_ability_finished", p_ability, p_cancelled);
}

ZCreature::ZCreature() {
	set_process(true);
}

ZCreature::~ZCreature() {
}

} // namespace godot
