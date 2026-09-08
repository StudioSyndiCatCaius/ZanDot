#include "z_leveler.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZLeveler::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_level_name", "level_name"), &ZLeveler::set_level_name);
	ClassDB::bind_method(D_METHOD("get_level_name"), &ZLeveler::get_level_name);

	ClassDB::bind_method(D_METHOD("set_max_level", "max_level"), &ZLeveler::set_max_level);
	ClassDB::bind_method(D_METHOD("get_max_level"), &ZLeveler::get_max_level);

	ClassDB::bind_method(D_METHOD("set_xp_curve", "xp_curve"), &ZLeveler::set_xp_curve);
	ClassDB::bind_method(D_METHOD("get_xp_curve"), &ZLeveler::get_xp_curve);

	ClassDB::bind_method(D_METHOD("set_round_xp_to", "round_xp_to"), &ZLeveler::set_round_xp_to);
	ClassDB::bind_method(D_METHOD("get_round_xp_to"), &ZLeveler::get_round_xp_to);

	ClassDB::bind_method(D_METHOD("get_xp_for_level", "level"), &ZLeveler::get_xp_for_level);
	ClassDB::bind_method(D_METHOD("get_level_for_xp", "xp"), &ZLeveler::get_level_for_xp);
	ClassDB::bind_method(D_METHOD("round_xp", "xp"), &ZLeveler::round_xp);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "level_name"), "set_level_name", "get_level_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_level", PROPERTY_HINT_RANGE, "1,999,1,or_greater"), "set_max_level", "get_max_level");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "xp_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_xp_curve", "get_xp_curve");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_xp_to", PROPERTY_HINT_RANGE, "-3,6,1"), "set_round_xp_to", "get_round_xp_to");
}

ZLeveler::ZLeveler() {
}

ZLeveler::~ZLeveler() {
}

void ZLeveler::set_level_name(const String &p_name) {
	if (level_name == p_name) {
		return;
	}
	level_name = p_name;
	emit_changed();
}

String ZLeveler::get_level_name() const {
	return level_name;
}

void ZLeveler::set_max_level(int p_level) {
	const int clamped = p_level < 1 ? 1 : p_level;
	if (max_level == clamped) {
		return;
	}
	max_level = clamped;
	emit_changed();
}

int ZLeveler::get_max_level() const {
	return max_level;
}

void ZLeveler::set_xp_curve(const Ref<Curve> &p_curve) {
	if (xp_curve == p_curve) {
		return;
	}
	xp_curve = p_curve;
	emit_changed();
}

Ref<Curve> ZLeveler::get_xp_curve() const {
	return xp_curve;
}

void ZLeveler::set_round_xp_to(int p_round_to) {
	if (round_xp_to == p_round_to) {
		return;
	}
	round_xp_to = p_round_to;
	emit_changed();
}

int ZLeveler::get_round_xp_to() const {
	return round_xp_to;
}

float ZLeveler::get_xp_for_level(int p_level) const {
	int level = p_level;
	if (level < 0) {
		level = 0;
	} else if (level > max_level) {
		level = max_level;
	}
	if (xp_curve.is_null()) {
		return round_xp((float)level);
	}
	return round_xp(xp_curve->sample((float)level));
}

int ZLeveler::get_level_for_xp(float p_xp) const {
	// Walk down so a flat tail on the curve resolves to the highest matching level.
	for (int level = max_level; level > 0; level--) {
		if (p_xp >= get_xp_for_level(level)) {
			return level;
		}
	}
	return 0;
}

float ZLeveler::round_xp(float p_xp) const {
	const double step = UtilityFunctions::pow(10.0, -(double)round_xp_to);
	if (step <= 0.0) {
		return p_xp;
	}
	return (float)UtilityFunctions::snappedf(p_xp, step);
}
