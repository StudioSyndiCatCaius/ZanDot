#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

// Scriptable action resource. Override run() / _run() in GDScript.
class ZGlobalScript : public Resource {
	GDCLASS(ZGlobalScript, Resource)

protected:
	static void _bind_methods();

	virtual void run();

	GDVIRTUAL0(_run)

public:
	// Dispatches to script _run / virtual run().
	void run_bind();

	ZGlobalScript();
	~ZGlobalScript();
};

} // namespace godot
