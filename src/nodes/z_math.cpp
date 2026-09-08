#include "z_math.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

namespace {
float _interp_weight(float p_dt, float p_speed, bool p_const) {
	if (p_dt <= 0.0f) {
		return 0.0f;
	}
	if (p_speed <= 0.0f) {
		return 1.0f;
	}
	if (p_const) {
		// Constant-speed: weight is not meaningful alone; callers use distance-based step.
		return -1.0f;
	}
	// Exponential smooth: 1 - e^(-speed * dt)
	return 1.0f - Math::exp(-p_speed * p_dt);
}
} // namespace

void Z_Math::_bind_methods() {
	ClassDB::bind_static_method("Z_Math", D_METHOD("f_interp", "current", "to", "dt", "speed", "const_"), &Z_Math::f_interp, DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_static_method("Z_Math", D_METHOD("F_Interp", "current", "to", "dt", "speed", "const_"), &Z_Math::f_interp, DEFVAL(1.0f), DEFVAL(false));

	ClassDB::bind_static_method("Z_Math", D_METHOD("v2_interp", "current", "to", "dt", "speed", "const_"), &Z_Math::v2_interp, DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_static_method("Z_Math", D_METHOD("V2_Interp", "current", "to", "dt", "speed", "const_"), &Z_Math::v2_interp, DEFVAL(1.0f), DEFVAL(false));

	ClassDB::bind_static_method("Z_Math", D_METHOD("v3_interp", "current", "to", "dt", "speed", "const_"), &Z_Math::v3_interp, DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_static_method("Z_Math", D_METHOD("V3_Interp", "current", "to", "dt", "speed", "const_"), &Z_Math::v3_interp, DEFVAL(1.0f), DEFVAL(false));

	ClassDB::bind_static_method("Z_Math", D_METHOD("t2_interp", "current", "to", "dt", "speed", "const_"), &Z_Math::t2_interp, DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_static_method("Z_Math", D_METHOD("T2_Interp", "current", "to", "dt", "speed", "const_"), &Z_Math::t2_interp, DEFVAL(1.0f), DEFVAL(false));

	ClassDB::bind_static_method("Z_Math", D_METHOD("t3_interp", "current", "to", "dt", "speed", "const_"), &Z_Math::t3_interp, DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_static_method("Z_Math", D_METHOD("T3_Interp", "current", "to", "dt", "speed", "const_"), &Z_Math::t3_interp, DEFVAL(1.0f), DEFVAL(false));
}

float Z_Math::f_interp(float p_current, float p_to, float p_dt, float p_speed, bool p_const) {
	if (p_const) {
		const float diff = p_to - p_current;
		const float max_step = Math::abs(p_speed) * Math::abs(p_dt);
		if (Math::abs(diff) <= max_step) {
			return p_to;
		}
		return p_current + (diff > 0.0f ? max_step : -max_step);
	}
	const float w = _interp_weight(p_dt, p_speed, false);
	return Math::lerp(p_current, p_to, w);
}

Vector2 Z_Math::v2_interp(const Vector2 &p_current, const Vector2 &p_to, float p_dt, float p_speed, bool p_const) {
	if (p_const) {
		const Vector2 diff = p_to - p_current;
		const float dist = diff.length();
		const float max_step = Math::abs(p_speed) * Math::abs(p_dt);
		if (dist <= max_step || dist <= CMP_EPSILON) {
			return p_to;
		}
		return p_current + diff * (max_step / dist);
	}
	const float w = _interp_weight(p_dt, p_speed, false);
	return p_current.lerp(p_to, w);
}

Vector3 Z_Math::v3_interp(const Vector3 &p_current, const Vector3 &p_to, float p_dt, float p_speed, bool p_const) {
	if (p_const) {
		const Vector3 diff = p_to - p_current;
		const float dist = diff.length();
		const float max_step = Math::abs(p_speed) * Math::abs(p_dt);
		if (dist <= max_step || dist <= CMP_EPSILON) {
			return p_to;
		}
		return p_current + diff * (max_step / dist);
	}
	const float w = _interp_weight(p_dt, p_speed, false);
	return p_current.lerp(p_to, w);
}

Transform2D Z_Math::t2_interp(const Transform2D &p_current, const Transform2D &p_to, float p_dt, float p_speed, bool p_const) {
	float w;
	if (p_const) {
		// Approximate constant-speed with a clamped weight from origin distance.
		const float dist = p_current.get_origin().distance_to(p_to.get_origin());
		const float max_step = Math::abs(p_speed) * Math::abs(p_dt);
		if (dist <= CMP_EPSILON) {
			w = Math::min(1.0f, max_step);
		} else {
			w = Math::min(1.0f, max_step / dist);
		}
	} else {
		w = _interp_weight(p_dt, p_speed, false);
	}
	return p_current.interpolate_with(p_to, w);
}

Transform3D Z_Math::t3_interp(const Transform3D &p_current, const Transform3D &p_to, float p_dt, float p_speed, bool p_const) {
	float w;
	if (p_const) {
		const float dist = p_current.origin.distance_to(p_to.origin);
		const float max_step = Math::abs(p_speed) * Math::abs(p_dt);
		if (dist <= CMP_EPSILON) {
			w = Math::min(1.0f, max_step);
		} else {
			w = Math::min(1.0f, max_step / dist);
		}
	} else {
		w = _interp_weight(p_dt, p_speed, false);
	}
	return p_current.interpolate_with(p_to, w);
}
