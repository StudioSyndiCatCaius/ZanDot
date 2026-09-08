#include "z_mover.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>

using namespace godot;

void ZMover::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_body_2d", "body"), &ZMover::set_body_2d);
	ClassDB::bind_method(D_METHOD("get_body_2d"), &ZMover::get_body_2d);

	ClassDB::bind_method(D_METHOD("set_body_3d", "body"), &ZMover::set_body_3d);
	ClassDB::bind_method(D_METHOD("get_body_3d"), &ZMover::get_body_3d);

	ClassDB::bind_method(D_METHOD("set_move_mode", "mode"), &ZMover::set_move_mode);
	ClassDB::bind_method(D_METHOD("get_move_mode"), &ZMover::get_move_mode);

	ClassDB::bind_method(D_METHOD("set_velocity", "velocity"), &ZMover::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &ZMover::get_velocity);

	ClassDB::bind_method(D_METHOD("set_apply_gravity", "enabled"), &ZMover::set_apply_gravity);
	ClassDB::bind_method(D_METHOD("get_apply_gravity"), &ZMover::get_apply_gravity);

	ClassDB::bind_method(D_METHOD("set_auto_find_body", "enabled"), &ZMover::set_auto_find_body);
	ClassDB::bind_method(D_METHOD("get_auto_find_body"), &ZMover::get_auto_find_body);

	ClassDB::bind_method(D_METHOD("move_by_input", "input"), &ZMover::move_by_input);
	ClassDB::bind_method(D_METHOD("move_by_look", "input", "rotation"), &ZMover::move_by_look);
	ClassDB::bind_method(D_METHOD("move_by_look_degrees", "input", "rotation_degrees"), &ZMover::move_by_look_degrees);
	ClassDB::bind_method(D_METHOD("move_by_look_quaternion", "input", "look"), &ZMover::move_by_look_quaternion);
	ClassDB::bind_method(D_METHOD("move_to_position", "position"), &ZMover::move_to_position);
	ClassDB::bind_method(D_METHOD("launch", "dir", "scale", "override_side_velocity", "override_up_velocity", "reset_gravity_curve"), &ZMover::launch, DEFVAL(true), DEFVAL(true), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("Move_ByInput", "input"), &ZMover::Move_ByInput);
	ClassDB::bind_method(D_METHOD("Move_ByLook", "input", "rotation"), &ZMover::Move_ByLook);
	ClassDB::bind_method(D_METHOD("Move_ByLookDegrees", "input", "rotation_degrees"), &ZMover::Move_ByLookDegrees);
	ClassDB::bind_method(D_METHOD("Move_ByLookQuaternion", "input", "look"), &ZMover::Move_ByLookQuaternion);
	ClassDB::bind_method(D_METHOD("Move_ToPosition", "position"), &ZMover::Move_ToPosition);
	ClassDB::bind_method(D_METHOD("Launch", "dir", "scale", "override_side_velocity", "override_up_velocity", "reset_gravity_curve"), &ZMover::Launch, DEFVAL(true), DEFVAL(true), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("is_on_floor"), &ZMover::is_on_floor);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "body_2d", PROPERTY_HINT_NODE_TYPE, "CharacterBody2D"), "set_body_2d", "get_body_2d");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "body_3d", PROPERTY_HINT_NODE_TYPE, "CharacterBody3D"), "set_body_3d", "get_body_3d");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "move_mode", PROPERTY_HINT_RESOURCE_TYPE, "ZMoveMode"), "set_move_mode", "get_move_mode");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "velocity"), "set_velocity", "get_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "apply_gravity"), "set_apply_gravity", "get_apply_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_find_body"), "set_auto_find_body", "get_auto_find_body");
}

ZMover::ZMover() {
	set_physics_process(true);
}

ZMover::~ZMover() {
}

void ZMover::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_ensure_move_mode();
			_resolve_bodies_if_needed();
		} break;
		case NOTIFICATION_READY: {
			_ensure_move_mode();
			_resolve_bodies_if_needed();
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			}
			_ensure_move_mode();
			_resolve_bodies_if_needed();
			const double dt = get_physics_process_delta_time();
			const Vector3 up = -move_mode->get_gravity_dir_normalized();
			const bool grounded = is_on_floor();
			if (grounded) {
				fall_time = 0.0;
			} else {
				fall_time += dt;
			}
			// Require Move_By* each frame while holding input; otherwise decelerate to stop.
			if (!wish_pending) {
				wish_dir = Vector3();
			}
			wish_pending = false;

			// Rotate toward the *input* direction, not current velocity — velocity swings
			// fast at low speed and slow at high speed, which made turn rate look like it
			// scaled with speed. Unreal's OrientRotationToMovement faces acceleration at a
			// flat rad/s; with no input we keep the current facing.
			// Tank mode additionally relies on rotating before integrating, since
			// _integrate_horizontal pushes along the body's facing.
			if (move_mode->get_rotate_with_movement()) {
				Vector3 wish = _project_onto_plane(wish_dir, up);
				if (wish.length_squared() > 1e-12f) {
					_rotate_body_toward(wish.normalized(), up, dt);
				}
			}

			_integrate_horizontal(dt, up, grounded);

			if (apply_gravity) {
				_integrate_gravity(dt, up, grounded);
			}
			_apply_to_bodies(dt);
		} break;
		default:
			break;
	}
}

void ZMover::_ensure_move_mode() {
	if (move_mode.is_null()) {
		move_mode.instantiate();
	}
}

void ZMover::_resolve_bodies_if_needed() {
	if (!auto_find_body) {
		return;
	}
	if (body_2d == nullptr && body_3d == nullptr) {
		Node *p = get_parent();
		if (p != nullptr) {
			body_3d = Object::cast_to<CharacterBody3D>(p);
			if (body_3d == nullptr) {
				body_2d = Object::cast_to<CharacterBody2D>(p);
			}
		}
	}
}

float ZMover::_project_gravity_strength(float p_fall_time) const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	float g = 9.8f;
	if (body_3d != nullptr && ps != nullptr && ps->has_setting("physics/3d/default_gravity")) {
		g = ps->get_setting("physics/3d/default_gravity");
	} else if (body_2d != nullptr && ps != nullptr && ps->has_setting("physics/2d/default_gravity")) {
		g = ps->get_setting("physics/2d/default_gravity");
	} else if (ps != nullptr && ps->has_setting("physics/3d/default_gravity")) {
		g = ps->get_setting("physics/3d/default_gravity");
	}
	return g * move_mode->get_gravity_scale() * move_mode->sample_gravity_accel_scale(p_fall_time);
}

Vector3 ZMover::_project_onto_plane(const Vector3 &p_vec, const Vector3 &p_normal) const {
	return p_vec - p_normal * p_vec.dot(p_normal);
}

Vector3 ZMover::_split_vertical(const Vector3 &p_vel, const Vector3 &p_up, Vector3 &r_horizontal) const {
	const float v = p_vel.dot(p_up);
	r_horizontal = p_vel - p_up * v;
	return p_up * v;
}

Vector3 ZMover::_body_forward_on_plane(const Vector3 &p_up) const {
	if (body_3d != nullptr && body_3d->is_inside_tree()) {
		// Godot: local -Z is forward.
		Vector3 fwd = -body_3d->get_global_transform().basis.get_column(2);
		fwd = _project_onto_plane(fwd, p_up);
		if (fwd.length_squared() > 1e-12f) {
			return fwd.normalized();
		}
		return Vector3();
	}
	if (body_2d != nullptr && body_2d->is_inside_tree()) {
		// 2D: rotation 0 faces +X (basis X column).
		const float angle = body_2d->get_global_rotation();
		Vector3 fwd(Math::cos(angle), Math::sin(angle), 0.0f);
		fwd = _project_onto_plane(fwd, p_up);
		if (fwd.length_squared() > 1e-12f) {
			return fwd.normalized();
		}
		return Vector3();
	}
	return Vector3();
}

void ZMover::_rotate_body_toward(const Vector3 &p_target_dir, const Vector3 &p_up, double p_delta) {
	Vector3 target = _project_onto_plane(p_target_dir, p_up);
	if (target.length_squared() < 1e-12f) {
		return;
	}
	target = target.normalized();

	const Vector3 rates = move_mode->get_velocity_rotation_rate();

	if (body_3d != nullptr && body_3d->is_inside_tree()) {
		// Yaw around gravity up; rate.y is max rad/s (0 = no turn this frame).
		const float max_yaw = Math::abs(rates.y) * (float)p_delta;
		if (max_yaw <= 0.0f) {
			return;
		}

		Vector3 forward = -body_3d->get_global_transform().basis.get_column(2);
		forward = _project_onto_plane(forward, p_up);
		if (forward.length_squared() < 1e-12f) {
			// Degenerate: snap flat basis looking at target.
			const Basis flat = Basis::looking_at(target, p_up);
			Transform3D xf = body_3d->get_global_transform();
			xf.basis = flat;
			body_3d->set_global_transform(xf);
			return;
		}
		forward = forward.normalized();

		const float angle = forward.signed_angle_to(target, p_up);
		const float step = Math::clamp(angle, -max_yaw, max_yaw);
		if (Math::abs(step) < 1e-8f) {
			return;
		}

		// Rotate only around up so pitch/roll from the scene stay intact.
		Transform3D xf = body_3d->get_global_transform();
		xf.basis = xf.basis.rotated(p_up, step);
		body_3d->set_global_transform(xf);
		return;
	}

	if (body_2d != nullptr && body_2d->is_inside_tree()) {
		// 2D faces +X at rotation 0; rate.z is max rad/s.
		const float max_step = Math::abs(rates.z) * (float)p_delta;
		if (max_step <= 0.0f) {
			return;
		}
		const float target_angle = Math::atan2(target.y, target.x);
		const float current = body_2d->get_global_rotation();
		// Shortest angular distance (same construction as Math::lerp_angle).
		float difference = Math::fmod(target_angle - current, (float)Math_TAU);
		float distance = Math::fmod(2.0f * difference, (float)Math_TAU) - difference;
		const float step = Math::clamp(distance, -max_step, max_step);
		body_2d->set_global_rotation(current + step);
	}
}

void ZMover::_integrate_horizontal(double p_delta, const Vector3 &p_up, bool p_grounded) {
	Vector3 horizontal;
	const Vector3 vertical = _split_vertical(velocity, p_up, horizontal);

	Vector3 wish = _project_onto_plane(wish_dir, p_up);
	if (wish.length_squared() > 1e-12f) {
		wish = wish.normalized();
	} else {
		wish = Vector3();
	}

	const float speed = move_mode->get_move_speed();
	const bool has_input = wish.length_squared() > 1e-12f;
	float rate;
	if (has_input) {
		rate = move_mode->get_move_accel();
		if (!p_grounded) {
			rate *= move_mode->get_air_control();
		}
	} else {
		rate = p_grounded ? move_mode->get_move_decel() : move_mode->get_air_friction();
	}

	Vector3 target;
	if (has_input && move_mode->get_forward_adjust_rotation()) {
		// Tank-like: push along current body facing (after rotation step when enabled).
		Vector3 forward = _body_forward_on_plane(p_up);
		if (forward.length_squared() < 1e-12f) {
			// No usable facing yet — fall back to wish so motion can start.
			forward = wish;
		}
		target = forward * speed;
	} else {
		// Strafe / default: velocity lerps directly toward world wish direction.
		target = wish * speed;
	}

	// Move horizontal velocity toward target at accel/decel rate.
	const Vector3 delta_v = target - horizontal;
	const float max_step = rate * (float)p_delta;
	if (delta_v.length_squared() <= max_step * max_step) {
		horizontal = target;
	} else if (max_step > 0.0f) {
		horizontal += delta_v.normalized() * max_step;
	}

	velocity = horizontal + vertical;
}

void ZMover::_integrate_gravity(double p_delta, const Vector3 &p_up, bool p_grounded) {
	// Skip gravity while grounded and not trying to leave the floor (keeps is_on_floor stable).
	if (p_grounded && velocity.dot(p_up) <= 0.0f) {
		// Kill residual downward speed into the floor.
		Vector3 horizontal;
		(void)_split_vertical(velocity, p_up, horizontal);
		velocity = horizontal;
		return;
	}

	const float g = _project_gravity_strength((float)fall_time);
	velocity += (-p_up) * g * (float)p_delta;
}

void ZMover::_apply_to_bodies(double p_delta) {
	(void)p_delta;

	if (body_3d != nullptr && body_3d->is_inside_tree()) {
		body_3d->set_velocity(velocity);
		body_3d->move_and_slide();
		velocity = body_3d->get_velocity();
	}

	if (body_2d != nullptr && body_2d->is_inside_tree()) {
		// 2D uses XY; map ZMover Vector3 (x,y,z) → (x,y).
		body_2d->set_velocity(Vector2(velocity.x, velocity.y));
		body_2d->move_and_slide();
		const Vector2 v2 = body_2d->get_velocity();
		velocity.x = v2.x;
		velocity.y = v2.y;
	}
}

void ZMover::_move_by_look_basis(const Vector2 &p_input, const Basis &p_basis) {
	Vector2 in = p_input;
	if (in.length_squared() > 1.0f) {
		in = in.normalized();
	}
	wish_pending = true;
	if (in.length_squared() < 1e-12f) {
		wish_dir = Vector3();
		return;
	}

	if (body_2d != nullptr && body_3d == nullptr) {
		// 2D: use basis X/Y as local right/forward on the plane.
		const Vector3 right = Vector3(p_basis.get_column(0).x, p_basis.get_column(0).y, 0.0f);
		const Vector3 forward = Vector3(p_basis.get_column(1).x, p_basis.get_column(1).y, 0.0f);
		Vector3 dir = right * in.x + forward * in.y;
		if (dir.length_squared() > 1e-12f) {
			wish_dir = dir.normalized();
		} else {
			wish_dir = Vector3();
		}
		return;
	}

	// 3D: project camera right/forward onto ground plane (perp to gravity).
	_ensure_move_mode();
	const Vector3 up = -move_mode->get_gravity_dir_normalized();
	// Godot camera: -Z is forward, +X is right.
	Vector3 right = _project_onto_plane(p_basis.get_column(0), up);
	Vector3 forward = _project_onto_plane(-p_basis.get_column(2), up);
	if (right.length_squared() > 1e-12f) {
		right = right.normalized();
	} else {
		right = Vector3();
	}
	if (forward.length_squared() > 1e-12f) {
		forward = forward.normalized();
	} else {
		forward = Vector3();
	}

	Vector3 dir = right * in.x + forward * in.y;
	if (dir.length_squared() > 1e-12f) {
		wish_dir = dir.normalized();
	} else {
		wish_dir = Vector3();
	}
}

void ZMover::set_body_2d(CharacterBody2D *p_body) {
	body_2d = p_body;
}

CharacterBody2D *ZMover::get_body_2d() const {
	return body_2d;
}

void ZMover::set_body_3d(CharacterBody3D *p_body) {
	body_3d = p_body;
}

CharacterBody3D *ZMover::get_body_3d() const {
	return body_3d;
}

void ZMover::set_move_mode(const Ref<ZMoveMode> &p_mode) {
	move_mode = p_mode;
}

Ref<ZMoveMode> ZMover::get_move_mode() const {
	return move_mode;
}

void ZMover::set_velocity(const Vector3 &p_velocity) {
	velocity = p_velocity;
}

Vector3 ZMover::get_velocity() const {
	return velocity;
}

void ZMover::set_apply_gravity(bool p_enabled) {
	apply_gravity = p_enabled;
}

bool ZMover::get_apply_gravity() const {
	return apply_gravity;
}

void ZMover::set_auto_find_body(bool p_enabled) {
	auto_find_body = p_enabled;
}

bool ZMover::get_auto_find_body() const {
	return auto_find_body;
}

void ZMover::move_by_input(const Vector2 &p_input) {
	Vector2 in = p_input;
	if (in.length_squared() > 1.0f) {
		in = in.normalized();
	}

	if (body_2d != nullptr && body_3d == nullptr) {
		// 2D: global XY axes.
		wish_dir = Vector3(in.x, in.y, 0.0f);
	} else {
		// 3D (default): global XZ plane (Y up). input.y → +Z.
		wish_dir = Vector3(in.x, 0.0f, in.y);
	}
	wish_pending = true;
}

void ZMover::move_by_look(const Vector2 &p_input, const Vector3 &p_rotation) {
	_move_by_look_basis(p_input, Basis::from_euler(p_rotation));
}

void ZMover::move_by_look_degrees(const Vector2 &p_input, const Vector3 &p_rotation_degrees) {
	const Vector3 rotation(
			Math::deg_to_rad(p_rotation_degrees.x),
			Math::deg_to_rad(p_rotation_degrees.y),
			Math::deg_to_rad(p_rotation_degrees.z));
	_move_by_look_basis(p_input, Basis::from_euler(rotation));
}

void ZMover::move_by_look_quaternion(const Vector2 &p_input, const Quaternion &p_look) {
	_move_by_look_basis(p_input, Basis(p_look));
}

void ZMover::move_to_position(const Vector3 &p_position) {
	_resolve_bodies_if_needed();

	Vector3 body_pos;
	if (body_3d != nullptr && body_3d->is_inside_tree()) {
		body_pos = body_3d->get_global_transform().origin;
	} else if (body_2d != nullptr && body_2d->is_inside_tree()) {
		const Vector2 p2 = body_2d->get_global_position();
		body_pos = Vector3(p2.x, p2.y, 0.0f);
	} else {
		return;
	}

	const Vector3 delta = p_position - body_pos;
	Vector2 input;
	if (body_2d != nullptr && body_3d == nullptr) {
		input = Vector2(delta.x, delta.y);
	} else {
		input = Vector2(delta.x, delta.z);
	}
	if (input.length_squared() > 1e-12f) {
		input = input.normalized();
	} else {
		input = Vector2();
	}
	move_by_input(input);
}

void ZMover::launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity, bool p_override_up_velocity, bool p_reset_gravity_curve) {
	_ensure_move_mode();
	Vector3 dir = p_dir;
	if (dir.length_squared() < 1e-12f) {
		return;
	}
	dir = dir.normalized();
	const Vector3 impulse = dir * p_scale;

	const Vector3 up = -move_mode->get_gravity_dir_normalized();
	Vector3 horiz;
	Vector3 vert = _split_vertical(velocity, up, horiz);
	Vector3 imp_horiz;
	Vector3 imp_vert = _split_vertical(impulse, up, imp_horiz);

	if (p_override_side_velocity) {
		horiz = imp_horiz;
	} else {
		horiz += imp_horiz;
	}
	if (p_override_up_velocity) {
		vert = imp_vert;
	} else {
		vert += imp_vert;
	}
	velocity = horiz + vert;

	if (p_reset_gravity_curve) {
		fall_time = 0.0;
	}
}

void ZMover::Move_ByInput(const Vector2 &p_input) {
	move_by_input(p_input);
}

void ZMover::Move_ByLook(const Vector2 &p_input, const Vector3 &p_rotation) {
	move_by_look(p_input, p_rotation);
}

void ZMover::Move_ByLookDegrees(const Vector2 &p_input, const Vector3 &p_rotation_degrees) {
	move_by_look_degrees(p_input, p_rotation_degrees);
}

void ZMover::Move_ByLookQuaternion(const Vector2 &p_input, const Quaternion &p_look) {
	move_by_look_quaternion(p_input, p_look);
}

void ZMover::Move_ToPosition(const Vector3 &p_position) {
	move_to_position(p_position);
}

void ZMover::Launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity, bool p_override_up_velocity, bool p_reset_gravity_curve) {
	launch(p_dir, p_scale, p_override_side_velocity, p_override_up_velocity, p_reset_gravity_curve);
}

bool ZMover::is_on_floor() const {
	if (body_3d != nullptr && body_3d->is_inside_tree()) {
		return body_3d->is_on_floor();
	}
	if (body_2d != nullptr && body_2d->is_inside_tree()) {
		return body_2d->is_on_floor();
	}
	return false;
}
