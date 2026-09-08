#include "z_bgm_player.h"

#include "z.h"
#include "z_settings.h"

#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ObjectID ZBgmPlayer::instance_id = ObjectID();

namespace {
// Below this the voice is treated as silent (and -80 dB is Godot's floor).
const float SILENCE_GAIN = 0.001f;
} // namespace

// Internal node with no scripting surface of its own — Z_BGM is the API.
void ZBgmPlayer::_bind_methods() {
}

ZBgmPlayer::ZBgmPlayer() {
}

ZBgmPlayer::~ZBgmPlayer() {
	if ((uint64_t)instance_id == get_instance_id()) {
		instance_id = ObjectID();
	}
}

ZBgmPlayer *ZBgmPlayer::get_existing() {
	if (instance_id.is_null()) {
		return nullptr;
	}
	ZBgmPlayer *player = Object::cast_to<ZBgmPlayer>(ObjectDB::get_instance(instance_id));
	if (player == nullptr) {
		instance_id = ObjectID();
	}
	return player;
}

ZBgmPlayer *ZBgmPlayer::get_or_create() {
	ZBgmPlayer *existing = get_existing();
	if (existing != nullptr) {
		return existing;
	}

	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr || tree->get_root() == nullptr) {
		UtilityFunctions::push_warning("Z_BGM: no active SceneTree yet.");
		return nullptr;
	}

	ZBgmPlayer *player = memnew(ZBgmPlayer);
	player->set_name("ZBgmPlayer");
	// Music should not stop when the game pauses.
	player->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	// Parented to the root viewport, so scene changes leave it running.
	tree->get_root()->add_child(player);
	instance_id = player->get_instance_id();
	return player;
}

void ZBgmPlayer::_ready() {
	set_process(true);
}

StringName ZBgmPlayer::_bus() const {
	const String configured = ZSettings::get(ZSettings::BGM_BUS, String("Master"));
	if (!configured.is_empty() && AudioServer::get_singleton()->get_bus_index(configured) >= 0) {
		return StringName(configured);
	}
	return StringName("Master");
}

int ZBgmPlayer::_alloc_voice() {
	int slot = -1;
	for (int i = 0; i < MAX_VOICES; i++) {
		if (voices[i].player == nullptr) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		// All busy — drop the quietest voice that is not the active one.
		float quietest = 2.0f;
		for (int i = 0; i < MAX_VOICES; i++) {
			if (i == active) {
				continue;
			}
			if (voices[i].gain < quietest) {
				quietest = voices[i].gain;
				slot = i;
			}
		}
		if (slot == -1) {
			slot = 0;
		}
		_free_voice(slot);
	}

	Voice &v = voices[slot];
	v.audio.unref();
	v.gain = 1.0f;
	v.target_gain = 1.0f;
	v.fade_speed = 0.0f;
	v.release = false;
	v.blending = false;

	v.player = memnew(AudioStreamPlayer);
	v.player->set_bus(_bus());
	add_child(v.player);
	v.player->connect("finished", callable_mp(this, &ZBgmPlayer::_on_voice_finished).bind(slot));
	return slot;
}

void ZBgmPlayer::_free_voice(int p_index) {
	Voice &v = voices[p_index];
	if (v.player != nullptr) {
		v.player->stop();
		v.player->queue_free();
		v.player = nullptr;
	}
	v.audio.unref();
	v.gain = 0.0f;
	v.target_gain = 0.0f;
	v.fade_speed = 0.0f;
	v.release = false;
	v.blending = false;
	if (active == p_index) {
		active = -1;
	}
}

void ZBgmPlayer::_release_all(bool p_fade, float p_fade_duration) {
	const bool fade = p_fade && p_fade_duration > 0.0f;
	for (int i = 0; i < MAX_VOICES; i++) {
		if (voices[i].player == nullptr) {
			continue;
		}
		if (!fade) {
			_free_voice(i);
			continue;
		}
		voices[i].target_gain = 0.0f;
		voices[i].fade_speed = 1.0f / p_fade_duration;
		voices[i].release = true;
	}
	active = -1;
}

void ZBgmPlayer::_apply_gain(int p_index) {
	Voice &v = voices[p_index];
	if (v.player == nullptr) {
		return;
	}
	float g = v.gain;
	if (g < 0.0f) {
		g = 0.0f;
	} else if (g > 1.0f) {
		g = 1.0f;
	}
	v.player->set_volume_db(g <= SILENCE_GAIN ? -80.0f : (float)UtilityFunctions::linear_to_db(g));
}

void ZBgmPlayer::_start_loop_blend(int p_index) {
	Voice &old_voice = voices[p_index];
	const Ref<ZBgmAudio> audio = old_voice.audio;
	if (audio.is_null()) {
		return;
	}
	const float blend = audio->get_loop_blend_time();
	if (blend <= 0.0f) {
		return;
	}

	// Fade the tail of this pass out while the fresh one fades in from loop_start.
	old_voice.blending = true;
	old_voice.target_gain = 0.0f;
	old_voice.fade_speed = 1.0f / blend;
	old_voice.release = true;

	// Never steals p_index: _alloc_voice skips the active voice, which is p_index here.
	const int next = _alloc_voice();
	Voice &v = voices[next];
	v.audio = audio;
	v.player->set_stream(audio->get_sound_source());
	v.gain = 0.0f;
	v.target_gain = 1.0f;
	v.fade_speed = 1.0f / blend;
	_apply_gain(next);
	v.player->play(audio->get_loop_start());
	active = next;
}

void ZBgmPlayer::_on_voice_finished(int p_index) {
	if (p_index < 0 || p_index >= MAX_VOICES) {
		return;
	}
	Voice &v = voices[p_index];
	if (v.player == nullptr) {
		return;
	}
	// Looping without an explicit loop region: restart the stream ourselves so
	// the AudioStream's own loop flag does not have to be set.
	if (p_index == active && v.audio.is_valid() && v.audio->get_looping() && !v.audio->has_loop_region()) {
		v.player->play(v.audio->get_loop_start());
		return;
	}
	_free_voice(p_index);
}

void ZBgmPlayer::_process(double p_delta) {
	for (int i = 0; i < MAX_VOICES; i++) {
		Voice &v = voices[i];
		if (v.player == nullptr) {
			continue;
		}

		if (v.fade_speed > 0.0f) {
			const float step = v.fade_speed * (float)p_delta;
			if (v.gain < v.target_gain) {
				v.gain = v.gain + step > v.target_gain ? v.target_gain : v.gain + step;
			} else if (v.gain > v.target_gain) {
				v.gain = v.gain - step < v.target_gain ? v.target_gain : v.gain - step;
			}
		} else {
			v.gain = v.target_gain;
		}
		_apply_gain(i);

		if (v.release && v.gain <= SILENCE_GAIN) {
			_free_voice(i);
			continue;
		}

		if (i != active || v.blending || v.audio.is_null() || !v.player->is_playing()) {
			continue;
		}
		if (!v.audio->has_loop_region()) {
			continue;
		}

		const float loop_start = v.audio->get_loop_start();
		const float loop_end = v.audio->get_loop_end();
		const float blend = v.audio->get_loop_blend_time();
		const float pos = v.player->get_playback_position();

		if (blend > 0.0f) {
			float blend_at = loop_end - blend;
			if (blend_at < loop_start) {
				blend_at = loop_start;
			}
			if (pos >= blend_at) {
				_start_loop_blend(i);
			}
		} else if (pos >= loop_end) {
			v.player->seek(loop_start);
		}
	}
}

bool ZBgmPlayer::play(const Ref<ZBgmTrack> &p_track, const StringName &p_variant, float p_start_time,
		bool p_fade_previous, float p_fade_duration, bool p_force_restart) {
	if (locked) {
		return false;
	}
	if (p_track.is_null()) {
		UtilityFunctions::push_warning("Z_BGM.Play: track is null.");
		return false;
	}

	const Ref<ZBgmAudio> audio = p_track->get_audio(p_variant);
	if (audio.is_null() || audio->get_sound_source().is_null()) {
		UtilityFunctions::push_warning("Z_BGM.Play: track has no playable ZBgmAudio for variant \"" + String(p_variant) + "\".");
		return false;
	}

	// Re-playing the current track is a no-op unless explicitly forced.
	if (!p_force_restart && current_track == p_track && current_variant == p_variant && is_playing()) {
		return false;
	}

	const bool had_previous = is_playing();
	const float fade = (p_fade_previous && p_fade_duration > 0.0f && had_previous) ? p_fade_duration : 0.0f;
	_release_all(fade > 0.0f, fade);

	const int i = _alloc_voice();
	Voice &v = voices[i];
	v.audio = audio;
	v.player->set_stream(audio->get_sound_source());
	// Crossfade: the incoming track rises as the outgoing one falls.
	v.gain = fade > 0.0f ? 0.0f : 1.0f;
	v.target_gain = 1.0f;
	v.fade_speed = fade > 0.0f ? 1.0f / fade : 0.0f;
	_apply_gain(i);
	v.player->play(p_start_time);
	active = i;

	current_track = p_track;
	current_variant = p_variant;
	_sync_to_save();
	return true;
}

bool ZBgmPlayer::stop(bool p_fade, float p_fade_duration) {
	if (locked) {
		return false;
	}
	_release_all(p_fade, p_fade ? p_fade_duration : 0.0f);
	current_track.unref();
	current_variant = StringName();
	_sync_to_save();
	return true;
}

void ZBgmPlayer::set_locked(bool p_locked) {
	if (locked == p_locked) {
		return;
	}
	locked = p_locked;
	_sync_to_save();
}

bool ZBgmPlayer::is_locked() const {
	return locked;
}

Ref<ZBgmTrack> ZBgmPlayer::get_current_track() const {
	return current_track;
}

StringName ZBgmPlayer::get_current_variant() const {
	return current_variant;
}

bool ZBgmPlayer::is_playing() const {
	return active >= 0 && voices[active].player != nullptr && voices[active].player->is_playing();
}

float ZBgmPlayer::get_position() {
	if (active < 0 || voices[active].player == nullptr) {
		return 0.0f;
	}
	return voices[active].player->get_playback_position();
}

void ZBgmPlayer::_sync_to_save() {
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return;
	}
	Ref<ZSaveGame> save = z->get_save_game();
	if (save.is_null()) {
		return;
	}
	save->set_bgm_track(current_track);
	save->set_bgm_variant(current_variant);
	save->set_bgm_locked(locked);
}

bool ZBgmPlayer::restore_from_save() {
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return false;
	}
	Ref<ZSaveGame> save = z->get_save_game();
	if (save.is_null()) {
		UtilityFunctions::push_warning("Z_BGM.RestoreFromSave: no save game loaded.");
		return false;
	}

	// Drop the lock first, otherwise the restore itself would be blocked.
	locked = false;
	const Ref<ZBgmTrack> track = save->get_bgm_track();
	if (track.is_valid()) {
		play(track, save->get_bgm_variant(), 0.0f, true, 0.5f, true);
	} else {
		stop(true, 0.5f);
	}
	set_locked(save->get_bgm_locked());
	return true;
}
