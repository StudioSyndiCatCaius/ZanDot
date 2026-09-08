#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

// Scriptable condition resource. Override check() / _check() in GDScript.
class ZGlobalCondition : public Resource {
	GDCLASS(ZGlobalCondition, Resource)

protected:
	static void _bind_methods();

	virtual bool check();

	GDVIRTUAL0R(bool, _check)

public:
	// Dispatches to script _check / virtual check().
	bool check_bind();

	ZGlobalCondition();
	~ZGlobalCondition();
};

} // namespace godot
