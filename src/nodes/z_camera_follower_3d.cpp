#include "z_camera_follower_3d.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

void ZCameraFollower3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target", "target"), &ZCameraFollower3D::set_target);
	ClassDB::bind_method(D_METHOD("get_target"), &ZCameraFollower3D::get_target);

	ClassDB::bind_method(D_METHOD("set_target_lerp", "lerp"), &ZCameraFollower3D::set_target_lerp);
	ClassDB::bind_method(D_METHOD("get_target_lerp"), &ZCameraFollower3D::get_target_lerp);

	ClassDB::bind_method(D_METHOD("set_look_speed", "speed"), &ZCameraFollower3D::set_look_speed);
	ClassDB::bind_method(D_METHOD("get_look_speed"), &ZCameraFollower3D::get_look_speed);

	ClassDB::bind_method(D_METHOD("set_snap_when_current", "enabled"), &ZCameraFollower3D::set_snap_when_current);
	ClassDB::bind_method(D_METHOD("get_snap_when_current"), &ZCameraFollower3D::get_snap_when_current);

	ClassDB::bind_method(D_METHOD("capture_start_rotation"), &ZCameraFollower3D::capture_start_rotation);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target", PROPERTY_HINT_NODE_TYPE, "Node3D"), "set_target", "get_target");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "target_lerp", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_target_lerp", "get_target_lerp");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "look_speed", PROPERTY_HINT_RANGE, "0,50,0.01,or_greater"), "set_look_speed", "get_look_speed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_when_current"), "set_snap_when_current", "get_snap_when_current");
}

ZCameraFollower3D::ZCameraFollower3D() {
}

ZCameraFollower3D::~ZCameraFollower3D() {
}

void ZCameraFollower3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_process(true);
			was_current = false;
			has_start_basis = false;
		} break;
		case NOTIFICATION_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}

			const bool now_current = is_current();
			if (now_current && !was_current) {
				_on_became_current();
			}
			was_current = now_current;

			if (!now_current) {
				break;
			}

			_apply_look(get_process_delta_time(), false);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			was_current = false;
			has_start_basis = false;
		} break;
		default:
			break;
	}
}

Node3D *ZCameraFollower3D::_get_target() const {
	if (target_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<Node3D>(ObjectDB::get_instance(target_id));
}

void ZCameraFollower3D::_on_became_current() {
	capture_start_rotation();
	if (snap_when_current) {
		_apply_look(0.0, true);
	}
}

void ZCameraFollower3D::capture_start_rotation() {
	start_basis = get_global_transform().basis.orthonormalized();
	has_start_basis = true;
}

void ZCameraFollower3D::_apply_look(double p_dt, bool p_snap) {
	if (!has_start_basis) {
		capture_start_rotation();
	}

	const Transform3D gt = get_global_transform();
	const Vector3 origin = gt.origin;
	const Quaternion start_q(start_basis);

	// With a valid target: desired = slerp(start, look_at, target_lerp).
	// With null/invalid target: ease back to the default (start) rotation.
	Quaternion desired_q = start_q;
	Node3D *tgt = _get_target();
	if (tgt != nullptr && tgt->is_inside_tree()) {
		const Vector3 target_pos = tgt->get_global_position();
		const Vector3 to_target = target_pos - origin;
		if (to_target.length_squared() >= 1e-10f) {
			// Desired full look-at (Godot forward is -Z).
			const Transform3D look_xf = gt.looking_at(target_pos, Vector3(0, 1, 0));
			const Quaternion look_q(look_xf.basis.orthonormalized());
			const float blend = Math::clamp(target_lerp, 0.0f, 1.0f);
			desired_q = start_q.slerp(look_q, blend);
		}
	}

	Quaternion next_q;
	if (p_snap || look_speed <= 0.0f) {
		next_q = desired_q;
	} else {
		const Quaternion cur_q(gt.basis.orthonormalized());
		const float t = 1.0f - Math::exp(-look_speed * (float)p_dt);
		next_q = cur_q.slerp(desired_q, t);
	}

	set_global_transform(Transform3D(Basis(next_q), origin));
}

void ZCameraFollower3D::set_target(Node3D *p_target) {
	if (p_target != nullptr) {
		target_id = ObjectID(p_target->get_instance_id());
	} else {
		target_id = ObjectID();
	}
}

Node3D *ZCameraFollower3D::get_target() const {
	return _get_target();
}

void ZCameraFollower3D::set_target_lerp(float p_lerp) {
	target_lerp = Math::clamp(p_lerp, 0.0f, 1.0f);
}

float ZCameraFollower3D::get_target_lerp() const {
	return target_lerp;
}

void ZCameraFollower3D::set_look_speed(float p_speed) {
	look_speed = Math::max(p_speed, 0.0f);
}

float ZCameraFollower3D::get_look_speed() const {
	return look_speed;
}

void ZCameraFollower3D::set_snap_when_current(bool p_enabled) {
	snap_when_current = p_enabled;
}

bool ZCameraFollower3D::get_snap_when_current() const {
	return snap_when_current;
}
