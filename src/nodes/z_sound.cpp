#include "z_sound.h"

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Z_Sound::_bind_methods() {
	ClassDB::bind_static_method("Z_Sound", D_METHOD("oneshot", "stream", "volume_db", "pitch_scale"), &Z_Sound::oneshot, DEFVAL(0.0f), DEFVAL(1.0f));
	ClassDB::bind_static_method("Z_Sound", D_METHOD("Oneshot", "stream", "volume_db", "pitch_scale"), &Z_Sound::oneshot, DEFVAL(0.0f), DEFVAL(1.0f));
}

void Z_Sound::oneshot(const Ref<AudioStream> &p_stream, float p_volume_db, float p_pitch_scale) {
	if (p_stream.is_null()) {
		UtilityFunctions::push_warning("Z_Sound.oneshot: stream is null.");
		return;
	}
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr) {
		UtilityFunctions::push_warning("Z_Sound.oneshot: no active SceneTree.");
		return;
	}

	AudioStreamPlayer *player = memnew(AudioStreamPlayer);
	player->set_stream(p_stream);
	player->set_volume_db(p_volume_db);
	player->set_pitch_scale(p_pitch_scale);
	tree->get_root()->add_child(player);
	player->connect("finished", callable_mp(static_cast<Node *>(player), &Node::queue_free));
	player->play();
}
