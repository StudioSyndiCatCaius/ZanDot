#pragma once
#include "../res/z_attribute.h"
#include "../res/z_creature_data.h"
#include "../res/z_leveler.h"

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
class ZAbility;

// Core gameplay actor component. Owns abilities, and the ZCreatureData holding
// this actor's attributes, XP, inventory and equipment.
class ZCreature : public Node {
	GDCLASS(ZCreature, Node)
private:
	ObjectID root_owner_id;
	bool auto_find_root_owner = true;
	bool autorun_utility = false;
	double utility_frequency = 0.2;
	double utility_variance = 0.05;
	AnimationPlayer *animation_player = nullptr;
	AnimationTree *animation_tree = nullptr;

	Ref<ZCreatureData> data;
	// Sets every attribute already present in data to its start value on ready.
	// Turn off when the data is restored from a save before the node enters the tree.
	bool reset_attributes_on_ready = true;

	double utility_timer = 0.0;
	HashSet<StringName> held_actions;

	void _resolve_root_owner_if_needed();
	bool _is_player_input_enabled() const;
	void _poll_ability_input(double p_delta);
	void _release_all_held();
	void _tick_utility(double p_delta);
	void _reset_utility_timer();
	// data is created on first use so scripts never have to null-check it.
	Ref<ZCreatureData> _ensure_data();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_root_owner(Node *p_owner);
	Node *get_root_owner() const;
	void set_auto_find_root_owner(bool p_enabled);
	bool get_auto_find_root_owner() const;
	void set_autorun_utility(bool p_enabled);
	bool get_autorun_utility() const;
	void set_utility_frequency(double p_frequency);
	double get_utility_frequency() const;
	void set_utility_variance(double p_variance);
	double get_utility_variance() const;

	void set_animation_player(AnimationPlayer *p_animation_player);
	AnimationPlayer *get_animation_player() const;
	void set_animation_tree(AnimationTree *p_animation_tree);
	AnimationTree *get_animation_tree() const;

	void set_data(const Ref<ZCreatureData> &p_data);
	Ref<ZCreatureData> get_data() const;

	void set_reset_attributes_on_ready(bool p_enabled);
	bool get_reset_attributes_on_ready() const;

	// --- Attributes ---------------------------------------------------------
	// Current value; the attribute's start value when data holds no entry yet.
	float attribute_get_current(const Ref<ZAttribute> &p_att) const;
	// max_value scaled by the attribute's rank curve at data.rank.
	float attribute_get_max(const Ref<ZAttribute> &p_att) const;
	// current / max, 0..1 (0 when max is 0).
	float attribute_get_percent(const Ref<ZAttribute> &p_att) const;

	// Subtracts p_amount from the attribute (negative heals), clamped to 0..max.
	// No-op for static attributes. Emits on_attribute_damaged with the amount
	// actually applied.
	void attribute_damage(const Ref<ZAttribute> &p_att, float p_amount, ZCreature *p_instigator = nullptr);
	// Writes an absolute value, clamped to 0..max. No signal.
	void attribute_set_current(const Ref<ZAttribute> &p_att, float p_value);
	// Restores the attribute to its start value for the current rank.
	void attribute_reset(const Ref<ZAttribute> &p_att);
	// attribute_reset for every attribute present in data.
	void attribute_reset_all();

	// --- Resource slots (equipment) -----------------------------------------
	void rslot_equip(const Ref<Resource> &p_slot, const Ref<Resource> &p_res);
	void rslot_unequip(const Ref<Resource> &p_slot);
	void rslot_unequip_all();
	Ref<Resource> rslot_get(const Ref<Resource> &p_slot) const;

	// --- Inventory ----------------------------------------------------------
	// Adds p_amount (negative takes away). Counts at or below zero drop the key.
	void inventory_give(const Ref<Resource> &p_res, int p_amount);
	void inventory_give_list(const Dictionary &p_list);
	// Moves the listed resources to p_to, capped at what is actually held.
	void inventory_transfer(const Dictionary &p_list, ZCreature *p_to);
	void inventory_transfer_all(ZCreature *p_to);
	int inventory_get_amount(const Ref<Resource> &p_res) const;

	// --- Levelling ----------------------------------------------------------
	void leveling_xp_add(const Ref<ZLeveler> &p_res, float p_xp);
	void leveling_xp_set(const Ref<ZLeveler> &p_res, float p_xp);

	void leveling_level_set(const Ref<ZLeveler> &p_res, int p_lvl);
	int leveling_level_get(const Ref<ZLeveler> &p_res) const;

	float leveling_xp_get_current(const Ref<ZLeveler> &p_res) const;
	// Progress through the current level, 0..1 (1.0 at max level).
	float leveling_xp_get_percent(const Ref<ZLeveler> &p_res) const;

	// XP thresholds bounding the current level.
	float leveling_xp_min(const Ref<ZLeveler> &p_res) const;
	float leveling_xp_max(const Ref<ZLeveler> &p_res) const;

	TypedArray<ZAbility> get_abilities() const;
	ZAbility *find_ability(const String &p_name) const;

	// Broadcasts p_notify via on_creature_notify and to every child ZAbility.
	void fire_notify(const String &p_notify);

	void _notify_ability_activated(ZAbility *p_ability, const Variant &p_context);
	void _notify_ability_finished(ZAbility *p_ability, bool p_cancelled);

	ZCreature();
	~ZCreature();
};
} // namespace godot
