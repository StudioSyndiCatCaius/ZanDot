#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class ZDataOption;

// Player manager resource. Always at least one per game instance; owned by Z.
// Holds a weak reference to the currently possessed pawn/scene root, and the list
// of objects that receive this player's input.
class ZPlayer : public Resource {
	GDCLASS(ZPlayer, Resource)

public:
	enum EInputState {
		NONE = 0,
		PRESSED = 1, // First frame the action went down.
		DOWN = 2, // Held (repeats every frame).
		RELEASED = 3, // Frame the action came up.
	};

private:
	int player_index = 0;
	ObjectID possessed_object_id;
	// Camera / aim euler (radians): X pitch, Y yaw, Z roll. Driven by look input.
	Vector3 control_rotation;
	// Objects receiving input_triggered(); each must implement that method to be added.
	// Held weakly (like possessed_scene) — targets are kept alive by their owner.
	Vector<ObjectID> input_target_ids;
	// One selected ZDataOption per player (UI lists share this slot).
	ObjectID selected_data_option_id;

	void _prune_input_targets();

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

	// --- Input routing ---
	// Only objects with an `input_triggered` method are accepted.
	void set_input_targets(const Array &p_targets);
	Array get_input_targets() const;
	bool input_target_add(Object *p_target);
	void input_target_remove(Object *p_target);
	void input_targets_clear();
	bool has_input_target(Object *p_target) const;

	// Dispatch one action to every target that accepts input right now:
	//   input_triggered(player: ZPlayer, action: StringName, state: EInputState, dt: float)
	// A target is skipped when it has `input_is_enabled()` and it returns false.
	void input_action(const StringName &p_action, EInputState p_state, double p_delta = 0.0);
	// Convenience wrappers — connect straight to ZInputListener's signals.
	void input_begin(const StringName &p_action);
	void input_update(const StringName &p_action, double p_delta);
	void input_end(const StringName &p_action);

	// UI selection — at most one ZDataOption selected per player.
	void set_selected_data_option(ZDataOption *p_option);
	ZDataOption *get_selected_data_option() const;
	void clear_selected_data_option();

	ZPlayer();
	~ZPlayer();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::ZPlayer::EInputState);
