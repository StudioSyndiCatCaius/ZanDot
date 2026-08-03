#pragma once

#include <godot_cpp/classes/marker3d.hpp>

namespace godot {

// Spawn point for a player pawn. player_index maps to Z.players[i].
class ZPlayerStart3D : public Marker3D {
	GDCLASS(ZPlayerStart3D, Marker3D)

private:
	int player_index = 0;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_player_index(int p_index);
	int get_player_index() const;

	ZPlayerStart3D();
	~ZPlayerStart3D();
};

} // namespace godot
