#pragma once

#include "z_save.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {

// Main per-playthrough save. Created on new game; loaded/saved by slot or path.
class ZSaveGame : public ZSave {
	GDCLASS(ZSaveGame, ZSave)

private:
	String current_scene;
	Transform3D current_transform;

protected:
	static void _bind_methods();

public:
	void set_current_scene(const String &p_scene);
	String get_current_scene() const;

	void set_current_transform(const Transform3D &p_transform);
	Transform3D get_current_transform() const;

	ZSaveGame();
	~ZSaveGame();
};

} // namespace godot
