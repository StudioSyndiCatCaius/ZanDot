#pragma once

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

// Stateless helper for fire-and-forget sound playback.
class Z_Sound : public RefCounted {
	GDCLASS(Z_Sound, RefCounted)

protected:
	static void _bind_methods();

public:
	// Spawns a transient AudioStreamPlayer under the root viewport, plays p_stream
	// once, and frees itself when playback finishes.
	static void oneshot(const Ref<AudioStream> &p_stream, float p_volume_db = 0.0f, float p_pitch_scale = 1.0f);
};

} // namespace godot
