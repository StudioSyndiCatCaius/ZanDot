#pragma once

#include "res/z_bgm_track.h"

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Runtime music player behind Z_BGM. Internal — game code talks to Z_BGM.
//
// Lives under the root viewport (so it survives scene changes) with
// PROCESS_MODE_ALWAYS (so music keeps playing while the tree is paused).
// A small pool of AudioStreamPlayer "voices" lets a track crossfade into the
// next one, and lets a looping track blend into its own loop point.
class ZBgmPlayer : public Node {
	GDCLASS(ZBgmPlayer, Node)

public:
	// One track fading out, one fading in, plus a loop-blend pair; 4 is plenty.
	static const int MAX_VOICES = 4;

private:
	struct Voice {
		AudioStreamPlayer *player = nullptr;
		Ref<ZBgmAudio> audio;
		float gain = 1.0f;
		float target_gain = 1.0f;
		// Gain change per second; <= 0 snaps straight to target_gain.
		float fade_speed = 0.0f;
		// Free this voice once it reaches silence.
		bool release = false;
		// Loop blend for this pass already handed off to a new voice.
		bool blending = false;
	};

	static ObjectID instance_id;

	Voice voices[MAX_VOICES];
	int active = -1;

	Ref<ZBgmTrack> current_track;
	StringName current_variant;
	bool locked = false;

	int _alloc_voice();
	void _free_voice(int p_index);
	void _release_all(bool p_fade, float p_fade_duration);
	void _apply_gain(int p_index);
	void _start_loop_blend(int p_index);
	void _on_voice_finished(int p_index);
	void _sync_to_save();
	StringName _bus() const;

protected:
	static void _bind_methods();

public:
	// Finds the live instance, creating and parenting it if needed.
	// Null when there is no SceneTree yet (e.g. during extension init).
	static ZBgmPlayer *get_or_create();
	// Live instance or null; never creates one.
	static ZBgmPlayer *get_existing();

	bool play(const Ref<ZBgmTrack> &p_track, const StringName &p_variant, float p_start_time,
			bool p_fade_previous, float p_fade_duration, bool p_force_restart);
	bool stop(bool p_fade, float p_fade_duration);

	void set_locked(bool p_locked);
	bool is_locked() const;

	Ref<ZBgmTrack> get_current_track() const;
	StringName get_current_variant() const;
	bool is_playing() const;
	float get_position();

	// Re-applies track / variant / lock from Z.save_game.
	bool restore_from_save();

	void _ready() override;
	void _process(double p_delta) override;

	ZBgmPlayer();
	~ZBgmPlayer();
};

} // namespace godot
