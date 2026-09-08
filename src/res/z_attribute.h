#pragma once

#include "z_resource.h"

#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/variant/color.hpp>

namespace godot {

// A single numeric stat a ZCreature can hold (health, stamina, mana, armour...).
// The resource is the *definition*; the per-creature current value lives in
// ZCreatureData.attributes, keyed by this resource.
class ZAttribute : public ZResource {
	GDCLASS(ZAttribute, ZResource)

private:
	float max_value = 100.0f;
	// true: the attribute can never be damaged; current always equals max.
	bool is_static = false;
	// Fraction of max the attribute starts at (1.0 = full).
	float start_value_percent = 1.0f;
	// Optional multiplier on max_value, sampled at the creature's rank.
	// Set the curve's domain to span the rank range you use.
	Ref<Curve> rank_value_curve;
	Color damage_color = Color(1, 0, 0, 1);
	// Values are snapped to this many decimal places (2 = 0.01, 0 = whole numbers,
	// -1 = nearest ten).
	int round_to = 2;

protected:
	static void _bind_methods();

public:
	void set_max_value(float p_value);
	float get_max_value() const;

	void set_is_static(bool p_static);
	bool get_is_static() const;

	void set_start_value_percent(float p_percent);
	float get_start_value_percent() const;

	void set_rank_value_curve(const Ref<Curve> &p_curve);
	Ref<Curve> get_rank_value_curve() const;

	void set_damage_color(const Color &p_color);
	Color get_damage_color() const;

	void set_round_to(int p_round_to);
	int get_round_to() const;

	// max_value scaled by rank_value_curve sampled at p_rank (1.0 with no curve).
	float get_max_value_at_rank(int p_rank) const;
	// Value the attribute starts at for the given rank.
	float get_start_value_at_rank(int p_rank) const;
	// Snap p_value to round_to decimal places.
	float round_value(float p_value) const;

	ZAttribute();
	~ZAttribute();
};

} // namespace godot
