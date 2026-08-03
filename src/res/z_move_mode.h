#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Tunable locomotion parameters used by ZMover.
class ZMoveMode : public Resource {
	GDCLASS(ZMoveMode, Resource)

private:
	float move_speed = 5.0f;
	float move_accel = 20.0f;
	float move_decel = 25.0f;
	float gravity_scale = 1.0f;
	Vector3 gravity_dir = Vector3(0.0f, -1.0f, 0.0f);
	// When true, ZMover rotates the character body toward movement.
	bool rotate_with_movement = false;
	// Max angular speed (rad/s) per Euler axis while rotating to face movement.
	// 3D ground characters primarily use Y (yaw); 2D uses Z.
	Vector3 velocity_rotation_rate = Vector3(0.0f, 10.0f, 10.0f);
	// true: while rotating, horizontal velocity stays along current body forward (tank-like).
	// false: horizontal velocity lerps directly toward the wish / input direction.
	bool forward_adjust_rotation = false;

protected:
	static void _bind_methods();

public:
	void set_move_speed(float p_speed);
	float get_move_speed() const;

	void set_move_accel(float p_accel);
	float get_move_accel() const;

	void set_move_decel(float p_decel);
	float get_move_decel() const;

	void set_gravity_scale(float p_scale);
	float get_gravity_scale() const;

	void set_gravity_dir(const Vector3 &p_dir);
	Vector3 get_gravity_dir() const;

	// Normalized gravity direction (falls back to -Y if zero-length).
	Vector3 get_gravity_dir_normalized() const;

	void set_rotate_with_movement(bool p_enabled);
	bool get_rotate_with_movement() const;

	void set_velocity_rotation_rate(const Vector3 &p_rate);
	Vector3 get_velocity_rotation_rate() const;

	void set_forward_adjust_rotation(bool p_enabled);
	bool get_forward_adjust_rotation() const;

	ZMoveMode();
	~ZMoveMode();
};

} // namespace godot
