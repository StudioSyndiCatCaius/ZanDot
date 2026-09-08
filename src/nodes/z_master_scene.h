#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/signal.hpp>

namespace godot {

// Optional master/root helper scene. Typically a global autoload-style scene
// referenced by Z.master_scene for fades and cross-level services.
class ZMasterScene : public Node {
	GDCLASS(ZMasterScene, Node)

private:
	bool autoset_as_master_scene = true;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	// Script override: return a Signal that emits when the fade finishes.
	// Default returns an invalid Signal (caller skips fade).
	GDVIRTUAL1R(Signal, _hook_play_scene_transit, bool)

public:
	void set_autoset_as_master_scene(bool p_value);
	bool get_autoset_as_master_scene() const;

	// fade_out=true starts fade-to-black (or equivalent); false starts fade-in.
	// Returns a Signal to await, or an invalid Signal to skip.
	Signal hook_play_scene_transit(bool p_fade_out);

	ZMasterScene();
	~ZMasterScene();
};

} // namespace godot
