#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
class ZAbility;

class ZAbilityManager : public Node {
	GDCLASS(ZAbilityManager, Node)
private:
	ObjectID root_owner_id;
	bool auto_find_root_owner = true;
	bool autorun_utility = false;
	double utility_frequency = 0.2;
	double utility_variance = 0.05;

	double utility_timer = 0.0;
	HashSet<StringName> held_actions;

	void _resolve_root_owner_if_needed();
	bool _is_player_input_enabled() const;
	void _poll_ability_input(double p_delta);
	void _release_all_held();
	void _tick_utility(double p_delta);
	void _reset_utility_timer();

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

	TypedArray<ZAbility> get_abilities() const;
	ZAbility *find_ability(const String &p_name) const;

	void _notify_ability_activated(ZAbility *p_ability, const Variant &p_context);
	void _notify_ability_finished(ZAbility *p_ability, bool p_cancelled);

	ZAbilityManager();
	~ZAbilityManager();
};
} // namespace godot
