#pragma once

#include "z_save.h"

namespace godot {

// Global/meta save: created on first launch, reloaded every boot, saved on quit.
class ZSaveGlobal : public ZSave {
	GDCLASS(ZSaveGlobal, ZSave)

protected:
	static void _bind_methods();

public:
	ZSaveGlobal();
	~ZSaveGlobal();
};

} // namespace godot
