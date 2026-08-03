#include "z_mover.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
	ClassDB::bind_method(D_METHOD("move_by_look", "input", "look"), &ZMover::move_by_look);
	ClassDB::bind_method(D_METHOD("launch", "dir", "scale", "override_side_velocity", "override_up_velocity"), &ZMover::launch, DEFVAL(true), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("Move_ByInput", "input"), &ZMover::Move_ByInput);
	ClassDB::bind_method(D_METHOD("Move_ByLook", "input", "look"), &ZMover::Move_ByLook);
	ClassDB::bind_method(D_METHOD("Launch", "dir", "scale", "override_side_velocity", "override_up_velocity"), &ZMover::Launch, DEFVAL(true), DEFVAL(true));

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
			// Require Move_By* each frame while holding input; otherwise decelerate to stop.
			if (!wish_pending) {
				wish_dir = Vector3();
			}
			wish_pending = false;

			const bool rotate_move = move_mode->get_rotate_with_movement();
			const bool tank = move_mode->get_forward_adjust_rotation();

			// Tank-like: rotate body toward wish first, then integrate along facing.
			// Strafe: integrate toward wish first, then face velocity.
			if (rotate_move && tank) {
				Vector3 wish = _project_onto_plane(wish_dir, up);
				if (wish.length_squared() > 1e-12f) {
					_rotate_body_toward(wish.normalized(), up, dt);
				}
			}

			_integrate_horizontal(dt, up);

			if (rotate_move && !tank) {
				Vector3 horizontal;
				(void)_split_vertical(velocity, up, horizontal);
				if (horizontal.length_squared() > 1e-12f) {
					_rotate_body_toward(horizontal.normalized(), up, dt);
				} else {
					Vector3 wish = _project_onto_plane(wish_dir, up);
					if (wish.length_squared() > 1e-12f) {
						_rotate_body_toward(wish.normalized(), up, dt);
					}
				}
			}

			if (apply_gravity) {
				_integrate_gravity(dt, up);
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

float ZMover::_project_gravity_strength() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	float g = 9.8f;
	if (body_3d != nullptr && ps != nullptr && ps->has_setting("physics/3d/default_gravity")) {
		g = ps->get_setting("physics/3d/default_gravity");
	} else if (body_2d != nullptr && ps != nullptr && ps->has_setting("physics/2d/default_gravity")) {
		g = ps->get_setting("physics/2d/default_gravity");
	} else if (ps != nullptr && ps->has_setting("physics/3d/default_gravity")) {
		g = ps->get_setting("physics/3d/default_gravity");
	}
	return g * move_mode->get_gravity_scale();
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

void ZMover::_integrate_horizontal(double p_delta, const Vector3 &p_up) {
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
	const float rate = has_input ? move_mode->get_move_accel() : move_mode->get_move_decel();

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

void ZMover::_integrate_gravity(double p_delta, const Vector3 &p_up) {
	// Skip gravity while grounded and not trying to leave the floor (keeps is_on_floor stable).
	if (is_on_floor() && velocity.dot(p_up) <= 0.0f) {
		// Kill residual downward speed into the floor.
		Vector3 horizontal;
		(void)_split_vertical(velocity, p_up, horizontal);
		velocity = horizontal;
		return;
	}

	const float g = _project_gravity_strength();
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

Basis ZMover::_look_to_basis(const Variant &p_look) const {
	switch (p_look.get_type()) {
		case Variant::BASIS:
			return p_look;
		case Variant::QUATERNION:
			return Basis(p_look.operator Quaternion());
		case Variant::TRANSFORM3D:
			return p_look.operator Transform3D().basis;
		case Variant::VECTOR3: {
			const Vector3 forward = p_look.operator Vector3();
			if (forward.length_squared() < 1e-12f) {
				return Basis();
			}
			// Build a look basis: -Z forward (Godot camera convention), Y up.
			return Basis::looking_at(forward.normalized(), Vector3(0, 1, 0));
		}
		case Variant::FLOAT:
		case Variant::INT: {
			// 2D angle in radians (rotation around Z).
			const float angle = (float)p_look.operator double();
			return Basis(Vector3(0, 0, 1), angle);
		}
		case Variant::TRANSFORM2D: {
			const Transform2D t = p_look;
			// Map 2D X/Y axes into 3D X/Y.
			const Vector2 x = t.columns[0];
			const Vector2 y = t.columns[1];
			return Basis(Vector3(x.x, x.y, 0), Vector3(y.x, y.y, 0), Vector3(0, 0, 1));
		}
		case Variant::OBJECT: {
			Object *obj = p_look;
			if (obj == nullptr) {
				return Basis();
			}
			Node3D *n3 = Object::cast_to<Node3D>(obj);
			if (n3 != nullptr) {
				return n3->get_global_transform().basis;
			}
			Node2D *n2 = Object::cast_to<Node2D>(obj);
			if (n2 != nullptr) {
				const float angle = n2->get_global_rotation();
				return Basis(Vector3(0, 0, 1), angle);
			}
			return Basis();
		}
		default:
			return Basis();
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

void ZMover::move_by_look(const Vector2 &p_input, const Variant &p_look) {
	Vector2 in = p_input;
	if (in.length_squared() > 1.0f) {
		in = in.normalized();
	}
	wish_pending = true;
	if (in.length_squared() < 1e-12f) {
		wish_dir = Vector3();
		return;
	}

	const Basis basis = _look_to_basis(p_look);

	if (body_2d != nullptr && body_3d == nullptr) {
		// 2D: use basis X/Y as local right/forward on the plane.
		const Vector3 right = Vector3(basis.get_column(0).x, basis.get_column(0).y, 0.0f);
		const Vector3 forward = Vector3(basis.get_column(1).x, basis.get_column(1).y, 0.0f);
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
	Vector3 right = _project_onto_plane(basis.get_column(0), up);
	Vector3 forward = _project_onto_plane(-basis.get_column(2), up);
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

void ZMover::launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity, bool p_override_up_velocity) {
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
}

void ZMover::Move_ByInput(const Vector2 &p_input) {
	move_by_input(p_input);
}

void ZMover::Move_ByLook(const Vector2 &p_input, const Variant &p_look) {
	move_by_look(p_input, p_look);
}

void ZMover::Launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity, bool p_override_up_velocity) {
	launch(p_dir, p_scale, p_override_side_velocity, p_override_up_velocity);
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
