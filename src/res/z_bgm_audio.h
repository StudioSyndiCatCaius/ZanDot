#pragma once

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/resource.hpp>

namespace godot {

// One playable music stream plus its loop behaviour. Held by a ZBgmTrack and
// played through Z_BGM.
//
// Loop points are driven by ZanDot rather than the AudioStream's own loop flag,
// so the same stream can be reused with different intros / loop regions.
class ZBgmAudio : public Resource {
	GDCLASS(ZBgmAudio, Resource)

private:
	Ref<AudioStream> sound_source;
	bool looping = true;
	// Where playback jumps back to when loop_end is reached (skipped on first play).
	float loop_start = 0.0f;
	// Seconds into the stream where the loop wraps. 0 (or <= loop_start) means
	// "wrap at the end of the stream" instead.
	float loop_end = 0.0f;
	// Crossfade length used when wrapping. 0 = hard seek, no blend.
	float loop_blend_time = 0.0f;

protected:
	static void _bind_methods();

public:
	void set_sound_source(const Ref<AudioStream> &p_stream);
	Ref<AudioStream> get_sound_source() const;

	void set_looping(bool p_looping);
	bool get_looping() const;

	void set_loop_start(float p_seconds);
	float get_loop_start() const;

	void set_loop_end(float p_seconds);
	float get_loop_end() const;

	void set_loop_blend_time(float p_seconds);
	float get_loop_blend_time() const;

	// True when loop_end marks a usable wrap point inside the stream.
	bool has_loop_region() const;

	ZBgmAudio();
	~ZBgmAudio();
};

} // namespace godot
