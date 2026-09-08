#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Stateless interpolation helpers (exponential smooth or constant-speed).
class Z_Math : public RefCounted {
	GDCLASS(Z_Math, RefCounted)

protected:
	static void _bind_methods();

public:
	// const_=false: framerate-independent exponential smooth (speed = response rate).
	// const_=true: move toward target at constant units/sec = speed.
	static float f_interp(float p_current, float p_to, float p_dt, float p_speed = 1.0f, bool p_const = false);
	static Vector2 v2_interp(const Vector2 &p_current, const Vector2 &p_to, float p_dt, float p_speed = 1.0f, bool p_const = false);
	static Vector3 v3_interp(const Vector3 &p_current, const Vector3 &p_to, float p_dt, float p_speed = 1.0f, bool p_const = false);
	static Transform2D t2_interp(const Transform2D &p_current, const Transform2D &p_to, float p_dt, float p_speed = 1.0f, bool p_const = false);
	static Transform3D t3_interp(const Transform3D &p_current, const Transform3D &p_to, float p_dt, float p_speed = 1.0f, bool p_const = false);
};

} // namespace godot
