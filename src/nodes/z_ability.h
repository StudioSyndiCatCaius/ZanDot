#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {
class ZCreature;

class ZAbility : public Node {
	GDCLASS(ZAbility, Node)
private:
	String input_action;
	bool cancel_ability_on_release = false;
	bool is_active = false;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	virtual void on_ability_activated(const Variant &p_context);
	virtual void on_ability_finished(bool p_cancelled);
	virtual void on_process_activated_ability(double p_delta);
	virtual float on_get_utility_score(const Variant &p_context);
	virtual bool on_ability_can_activate(const Variant &p_context);
	virtual void on_creature_notify(const String &p_notify);

	GDVIRTUAL1(_on_ability_activated, Variant)
	GDVIRTUAL1(_on_ability_finished, bool)
	GDVIRTUAL1(_process_activated_ability, double)
	GDVIRTUAL1R(float, _get_utility_score, Variant)
	GDVIRTUAL1R(bool, _ability_can_activate, Variant)
	GDVIRTUAL1(_on_creature_notify, String)

public:
	void set_input_action(const String &p_action);
	String get_input_action() const;
	void set_cancel_ability_on_release(bool p_enabled);
	bool get_cancel_ability_on_release() const;
	bool get_is_active() const;

	bool ability_activate(const Variant &p_context = Variant());
	void ability_stop(bool p_cancelled = false);
	bool ability_can_activate(const Variant &p_context = Variant());
	float get_utility_score(const Variant &p_context = Variant());
	void creature_notify(const String &p_notify);

	ZCreature *get_creature() const;
	Node *get_root_owner() const;

	ZAbility();
	~ZAbility();
};
} // namespace godot
