#pragma once

#include "res/z_bgm_track.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Game music front end. Static, like Z_Sound — call `Z_BGM.Play(track)` from
// anywhere; the underlying ZBgmPlayer node is created on first use, parented to
// the root viewport, and survives scene changes.
//
// Current track / variant / lock state is mirrored onto Z.save_game, so a
// loaded save can resume its music via RestoreFromSave().
class Z_BGM : public RefCounted {
	GDCLASS(Z_BGM, RefCounted)

protected:
	static void _bind_methods();

public:
	// Starts p_bgm. Playing the track that is already playing does nothing
	// unless p_force_restart is set. When p_fade_previous is true and something
	// is already playing, the two crossfade over p_fade_duration.
	// p_variant picks an entry from the track's audio_named (empty = audio_default).
	// Returns false when locked, or when the track has no playable audio.
	static bool play(const Ref<ZBgmTrack> &p_bgm, float p_start_time = 0.0f, bool p_fade_previous = true,
			float p_fade_duration = 0.5f, bool p_force_restart = false, const StringName &p_variant = StringName());

	// While locked the music cannot be stopped or changed.
	static void lock(bool p_locked);
	static bool is_locked();

	static bool stop(bool p_fade = true, float p_fade_duration = 1.0f);

	static Ref<ZBgmTrack> get_current_track();
	static StringName get_current_variant();
	static bool is_playing();
	static float get_position();

	// Re-applies the track / variant / lock stored on Z.save_game.
	static bool restore_from_save();
};

} // namespace godot
