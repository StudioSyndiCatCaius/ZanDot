#pragma once

#include "z_bgm_track.h"
#include "z_save.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {

// Main per-playthrough save. Created on new game; loaded/saved by slot or path.
class ZSaveGame : public ZSave {
	GDCLASS(ZSaveGame, ZSave)

private:
	String current_scene;
	Transform3D current_transform;

	// Music state, mirrored by Z_BGM so a loaded save can resume its track.
	Ref<ZBgmTrack> bgm_track;
	StringName bgm_variant;
	bool bgm_locked = false;

protected:
	static void _bind_methods();

public:
	void set_current_scene(const String &p_scene);
	String get_current_scene() const;

	void set_current_transform(const Transform3D &p_transform);
	Transform3D get_current_transform() const;

	void set_bgm_track(const Ref<ZBgmTrack> &p_track);
	Ref<ZBgmTrack> get_bgm_track() const;

	void set_bgm_variant(const StringName &p_variant);
	StringName get_bgm_variant() const;

	void set_bgm_locked(bool p_locked);
	bool get_bgm_locked() const;

	ZSaveGame();
	~ZSaveGame();
};

} // namespace godot
