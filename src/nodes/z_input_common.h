#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class InputEvent;
class ZPlayer;

// One node with plain signals for the input every game needs: confirm / cancel,
// face buttons, shoulders, triggers, move, look, menu navigation.
//
// Two ways to drive it:
//   * default — polls the InputMap itself every frame.
//   * is_input_target — registers with Z.players[player_index] instead and reacts
//     to that player's dispatch, so a pause menu can steal input from the pawn.
//     Analog move/look still polls, since sticks are not action edges.
//
// Every binding is an exported action name, defaulting to the `z_*` actions
// ZanDot registers on load (see ZInputDefaults) plus Godot's own ui_select /
// ui_cancel for confirm / cancel.
class ZInputCommon : public Node {
	GDCLASS(ZInputCommon, Node)

public:
	// Indices into the action-name table; also the `action` property indices.
	enum EAction {
		ACTION_CONFIRM,
		ACTION_CANCEL,
		ACTION_FACE_TOP,
		ACTION_FACE_LEFT,
		ACTION_SPECIAL_L,
		ACTION_SPECIAL_R,
		ACTION_SHOULDER_L,
		ACTION_SHOULDER_R,
		ACTION_TRIGGER_L,
		ACTION_TRIGGER_R,
		ACTION_MOVE_UP,
		ACTION_MOVE_DOWN,
		ACTION_MOVE_LEFT,
		ACTION_MOVE_RIGHT,
		ACTION_LOOK_UP,
		ACTION_LOOK_DOWN,
		ACTION_LOOK_LEFT,
		ACTION_LOOK_RIGHT,
		ACTION_NAV_UP,
		ACTION_NAV_DOWN,
		ACTION_NAV_LEFT,
		ACTION_NAV_RIGHT,
		ACTION_MAX,
	};

	// Side passed to the shoulder / trigger signals.
	enum ESide {
		SIDE_LEFT = -1,
		SIDE_RIGHT = 1,
	};

private:
	// Input is only accepted while this node is visible in the tree (any node
	// type; non-visual ones fall back to "is inside the tree").
	ObjectID visibility_dependency_id;
	// Route through Z.players[player_index] instead of polling actions directly.
	bool is_input_target = false;
	int player_index = 0;

	StringName actions[ACTION_MAX];

	bool use_mouse_look = true;
	// Radians of look axis per pixel of mouse motion, before the frame delta.
	float mouse_look_sensitivity = 0.12f;

	// Edge detection for the button-style actions.
	bool held[ACTION_MAX] = {};
	// Mouse motion accumulated since the last poll.
	Vector2 mouse_motion;
	// Whether the last emit was a non-zero axis, so a single zero can close it out.
	bool move_was_active = false;
	bool rotate_was_active = false;
	// Set while registered with a ZPlayer, so we unregister exactly once.
	ObjectID registered_player_id;

	ZPlayer *_resolve_player() const;
	bool _is_input_enabled() const;

	void _sync_player_registration();
	void _unregister_from_player();

	// Button edge → signal. p_pressed is this frame's raw state.
	void _update_button(EAction p_action, bool p_pressed, double p_delta);
	void _emit_for_action(EAction p_action);
	void _poll_buttons(double p_delta);
	void _poll_axes(double p_delta);
	void _release_all();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	virtual void _input(const Ref<InputEvent> &p_event) override;

	void set_visibility_dependency(Node *p_node);
	Node *get_visibility_dependency() const;

	void set_is_input_target(bool p_enabled);
	bool get_is_input_target() const;

	void set_player_index(int p_index);
	int get_player_index() const;

	void set_action(int p_action, const StringName &p_name);
	StringName get_action(int p_action) const;

	void set_use_mouse_look(bool p_enabled);
	bool get_use_mouse_look() const;

	void set_mouse_look_sensitivity(float p_sensitivity);
	float get_mouse_look_sensitivity() const;

	// ZPlayer input-target contract (used when is_input_target is on).
	void input_triggered(ZPlayer *p_player, const StringName &p_action, int p_state, double p_delta);
	bool input_is_enabled() const;

	// Manual poll (also called every frame from _process).
	void poll_input(double p_delta);

	ZInputCommon();
	~ZInputCommon();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::ZInputCommon::EAction);
VARIANT_ENUM_CAST(godot::ZInputCommon::ESide);
