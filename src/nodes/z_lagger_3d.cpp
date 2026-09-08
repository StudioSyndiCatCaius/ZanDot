#include "z_lagger_3d.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>

using namespace godot;

void ZLagger3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_lag_position", "enabled"), &ZLagger3D::set_lag_position);
	ClassDB::bind_method(D_METHOD("get_lag_position"), &ZLagger3D::get_lag_position);

	ClassDB::bind_method(D_METHOD("set_lag_rotation", "enabled"), &ZLagger3D::set_lag_rotation);
	ClassDB::bind_method(D_METHOD("get_lag_rotation"), &ZLagger3D::get_lag_rotation);

	ClassDB::bind_method(D_METHOD("set_position_speed", "speed"), &ZLagger3D::set_position_speed);
	ClassDB::bind_method(D_METHOD("get_position_speed"), &ZLagger3D::get_position_speed);

	ClassDB::bind_method(D_METHOD("set_rotation_speed", "speed"), &ZLagger3D::set_rotation_speed);
	ClassDB::bind_method(D_METHOD("get_rotation_speed"), &ZLagger3D::get_rotation_speed);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lag_position"), "set_lag_position", "get_lag_position");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lag_rotation"), "set_lag_rotation", "get_lag_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "position_speed", PROPERTY_HINT_RANGE, "0.1,50,0.1,or_greater"), "set_position_speed", "get_position_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_speed", PROPERTY_HINT_RANGE, "0.1,50,0.1,or_greater"), "set_rotation_speed", "get_rotation_speed");
}

ZLagger3D::ZLagger3D() {
}

ZLagger3D::~ZLagger3D() {
}

void ZLagger3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process(true);
			_prepare_children();
		} break;
		case NOTIFICATION_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}
			const double dt = get_process_delta_time();
			const Transform3D target = get_global_transform();
			const float pos_t = position_speed > 0.0f ? 1.0f - Math::exp(-position_speed * (float)dt) : 1.0f;
			const float rot_t = rotation_speed > 0.0f ? 1.0f - Math::exp(-rotation_speed * (float)dt) : 1.0f;

			for (int i = 0; i < get_child_count(); i++) {
				Node3D *child = Object::cast_to<Node3D>(get_child(i));
				if (child == nullptr) {
					continue;
				}
				if (!child->is_set_as_top_level()) {
					child->set_as_top_level(true);
					child->set_global_transform(target);
					continue;
				}

				const Transform3D cur = child->get_global_transform();
				const Vector3 new_origin = lag_position ? cur.origin.lerp(target.origin, pos_t) : target.origin;
				const Basis new_basis = lag_rotation ? Basis(Quaternion(cur.basis.orthonormalized()).slerp(Quaternion(target.basis.orthonormalized()), rot_t)) : target.basis;
				child->set_global_transform(Transform3D(new_basis, new_origin));
			}
		} break;
		default:
			break;
	}
}

void ZLagger3D::_prepare_children() {
	const Transform3D target = get_global_transform();
	for (int i = 0; i < get_child_count(); i++) {
		Node3D *child = Object::cast_to<Node3D>(get_child(i));
		if (child == nullptr) {
			continue;
		}
		child->set_as_top_level(true);
		child->set_global_transform(target);
	}
}

void ZLagger3D::set_lag_position(bool p_enabled) {
	lag_position = p_enabled;
}

bool ZLagger3D::get_lag_position() const {
	return lag_position;
}

void ZLagger3D::set_lag_rotation(bool p_enabled) {
	lag_rotation = p_enabled;
}

bool ZLagger3D::get_lag_rotation() const {
	return lag_rotation;
}

void ZLagger3D::set_position_speed(float p_speed) {
	position_speed = p_speed;
}

float ZLagger3D::get_position_speed() const {
	return position_speed;
}

void ZLagger3D::set_rotation_speed(float p_speed) {
	rotation_speed = p_speed;
}

float ZLagger3D::get_rotation_speed() const {
	return rotation_speed;
}
