#include "z_attribute.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZAttribute::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_value", "max_value"), &ZAttribute::set_max_value);
	ClassDB::bind_method(D_METHOD("get_max_value"), &ZAttribute::get_max_value);

	ClassDB::bind_method(D_METHOD("set_is_static", "is_static"), &ZAttribute::set_is_static);
	ClassDB::bind_method(D_METHOD("get_is_static"), &ZAttribute::get_is_static);

	ClassDB::bind_method(D_METHOD("set_start_value_percent", "start_value_percent"), &ZAttribute::set_start_value_percent);
	ClassDB::bind_method(D_METHOD("get_start_value_percent"), &ZAttribute::get_start_value_percent);

	ClassDB::bind_method(D_METHOD("set_rank_value_curve", "rank_value_curve"), &ZAttribute::set_rank_value_curve);
	ClassDB::bind_method(D_METHOD("get_rank_value_curve"), &ZAttribute::get_rank_value_curve);

	ClassDB::bind_method(D_METHOD("set_damage_color", "damage_color"), &ZAttribute::set_damage_color);
	ClassDB::bind_method(D_METHOD("get_damage_color"), &ZAttribute::get_damage_color);

	ClassDB::bind_method(D_METHOD("set_round_to", "round_to"), &ZAttribute::set_round_to);
	ClassDB::bind_method(D_METHOD("get_round_to"), &ZAttribute::get_round_to);

	ClassDB::bind_method(D_METHOD("get_max_value_at_rank", "rank"), &ZAttribute::get_max_value_at_rank);
	ClassDB::bind_method(D_METHOD("get_start_value_at_rank", "rank"), &ZAttribute::get_start_value_at_rank);
	ClassDB::bind_method(D_METHOD("round_value", "value"), &ZAttribute::round_value);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_value"), "set_max_value", "get_max_value");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_static"), "set_is_static", "get_is_static");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "start_value_percent", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_start_value_percent", "get_start_value_percent");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rank_value_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_rank_value_curve", "get_rank_value_curve");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "damage_color"), "set_damage_color", "get_damage_color");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_to", PROPERTY_HINT_RANGE, "-3,6,1"), "set_round_to", "get_round_to");
}

ZAttribute::ZAttribute() {
}

ZAttribute::~ZAttribute() {
}

void ZAttribute::set_max_value(float p_value) {
	if (max_value == p_value) {
		return;
	}
	max_value = p_value;
	emit_changed();
}

float ZAttribute::get_max_value() const {
	return max_value;
}

void ZAttribute::set_is_static(bool p_static) {
	if (is_static == p_static) {
		return;
	}
	is_static = p_static;
	emit_changed();
}

bool ZAttribute::get_is_static() const {
	return is_static;
}

void ZAttribute::set_start_value_percent(float p_percent) {
	if (start_value_percent == p_percent) {
		return;
	}
	start_value_percent = p_percent;
	emit_changed();
}

float ZAttribute::get_start_value_percent() const {
	return start_value_percent;
}

void ZAttribute::set_rank_value_curve(const Ref<Curve> &p_curve) {
	if (rank_value_curve == p_curve) {
		return;
	}
	rank_value_curve = p_curve;
	emit_changed();
}

Ref<Curve> ZAttribute::get_rank_value_curve() const {
	return rank_value_curve;
}

void ZAttribute::set_damage_color(const Color &p_color) {
	if (damage_color == p_color) {
		return;
	}
	damage_color = p_color;
	emit_changed();
}

Color ZAttribute::get_damage_color() const {
	return damage_color;
}

void ZAttribute::set_round_to(int p_round_to) {
	if (round_to == p_round_to) {
		return;
	}
	round_to = p_round_to;
	emit_changed();
}

int ZAttribute::get_round_to() const {
	return round_to;
}

float ZAttribute::get_max_value_at_rank(int p_rank) const {
	if (rank_value_curve.is_null()) {
		return round_value(max_value);
	}
	return round_value(max_value * rank_value_curve->sample((float)p_rank));
}

float ZAttribute::get_start_value_at_rank(int p_rank) const {
	const float max_at_rank = get_max_value_at_rank(p_rank);
	if (is_static) {
		return max_at_rank;
	}
	return round_value(max_at_rank * start_value_percent);
}

float ZAttribute::round_value(float p_value) const {
	const double step = UtilityFunctions::pow(10.0, -(double)round_to);
	if (step <= 0.0) {
		return p_value;
	}
	return (float)UtilityFunctions::snappedf(p_value, step);
}
