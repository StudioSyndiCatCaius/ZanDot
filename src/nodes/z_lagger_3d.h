#pragma once

#include <godot_cpp/classes/node3d.hpp>

namespace godot {

// Optionally lags direct child Node3Ds behind this node's own transform
// (position and/or rotation), like a spring arm. Children are made top-level
// so their global transform is driven here instead of inherited instantly.
class ZLagger3D : public Node3D {
	GDCLASS(ZLagger3D, Node3D)

private:
	bool lag_position = true;
	bool lag_rotation = true;
	// Catch-up rate (1/seconds); higher snaps faster, lower lags more.
	float position_speed = 10.0f;
	float rotation_speed = 10.0f;

	void _prepare_children();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_lag_position(bool p_enabled);
	bool get_lag_position() const;

	void set_lag_rotation(bool p_enabled);
	bool get_lag_rotation() const;

	void set_position_speed(float p_speed);
	float get_position_speed() const;

	void set_rotation_speed(float p_speed);
	float get_rotation_speed() const;

	ZLagger3D();
	~ZLagger3D();
};

} // namespace godot
