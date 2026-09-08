#include "z_bgm_audio.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZBgmAudio::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_sound_source", "stream"), &ZBgmAudio::set_sound_source);
	ClassDB::bind_method(D_METHOD("get_sound_source"), &ZBgmAudio::get_sound_source);

	ClassDB::bind_method(D_METHOD("set_looping", "looping"), &ZBgmAudio::set_looping);
	ClassDB::bind_method(D_METHOD("get_looping"), &ZBgmAudio::get_looping);

	ClassDB::bind_method(D_METHOD("set_loop_start", "seconds"), &ZBgmAudio::set_loop_start);
	ClassDB::bind_method(D_METHOD("get_loop_start"), &ZBgmAudio::get_loop_start);

	ClassDB::bind_method(D_METHOD("set_loop_end", "seconds"), &ZBgmAudio::set_loop_end);
	ClassDB::bind_method(D_METHOD("get_loop_end"), &ZBgmAudio::get_loop_end);

	ClassDB::bind_method(D_METHOD("set_loop_blend_time", "seconds"), &ZBgmAudio::set_loop_blend_time);
	ClassDB::bind_method(D_METHOD("get_loop_blend_time"), &ZBgmAudio::get_loop_blend_time);

	ClassDB::bind_method(D_METHOD("has_loop_region"), &ZBgmAudio::has_loop_region);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sound_source", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_sound_source", "get_sound_source");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "looping"), "set_looping", "get_looping");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_start", PROPERTY_HINT_RANGE, "0,600,0.001,or_greater,suffix:s"), "set_loop_start", "get_loop_start");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_end", PROPERTY_HINT_RANGE, "0,600,0.001,or_greater,suffix:s"), "set_loop_end", "get_loop_end");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_blend_time", PROPERTY_HINT_RANGE, "0,10,0.01,or_greater,suffix:s"), "set_loop_blend_time", "get_loop_blend_time");
}

ZBgmAudio::ZBgmAudio() {
}

ZBgmAudio::~ZBgmAudio() {
}

void ZBgmAudio::set_sound_source(const Ref<AudioStream> &p_stream) {
	if (sound_source == p_stream) {
		return;
	}
	sound_source = p_stream;
	emit_changed();
}

Ref<AudioStream> ZBgmAudio::get_sound_source() const {
	return sound_source;
}

void ZBgmAudio::set_looping(bool p_looping) {
	if (looping == p_looping) {
		return;
	}
	looping = p_looping;
	emit_changed();
}

bool ZBgmAudio::get_looping() const {
	return looping;
}

void ZBgmAudio::set_loop_start(float p_seconds) {
	const float v = p_seconds < 0.0f ? 0.0f : p_seconds;
	if (loop_start == v) {
		return;
	}
	loop_start = v;
	emit_changed();
}

float ZBgmAudio::get_loop_start() const {
	return loop_start;
}

void ZBgmAudio::set_loop_end(float p_seconds) {
	const float v = p_seconds < 0.0f ? 0.0f : p_seconds;
	if (loop_end == v) {
		return;
	}
	loop_end = v;
	emit_changed();
}

float ZBgmAudio::get_loop_end() const {
	return loop_end;
}

void ZBgmAudio::set_loop_blend_time(float p_seconds) {
	const float v = p_seconds < 0.0f ? 0.0f : p_seconds;
	if (loop_blend_time == v) {
		return;
	}
	loop_blend_time = v;
	emit_changed();
}

float ZBgmAudio::get_loop_blend_time() const {
	return loop_blend_time;
}

bool ZBgmAudio::has_loop_region() const {
	return looping && loop_end > loop_start;
}
