#include "z_bgm.h"

#include "z_bgm_player.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Z_BGM::_bind_methods() {
	// Spec-style names (PascalCase, as elsewhere in ZanDot) …
	ClassDB::bind_static_method("Z_BGM", D_METHOD("Play", "bgm", "start_time", "fade_previous", "fade_duration", "force_restart", "variant"),
			&Z_BGM::play, DEFVAL(0.0f), DEFVAL(true), DEFVAL(0.5f), DEFVAL(false), DEFVAL(StringName()));
	ClassDB::bind_static_method("Z_BGM", D_METHOD("Lock", "locked"), &Z_BGM::lock);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("Stop", "fade", "fade_duration"), &Z_BGM::stop, DEFVAL(true), DEFVAL(1.0f));
	ClassDB::bind_static_method("Z_BGM", D_METHOD("RestoreFromSave"), &Z_BGM::restore_from_save);

	// … plus snake_case aliases.
	ClassDB::bind_static_method("Z_BGM", D_METHOD("play", "bgm", "start_time", "fade_previous", "fade_duration", "force_restart", "variant"),
			&Z_BGM::play, DEFVAL(0.0f), DEFVAL(true), DEFVAL(0.5f), DEFVAL(false), DEFVAL(StringName()));
	ClassDB::bind_static_method("Z_BGM", D_METHOD("lock", "locked"), &Z_BGM::lock);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("stop", "fade", "fade_duration"), &Z_BGM::stop, DEFVAL(true), DEFVAL(1.0f));
	ClassDB::bind_static_method("Z_BGM", D_METHOD("restore_from_save"), &Z_BGM::restore_from_save);

	ClassDB::bind_static_method("Z_BGM", D_METHOD("is_locked"), &Z_BGM::is_locked);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("is_playing"), &Z_BGM::is_playing);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("get_current_track"), &Z_BGM::get_current_track);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("get_current_variant"), &Z_BGM::get_current_variant);
	ClassDB::bind_static_method("Z_BGM", D_METHOD("get_position"), &Z_BGM::get_position);
}

bool Z_BGM::play(const Ref<ZBgmTrack> &p_bgm, float p_start_time, bool p_fade_previous,
		float p_fade_duration, bool p_force_restart, const StringName &p_variant) {
	ZBgmPlayer *player = ZBgmPlayer::get_or_create();
	if (player == nullptr) {
		return false;
	}
	return player->play(p_bgm, p_variant, p_start_time, p_fade_previous, p_fade_duration, p_force_restart);
}

void Z_BGM::lock(bool p_locked) {
	// Locking is only meaningful once something exists to lock; unlocking a
	// player that was never created is a no-op.
	ZBgmPlayer *player = p_locked ? ZBgmPlayer::get_or_create() : ZBgmPlayer::get_existing();
	if (player == nullptr) {
		return;
	}
	player->set_locked(p_locked);
}

bool Z_BGM::is_locked() {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	return player != nullptr && player->is_locked();
}

bool Z_BGM::stop(bool p_fade, float p_fade_duration) {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	if (player == nullptr) {
		return false;
	}
	return player->stop(p_fade, p_fade_duration);
}

Ref<ZBgmTrack> Z_BGM::get_current_track() {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	if (player == nullptr) {
		return Ref<ZBgmTrack>();
	}
	return player->get_current_track();
}

StringName Z_BGM::get_current_variant() {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	if (player == nullptr) {
		return StringName();
	}
	return player->get_current_variant();
}

bool Z_BGM::is_playing() {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	return player != nullptr && player->is_playing();
}

float Z_BGM::get_position() {
	ZBgmPlayer *player = ZBgmPlayer::get_existing();
	if (player == nullptr) {
		return 0.0f;
	}
	return player->get_position();
}

bool Z_BGM::restore_from_save() {
	ZBgmPlayer *player = ZBgmPlayer::get_or_create();
	if (player == nullptr) {
		return false;
	}
	return player->restore_from_save();
}
