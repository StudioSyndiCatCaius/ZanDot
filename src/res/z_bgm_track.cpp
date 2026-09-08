#include "z_bgm_track.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {

// Inspector hint for Dictionary[StringName, ZBgmAudio].
String named_audio_hint() {
	return String::num_int64(Variant::STRING_NAME) + "/" + String::num_int64(PROPERTY_HINT_NONE) + ":;" +
			String::num_int64(Variant::OBJECT) + "/" + String::num_int64(PROPERTY_HINT_RESOURCE_TYPE) + ":ZBgmAudio";
}

Dictionary as_named_audio_dict(const Dictionary &p_from) {
	return Dictionary(p_from, Variant::STRING_NAME, StringName(), Variant(), Variant::OBJECT, StringName("ZBgmAudio"), Variant());
}

} // namespace

void ZBgmTrack::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_audio_default", "audio"), &ZBgmTrack::set_audio_default);
	ClassDB::bind_method(D_METHOD("get_audio_default"), &ZBgmTrack::get_audio_default);

	ClassDB::bind_method(D_METHOD("set_audio_named", "audio"), &ZBgmTrack::set_audio_named);
	ClassDB::bind_method(D_METHOD("get_audio_named"), &ZBgmTrack::get_audio_named);

	ClassDB::bind_method(D_METHOD("get_audio", "name"), &ZBgmTrack::get_audio, DEFVAL(StringName()));
	ClassDB::bind_method(D_METHOD("has_audio", "name"), &ZBgmTrack::has_audio);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "audio_default", PROPERTY_HINT_RESOURCE_TYPE, "ZBgmAudio"), "set_audio_default", "get_audio_default");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "audio_named", PROPERTY_HINT_DICTIONARY_TYPE, named_audio_hint()), "set_audio_named", "get_audio_named");
}

ZBgmTrack::ZBgmTrack() {
	audio_named = as_named_audio_dict(Dictionary());
}

ZBgmTrack::~ZBgmTrack() {
}

void ZBgmTrack::set_audio_default(const Ref<ZBgmAudio> &p_audio) {
	if (audio_default == p_audio) {
		return;
	}
	audio_default = p_audio;
	emit_changed();
}

Ref<ZBgmAudio> ZBgmTrack::get_audio_default() const {
	return audio_default;
}

void ZBgmTrack::set_audio_named(const Dictionary &p_audio) {
	audio_named = as_named_audio_dict(p_audio);
	emit_changed();
}

Dictionary ZBgmTrack::get_audio_named() const {
	return audio_named;
}

Ref<ZBgmAudio> ZBgmTrack::get_audio(const StringName &p_name) const {
	if (!p_name.is_empty() && audio_named.has(p_name)) {
		Ref<ZBgmAudio> named = audio_named[p_name];
		if (named.is_valid()) {
			return named;
		}
	}
	return audio_default;
}

bool ZBgmTrack::has_audio(const StringName &p_name) const {
	if (p_name.is_empty()) {
		return audio_default.is_valid();
	}
	return audio_named.has(p_name) && Ref<ZBgmAudio>(audio_named[p_name]).is_valid();
}
