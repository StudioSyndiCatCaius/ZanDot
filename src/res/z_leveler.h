#pragma once

#include "z_resource.h"

#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// A progressive leveller (character level, weapon proficiency, faction rank...).
// The resource is the *definition*; the per-creature XP total lives in
// ZCreatureData.xp, keyed by this resource. Level is derived from XP.
class ZLeveler : public ZResource {
	GDCLASS(ZLeveler, ZResource)

private:
	String level_name;
	int max_level = 99;
	// Cumulative XP needed to reach a level, sampled at the level number.
	// Set the curve's domain to 0 … max_level and its range to your XP scale.
	// With no curve one XP is one level.
	Ref<Curve> xp_curve;
	// XP is snapped to this many decimal places (2 = 0.01, 0 = whole numbers).
	int round_xp_to = 2;

protected:
	static void _bind_methods();

public:
	void set_level_name(const String &p_name);
	String get_level_name() const;

	void set_max_level(int p_level);
	int get_max_level() const;

	void set_xp_curve(const Ref<Curve> &p_curve);
	Ref<Curve> get_xp_curve() const;

	void set_round_xp_to(int p_round_to);
	int get_round_xp_to() const;

	// Cumulative XP required to be at p_level (clamped to 0 … max_level).
	float get_xp_for_level(int p_level) const;
	// Highest level whose XP requirement p_xp meets.
	int get_level_for_xp(float p_xp) const;
	// Snap p_xp to round_xp_to decimal places.
	float round_xp(float p_xp) const;

	ZLeveler();
	~ZLeveler();
};

} // namespace godot
