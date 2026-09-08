#pragma once

#include "z_bgm_audio.h"
#include "z_resource.h"

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// A piece of game music, as passed to Z_BGM.Play().
//
// `audio_default` is what plays normally; `audio_named` holds alternate
// arrangements of the same track (e.g. &"combat", &"muffled") selected via the
// `variant` argument on Play().
class ZBgmTrack : public ZResource {
	GDCLASS(ZBgmTrack, ZResource)

private:
	Ref<ZBgmAudio> audio_default;
	Dictionary audio_named; // Dictionary[StringName, ZBgmAudio]

protected:
	static void _bind_methods();

public:
	void set_audio_default(const Ref<ZBgmAudio> &p_audio);
	Ref<ZBgmAudio> get_audio_default() const;

	void set_audio_named(const Dictionary &p_audio);
	Dictionary get_audio_named() const;

	// Named entry when p_name is set and present, otherwise audio_default.
	Ref<ZBgmAudio> get_audio(const StringName &p_name = StringName()) const;
	bool has_audio(const StringName &p_name) const;

	ZBgmTrack();
	~ZBgmTrack();
};

} // namespace godot
