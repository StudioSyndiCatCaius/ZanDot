#pragma once

#include "res/z_move_mode.h"

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Physical mover component for CharacterBody2D and/or CharacterBody3D.
// Call Move_ByInput / Move_ByLook each frame (or when input changes); physics
// integration + move_and_slide run in _physics_process.
class ZMover : public Node {
	GDCLASS(ZMover, Node)

private:
	CharacterBody2D *body_2d = nullptr;
	CharacterBody3D *body_3d = nullptr;
	Ref<ZMoveMode> move_mode;
	Vector3 velocity;

	// Desired horizontal move direction (unit, or zero). Set by Move_By* APIs.
	Vector3 wish_dir;
	// True if Move_By* was called since the last physics tick (otherwise wish is treated as zero).
	bool wish_pending = false;
	bool apply_gravity = true;
	bool auto_find_body = true;

	void _ensure_move_mode();
	void _resolve_bodies_if_needed();
	float _project_gravity_strength() const;
	Vector3 _project_onto_plane(const Vector3 &p_vec, const Vector3 &p_normal) const;
	Vector3 _split_vertical(const Vector3 &p_vel, const Vector3 &p_up, Vector3 &r_horizontal) const;
	// Body forward projected onto the movement plane (unit or zero).
	Vector3 _body_forward_on_plane(const Vector3 &p_up) const;
	// Rotate body toward p_target_dir (on movement plane). Rate from ZMoveMode.
	void _rotate_body_toward(const Vector3 &p_target_dir, const Vector3 &p_up, double p_delta);
	void _integrate_horizontal(double p_delta, const Vector3 &p_up);
	void _integrate_gravity(double p_delta, const Vector3 &p_up);
	void _apply_to_bodies(double p_delta);
	Basis _look_to_basis(const Variant &p_look) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_body_2d(CharacterBody2D *p_body);
	CharacterBody2D *get_body_2d() const;

	void set_body_3d(CharacterBody3D *p_body);
	CharacterBody3D *get_body_3d() const;

	void set_move_mode(const Ref<ZMoveMode> &p_mode);
	Ref<ZMoveMode> get_move_mode() const;

	void set_velocity(const Vector3 &p_velocity);
	Vector3 get_velocity() const;

	void set_apply_gravity(bool p_enabled);
	bool get_apply_gravity() const;

	void set_auto_find_body(bool p_enabled);
	bool get_auto_find_body() const;

	// World-aligned move. input.x = +X, input.y = +Z (3D) / +Y (2D planar use).
	// 3D uses XZ; 2D uses XY. Magnitude > 1 is clamped.
	void move_by_input(const Vector2 &p_input);
	// Camera/look-relative move. p_look accepts Basis, Quaternion, Transform3D,
	// Vector3 (forward), float (2D radians), or Node2D/Node3D (uses global basis/rotation).
	void move_by_look(const Vector2 &p_input, const Variant &p_look);
	// Impulse-style launch. dir is direction (normalized internally); scale is speed.
	// override_side/up replace those components of velocity; otherwise they are added.
	void launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity = true, bool p_override_up_velocity = true);

	// Spec-style names.
	void Move_ByInput(const Vector2 &p_input);
	void Move_ByLook(const Vector2 &p_input, const Variant &p_look);
	void Launch(const Vector3 &p_dir, float p_scale, bool p_override_side_velocity = true, bool p_override_up_velocity = true);

	// True if standing on floor (3D or 2D body).
	bool is_on_floor() const;

	ZMover();
	~ZMover();
};

} // namespace godot
