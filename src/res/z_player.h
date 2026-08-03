#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Player manager resource. Always at least one per game instance; owned by Z.
// Holds a weak reference to the currently possessed pawn/scene root.
class ZPlayer : public Resource {
	GDCLASS(ZPlayer, Resource)

private:
	int player_index = 0;
	ObjectID possessed_object_id;
	// Camera / aim euler (radians): X pitch, Y yaw, Z roll. Driven by look input.
	Vector3 control_rotation;

protected:
	static void _bind_methods();

public:
	void set_player_index(int p_index);
	int get_player_index() const;

	void set_control_rotation(const Vector3 &p_rotation);
	Vector3 get_control_rotation() const;

	// Runtime possession (not serialized).
	void possess(Node *p_scene);
	void unpossess();
	Node *get_possessed_scene() const;
	void set_possessed_scene(Node *p_scene);

	ZPlayer();
	~ZPlayer();
};

} // namespace godot
