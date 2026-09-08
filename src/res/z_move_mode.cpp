#include "z_move_mode.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMoveMode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_move_speed", "speed"), &ZMoveMode::set_move_speed);
	ClassDB::bind_method(D_METHOD("get_move_speed"), &ZMoveMode::get_move_speed);

	ClassDB::bind_method(D_METHOD("set_move_accel", "accel"), &ZMoveMode::set_move_accel);
	ClassDB::bind_method(D_METHOD("get_move_accel"), &ZMoveMode::get_move_accel);

	ClassDB::bind_method(D_METHOD("set_move_decel", "decel"), &ZMoveMode::set_move_decel);
	ClassDB::bind_method(D_METHOD("get_move_decel"), &ZMoveMode::get_move_decel);

	ClassDB::bind_method(D_METHOD("set_air_control", "control"), &ZMoveMode::set_air_control);
	ClassDB::bind_method(D_METHOD("get_air_control"), &ZMoveMode::get_air_control);

	ClassDB::bind_method(D_METHOD("set_air_friction", "friction"), &ZMoveMode::set_air_friction);
	ClassDB::bind_method(D_METHOD("get_air_friction"), &ZMoveMode::get_air_friction);

	ClassDB::bind_method(D_METHOD("set_gravity_scale", "scale"), &ZMoveMode::set_gravity_scale);
	ClassDB::bind_method(D_METHOD("get_gravity_scale"), &ZMoveMode::get_gravity_scale);

	ClassDB::bind_method(D_METHOD("set_gravity_dir", "dir"), &ZMoveMode::set_gravity_dir);
	ClassDB::bind_method(D_METHOD("get_gravity_dir"), &ZMoveMode::get_gravity_dir);

	ClassDB::bind_method(D_METHOD("get_gravity_dir_normalized"), &ZMoveMode::get_gravity_dir_normalized);

	ClassDB::bind_method(D_METHOD("set_gravity_accel_curve", "curve"), &ZMoveMode::set_gravity_accel_curve);
	ClassDB::bind_method(D_METHOD("get_gravity_accel_curve"), &ZMoveMode::get_gravity_accel_curve);

	ClassDB::bind_method(D_METHOD("set_rotate_with_movement", "enabled"), &ZMoveMode::set_rotate_with_movement);
	ClassDB::bind_method(D_METHOD("get_rotate_with_movement"), &ZMoveMode::get_rotate_with_movement);

	ClassDB::bind_method(D_METHOD("set_velocity_rotation_rate", "rate"), &ZMoveMode::set_velocity_rotation_rate);
	ClassDB::bind_method(D_METHOD("get_velocity_rotation_rate"), &ZMoveMode::get_velocity_rotation_rate);

	ClassDB::bind_method(D_METHOD("set_forward_adjust_rotation", "enabled"), &ZMoveMode::set_forward_adjust_rotation);
	ClassDB::bind_method(D_METHOD("get_forward_adjust_rotation"), &ZMoveMode::get_forward_adjust_rotation);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed", PROPERTY_HINT_RANGE, "0,100,0.01,or_greater"), "set_move_speed", "get_move_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_accel", PROPERTY_HINT_RANGE, "0,500,0.01,or_greater"), "set_move_accel", "get_move_accel");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_decel", PROPERTY_HINT_RANGE, "0,500,0.01,or_greater"), "set_move_decel", "get_move_decel");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_control", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_air_control", "get_air_control");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_friction", PROPERTY_HINT_RANGE, "0,500,0.01,or_greater"), "set_air_friction", "get_air_friction");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity_scale", PROPERTY_HINT_RANGE, "0,10,0.01,or_greater"), "set_gravity_scale", "get_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "gravity_dir"), "set_gravity_dir", "get_gravity_dir");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gravity_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_gravity_accel_curve", "get_gravity_accel_curve");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rotate_with_movement"), "set_rotate_with_movement", "get_rotate_with_movement");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "velocity_rotation_rate"), "set_velocity_rotation_rate", "get_velocity_rotation_rate");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "forward_adjust_rotation"), "set_forward_adjust_rotation", "get_forward_adjust_rotation");
}

ZMoveMode::ZMoveMode() {
}

ZMoveMode::~ZMoveMode() {
}

void ZMoveMode::set_move_speed(float p_speed) {
	if (move_speed == p_speed) {
		return;
	}
	move_speed = p_speed;
	emit_changed();
}

float ZMoveMode::get_move_speed() const {
	return move_speed;
}

void ZMoveMode::set_move_accel(float p_accel) {
	if (move_accel == p_accel) {
		return;
	}
	move_accel = p_accel;
	emit_changed();
}

float ZMoveMode::get_move_accel() const {
	return move_accel;
}

void ZMoveMode::set_move_decel(float p_decel) {
	if (move_decel == p_decel) {
		return;
	}
	move_decel = p_decel;
	emit_changed();
}

float ZMoveMode::get_move_decel() const {
	return move_decel;
}

void ZMoveMode::set_air_control(float p_control) {
	if (air_control == p_control) {
		return;
	}
	air_control = p_control;
	emit_changed();
}

float ZMoveMode::get_air_control() const {
	return air_control;
}

void ZMoveMode::set_air_friction(float p_friction) {
	if (air_friction == p_friction) {
		return;
	}
	air_friction = p_friction;
	emit_changed();
}

float ZMoveMode::get_air_friction() const {
	return air_friction;
}

void ZMoveMode::set_gravity_scale(float p_scale) {
	if (gravity_scale == p_scale) {
		return;
	}
	gravity_scale = p_scale;
	emit_changed();
}

float ZMoveMode::get_gravity_scale() const {
	return gravity_scale;
}

void ZMoveMode::set_gravity_dir(const Vector3 &p_dir) {
	if (gravity_dir == p_dir) {
		return;
	}
	gravity_dir = p_dir;
	emit_changed();
}

Vector3 ZMoveMode::get_gravity_dir() const {
	return gravity_dir;
}

Vector3 ZMoveMode::get_gravity_dir_normalized() const {
	if (gravity_dir.length_squared() < 1e-12f) {
		return Vector3(0.0f, -1.0f, 0.0f);
	}
	return gravity_dir.normalized();
}

void ZMoveMode::set_gravity_accel_curve(const Ref<Curve> &p_curve) {
	if (gravity_accel_curve == p_curve) {
		return;
	}
	gravity_accel_curve = p_curve;
	emit_changed();
}

Ref<Curve> ZMoveMode::get_gravity_accel_curve() const {
	return gravity_accel_curve;
}

float ZMoveMode::sample_gravity_accel_scale(float p_fall_time) const {
	if (gravity_accel_curve.is_null()) {
		return 1.0f;
	}
	return gravity_accel_curve->sample(p_fall_time);
}

void ZMoveMode::set_rotate_with_movement(bool p_enabled) {
	if (rotate_with_movement == p_enabled) {
		return;
	}
	rotate_with_movement = p_enabled;
	emit_changed();
}

bool ZMoveMode::get_rotate_with_movement() const {
	return rotate_with_movement;
}

void ZMoveMode::set_velocity_rotation_rate(const Vector3 &p_rate) {
	if (velocity_rotation_rate == p_rate) {
		return;
	}
	velocity_rotation_rate = p_rate;
	emit_changed();
}

Vector3 ZMoveMode::get_velocity_rotation_rate() const {
	return velocity_rotation_rate;
}

void ZMoveMode::set_forward_adjust_rotation(bool p_enabled) {
	if (forward_adjust_rotation == p_enabled) {
		return;
	}
	forward_adjust_rotation = p_enabled;
	emit_changed();
}

bool ZMoveMode::get_forward_adjust_rotation() const {
	return forward_adjust_rotation;
}
