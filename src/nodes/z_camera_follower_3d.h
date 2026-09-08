#pragma once

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/basis.hpp>

namespace godot {

class Node3D;

// Camera3D that looks toward a target Node3D while it is the current camera.
// Desired rotation is slerp(start, look_at, target_lerp); current rotation
// eases toward that at look_speed. Start is captured when the camera becomes current.
class ZCameraFollower3D : public Camera3D {
	GDCLASS(ZCameraFollower3D, Camera3D)

private:
	ObjectID target_id;
	// 0 = keep starting rotation, 1 = full look-at toward target.
	float target_lerp = 1.0f;
	// Exponential catch-up rate (1/seconds) toward the desired rotation.
	float look_speed = 1.0f;
	// When this camera becomes current, snap immediately to desired rotation.
	bool snap_when_current = true;

	bool was_current = false;
	bool has_start_basis = false;
	Basis start_basis;

	Node3D *_get_target() const;
	void _on_became_current();
	void _apply_look(double p_dt, bool p_snap);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_target(Node3D *p_target);
	Node3D *get_target() const;

	void set_target_lerp(float p_lerp);
	float get_target_lerp() const;

	void set_look_speed(float p_speed);
	float get_look_speed() const;

	void set_snap_when_current(bool p_enabled);
	bool get_snap_when_current() const;

	// Re-capture starting rotation from the current transform (e.g. after a cut).
	void capture_start_rotation();

	ZCameraFollower3D();
	~ZCameraFollower3D();
};

} // namespace godot
