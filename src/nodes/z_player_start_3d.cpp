#include "z_player_start_3d.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZPlayerStart3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZPlayerStart3D::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZPlayerStart3D::get_player_index);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index", PROPERTY_HINT_RANGE, "0,7,1"), "set_player_index", "get_player_index");
}

ZPlayerStart3D::ZPlayerStart3D() {
}

ZPlayerStart3D::~ZPlayerStart3D() {
}

void ZPlayerStart3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		add_to_group("z_player_start");
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		remove_from_group("z_player_start");
	}
}

void ZPlayerStart3D::set_player_index(int p_index) {
	player_index = p_index;
}

int ZPlayerStart3D::get_player_index() const {
	return player_index;
}
