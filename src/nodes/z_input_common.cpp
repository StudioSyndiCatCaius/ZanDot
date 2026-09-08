#include "z_input_common.h"

#include "z.h"
#include "../res/z_player.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

namespace {

struct ActionDef {
	const char *property;
	const char *default_action;
};

// Order must match ZInputCommon::EAction.
const ActionDef ACTION_DEFS[ZInputCommon::ACTION_MAX] = {
	{ "action_confirm", "ui_select" },
	{ "action_cancel", "ui_cancel" },
	{ "action_face_top", "z_face_top" },
	{ "action_face_left", "z_face_left" },
	{ "action_special_l", "z_special_l" },
	{ "action_special_r", "z_special_r" },
	{ "action_shoulder_l", "z_shoulder_l" },
	{ "action_shoulder_r", "z_shoulder_r" },
	{ "action_trigger_l", "z_trigger_l" },
	{ "action_trigger_r", "z_trigger_r" },
	{ "action_move_up", "z_move_up" },
	{ "action_move_down", "z_move_down" },
	{ "action_move_left", "z_move_left" },
	{ "action_move_right", "z_move_right" },
	{ "action_look_up", "z_look_up" },
	{ "action_look_down", "z_look_down" },
	{ "action_look_left", "z_look_left" },
	{ "action_look_right", "z_look_right" },
	{ "action_nav_up", "z_nav_up" },
	{ "action_nav_down", "z_nav_down" },
	{ "action_nav_left", "z_nav_left" },
	{ "action_nav_right", "z_nav_right" },
};

// Actions that fire a signal on their press edge. Move/look are analog and
// handled separately; nav is here but emits a direction instead of a bare signal.
const ZInputCommon::EAction BUTTON_ACTIONS[] = {
	ZInputCommon::ACTION_CONFIRM,
	ZInputCommon::ACTION_CANCEL,
	ZInputCommon::ACTION_FACE_TOP,
	ZInputCommon::ACTION_FACE_LEFT,
	ZInputCommon::ACTION_SPECIAL_L,
	ZInputCommon::ACTION_SPECIAL_R,
	ZInputCommon::ACTION_SHOULDER_L,
	ZInputCommon::ACTION_SHOULDER_R,
	ZInputCommon::ACTION_TRIGGER_L,
	ZInputCommon::ACTION_TRIGGER_R,
	ZInputCommon::ACTION_NAV_UP,
	ZInputCommon::ACTION_NAV_DOWN,
	ZInputCommon::ACTION_NAV_LEFT,
	ZInputCommon::ACTION_NAV_RIGHT,
};

const int BUTTON_ACTION_COUNT = (int)(sizeof(BUTTON_ACTIONS) / sizeof(BUTTON_ACTIONS[0]));

} // namespace

void ZInputCommon::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_visibility_dependency", "node"), &ZInputCommon::set_visibility_dependency);
	ClassDB::bind_method(D_METHOD("get_visibility_dependency"), &ZInputCommon::get_visibility_dependency);

	ClassDB::bind_method(D_METHOD("set_is_input_target", "enabled"), &ZInputCommon::set_is_input_target);
	ClassDB::bind_method(D_METHOD("get_is_input_target"), &ZInputCommon::get_is_input_target);

	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZInputCommon::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZInputCommon::get_player_index);

	ClassDB::bind_method(D_METHOD("set_action", "action", "name"), &ZInputCommon::set_action);
	ClassDB::bind_method(D_METHOD("get_action", "action"), &ZInputCommon::get_action);

	ClassDB::bind_method(D_METHOD("set_use_mouse_look", "enabled"), &ZInputCommon::set_use_mouse_look);
	ClassDB::bind_method(D_METHOD("get_use_mouse_look"), &ZInputCommon::get_use_mouse_look);

	ClassDB::bind_method(D_METHOD("set_mouse_look_sensitivity", "sensitivity"), &ZInputCommon::set_mouse_look_sensitivity);
	ClassDB::bind_method(D_METHOD("get_mouse_look_sensitivity"), &ZInputCommon::get_mouse_look_sensitivity);

	// ZPlayer input-target contract — must stay bound for has_method() to find them.
	ClassDB::bind_method(D_METHOD("input_triggered", "player", "action", "state", "dt"), &ZInputCommon::input_triggered);
	ClassDB::bind_method(D_METHOD("input_is_enabled"), &ZInputCommon::input_is_enabled);

	ClassDB::bind_method(D_METHOD("poll_input", "delta"), &ZInputCommon::poll_input);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "visibility_dependency", PROPERTY_HINT_NODE_TYPE, "Node"), "set_visibility_dependency", "get_visibility_dependency");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_input_target"), "set_is_input_target", "get_is_input_target");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index", PROPERTY_HINT_RANGE, "0,7,1"), "set_player_index", "get_player_index");

	ADD_GROUP("Mouse Look", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_mouse_look"), "set_use_mouse_look", "get_use_mouse_look");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mouse_look_sensitivity", PROPERTY_HINT_RANGE, "0,1,0.005,or_greater"), "set_mouse_look_sensitivity", "get_mouse_look_sensitivity");

	// One indexed property per binding, so the inspector shows 22 named action
	// fields while the table stays a single array.
	ADD_GROUP("Actions", "action_");
	for (int i = 0; i < ACTION_MAX; i++) {
		ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, ACTION_DEFS[i].property), "set_action", "get_action", i);
	}

	ADD_SIGNAL(MethodInfo("input_confirm"));
	ADD_SIGNAL(MethodInfo("input_cancel"));
	ADD_SIGNAL(MethodInfo("input_face_top"));
	ADD_SIGNAL(MethodInfo("input_face_left"));
	ADD_SIGNAL(MethodInfo("special_R"));
	ADD_SIGNAL(MethodInfo("special_L"));
	ADD_SIGNAL(MethodInfo("shoulder", PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("input_trigger", PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("input_trigger_update", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::FLOAT, "dt")));
	ADD_SIGNAL(MethodInfo("input_move", PropertyInfo(Variant::VECTOR3, "axis"), PropertyInfo(Variant::FLOAT, "dt")));
	ADD_SIGNAL(MethodInfo("input_rotate", PropertyInfo(Variant::VECTOR3, "axis"), PropertyInfo(Variant::FLOAT, "dt")));
	ADD_SIGNAL(MethodInfo("input_nav", PropertyInfo(Variant::VECTOR2I, "axis")));

	BIND_ENUM_CONSTANT(ACTION_CONFIRM);
	BIND_ENUM_CONSTANT(ACTION_CANCEL);
	BIND_ENUM_CONSTANT(ACTION_FACE_TOP);
	BIND_ENUM_CONSTANT(ACTION_FACE_LEFT);
	BIND_ENUM_CONSTANT(ACTION_SPECIAL_L);
	BIND_ENUM_CONSTANT(ACTION_SPECIAL_R);
	BIND_ENUM_CONSTANT(ACTION_SHOULDER_L);
	BIND_ENUM_CONSTANT(ACTION_SHOULDER_R);
	BIND_ENUM_CONSTANT(ACTION_TRIGGER_L);
	BIND_ENUM_CONSTANT(ACTION_TRIGGER_R);
	BIND_ENUM_CONSTANT(ACTION_MOVE_UP);
	BIND_ENUM_CONSTANT(ACTION_MOVE_DOWN);
	BIND_ENUM_CONSTANT(ACTION_MOVE_LEFT);
	BIND_ENUM_CONSTANT(ACTION_MOVE_RIGHT);
	BIND_ENUM_CONSTANT(ACTION_LOOK_UP);
	BIND_ENUM_CONSTANT(ACTION_LOOK_DOWN);
	BIND_ENUM_CONSTANT(ACTION_LOOK_LEFT);
	BIND_ENUM_CONSTANT(ACTION_LOOK_RIGHT);
	BIND_ENUM_CONSTANT(ACTION_NAV_UP);
	BIND_ENUM_CONSTANT(ACTION_NAV_DOWN);
	BIND_ENUM_CONSTANT(ACTION_NAV_LEFT);
	BIND_ENUM_CONSTANT(ACTION_NAV_RIGHT);
	BIND_ENUM_CONSTANT(ACTION_MAX);

	BIND_ENUM_CONSTANT(SIDE_LEFT);
	BIND_ENUM_CONSTANT(SIDE_RIGHT);
}

ZInputCommon::ZInputCommon() {
	for (int i = 0; i < ACTION_MAX; i++) {
		actions[i] = StringName(ACTION_DEFS[i].default_action);
	}
	set_process(true);
	set_process_input(true);
}

ZInputCommon::~ZInputCommon() {
}

void ZInputCommon::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (Engine::get_singleton()->is_editor_hint()) {
				set_process(false);
				set_process_input(false);
				return;
			}
			_sync_player_registration();
		} break;
		case NOTIFICATION_PROCESS: {
			poll_input(get_process_delta_time());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_unregister_from_player();
			_release_all();
		} break;
		default:
			break;
	}
}

void ZInputCommon::_input(const Ref<InputEvent> &p_event) {
	if (!use_mouse_look) {
		return;
	}
	Ref<InputEventMouseMotion> motion = p_event;
	if (motion.is_null()) {
		return;
	}
	// Only while the mouse is captured — otherwise moving the cursor over a menu
	// would swing the camera.
	Input *input = Input::get_singleton();
	if (input == nullptr || input->get_mouse_mode() != Input::MOUSE_MODE_CAPTURED) {
		return;
	}
	mouse_motion += motion->get_relative();
}

// --- Properties -------------------------------------------------------------

void ZInputCommon::set_visibility_dependency(Node *p_node) {
	visibility_dependency_id = p_node != nullptr ? ObjectID(p_node->get_instance_id()) : ObjectID();
}

Node *ZInputCommon::get_visibility_dependency() const {
	return Object::cast_to<Node>(ObjectDB::get_instance(visibility_dependency_id));
}

void ZInputCommon::set_is_input_target(bool p_enabled) {
	if (is_input_target == p_enabled) {
		return;
	}
	is_input_target = p_enabled;
	_sync_player_registration();
}

bool ZInputCommon::get_is_input_target() const {
	return is_input_target;
}

void ZInputCommon::set_player_index(int p_index) {
	if (player_index == p_index) {
		return;
	}
	// Move the registration across rather than leaving it on the old player.
	_unregister_from_player();
	player_index = p_index;
	_sync_player_registration();
}

int ZInputCommon::get_player_index() const {
	return player_index;
}

void ZInputCommon::set_action(int p_action, const StringName &p_name) {
	if (p_action < 0 || p_action >= ACTION_MAX) {
		return;
	}
	actions[p_action] = p_name;
}

StringName ZInputCommon::get_action(int p_action) const {
	if (p_action < 0 || p_action >= ACTION_MAX) {
		return StringName();
	}
	return actions[p_action];
}

void ZInputCommon::set_use_mouse_look(bool p_enabled) {
	use_mouse_look = p_enabled;
	if (!use_mouse_look) {
		mouse_motion = Vector2();
	}
}

bool ZInputCommon::get_use_mouse_look() const {
	return use_mouse_look;
}

void ZInputCommon::set_mouse_look_sensitivity(float p_sensitivity) {
	mouse_look_sensitivity = p_sensitivity;
}

float ZInputCommon::get_mouse_look_sensitivity() const {
	return mouse_look_sensitivity;
}

// --- Gating & player wiring -------------------------------------------------

bool ZInputCommon::_is_input_enabled() const {
	Node *root = get_visibility_dependency();
	if (root == nullptr) {
		// No gate configured → always allow input.
		return true;
	}

	// Prefer CanvasItem / Node3D visibility; fall back to tree presence.
	CanvasItem *ci = Object::cast_to<CanvasItem>(root);
	if (ci != nullptr) {
		return ci->is_visible_in_tree();
	}
	Node3D *n3 = Object::cast_to<Node3D>(root);
	if (n3 != nullptr) {
		return n3->is_visible_in_tree();
	}
	return root->is_inside_tree();
}

bool ZInputCommon::input_is_enabled() const {
	return _is_input_enabled();
}

ZPlayer *ZInputCommon::_resolve_player() const {
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return nullptr;
	}
	TypedArray<ZPlayer> players = z->get_players();
	if (player_index < 0 || player_index >= players.size()) {
		return nullptr;
	}
	return Object::cast_to<ZPlayer>(players[player_index]);
}

void ZInputCommon::_sync_player_registration() {
	if (!is_inside_tree() || Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!is_input_target) {
		_unregister_from_player();
		return;
	}
	ZPlayer *player = _resolve_player();
	if (player == nullptr || ObjectID(player->get_instance_id()) == registered_player_id) {
		return;
	}
	_unregister_from_player();
	if (player->input_target_add(this)) {
		registered_player_id = ObjectID(player->get_instance_id());
	}
}

void ZInputCommon::_unregister_from_player() {
	if (registered_player_id.is_null()) {
		return;
	}
	ZPlayer *player = Object::cast_to<ZPlayer>(ObjectDB::get_instance(registered_player_id));
	if (player != nullptr) {
		player->input_target_remove(this);
	}
	registered_player_id = ObjectID();
}

// --- Signal emission --------------------------------------------------------

void ZInputCommon::_emit_for_action(EAction p_action) {
	switch (p_action) {
		case ACTION_CONFIRM:
			emit_signal("input_confirm");
			break;
		case ACTION_CANCEL:
			emit_signal("input_cancel");
			break;
		case ACTION_FACE_TOP:
			emit_signal("input_face_top");
			break;
		case ACTION_FACE_LEFT:
			emit_signal("input_face_left");
			break;
		case ACTION_SPECIAL_L:
			emit_signal("special_L");
			break;
		case ACTION_SPECIAL_R:
			emit_signal("special_R");
			break;
		case ACTION_SHOULDER_L:
			emit_signal("shoulder", (int)SIDE_LEFT);
			break;
		case ACTION_SHOULDER_R:
			emit_signal("shoulder", (int)SIDE_RIGHT);
			break;
		case ACTION_TRIGGER_L:
			emit_signal("input_trigger", (int)SIDE_LEFT);
			break;
		case ACTION_TRIGGER_R:
			emit_signal("input_trigger", (int)SIDE_RIGHT);
			break;
		// Screen convention (+y is down), so a list index can just add axis.y.
		case ACTION_NAV_UP:
			emit_signal("input_nav", Vector2i(0, -1));
			break;
		case ACTION_NAV_DOWN:
			emit_signal("input_nav", Vector2i(0, 1));
			break;
		case ACTION_NAV_LEFT:
			emit_signal("input_nav", Vector2i(-1, 0));
			break;
		case ACTION_NAV_RIGHT:
			emit_signal("input_nav", Vector2i(1, 0));
			break;
		default:
			break;
	}
}

void ZInputCommon::_update_button(EAction p_action, bool p_pressed, double p_delta) {
	if (p_pressed && !held[p_action]) {
		held[p_action] = true;
		_emit_for_action(p_action);
	} else if (!p_pressed) {
		held[p_action] = false;
	}

	// Triggers keep reporting while held, so a charge/aim can ramp up.
	if (p_pressed && p_action == ACTION_TRIGGER_L) {
		emit_signal("input_trigger_update", (int)SIDE_LEFT, p_delta);
	} else if (p_pressed && p_action == ACTION_TRIGGER_R) {
		emit_signal("input_trigger_update", (int)SIDE_RIGHT, p_delta);
	}
}

void ZInputCommon::_release_all() {
	for (int i = 0; i < ACTION_MAX; i++) {
		held[i] = false;
	}
	mouse_motion = Vector2();
	move_was_active = false;
	rotate_was_active = false;
}

// --- Polling ----------------------------------------------------------------

void ZInputCommon::_poll_buttons(double p_delta) {
	Input *input = Input::get_singleton();
	InputMap *map = InputMap::get_singleton();
	if (input == nullptr) {
		return;
	}

	for (int i = 0; i < BUTTON_ACTION_COUNT; i++) {
		const EAction action = BUTTON_ACTIONS[i];
		const StringName &name = actions[action];
		// Unbound or cleared slots are simply inactive, not an error — a project
		// may only want a subset of these.
		if (name == StringName() || (map != nullptr && !map->has_action(name))) {
			held[action] = false;
			continue;
		}
		_update_button(action, input->is_action_pressed(name), p_delta);
	}
}

void ZInputCommon::_poll_axes(double p_delta) {
	Input *input = Input::get_singleton();
	if (input == nullptr) {
		return;
	}

	// Move: matches ZMover::move_by_input — x = strafe, y = forward/back where
	// "up" is negative (Godot's forward is -Z).
	Vector2 move = input->get_vector(actions[ACTION_MOVE_LEFT], actions[ACTION_MOVE_RIGHT], actions[ACTION_MOVE_UP], actions[ACTION_MOVE_DOWN]);
	const bool move_active = move != Vector2();
	if (move_active || move_was_active) {
		emit_signal("input_move", Vector3(move.x, move.y, 0.0f), p_delta);
	}
	move_was_active = move_active;

	// Look: euler deltas matching ZPlayer.control_rotation (X pitch, Y yaw).
	float pitch = input->get_axis(actions[ACTION_LOOK_DOWN], actions[ACTION_LOOK_UP]);
	float yaw = input->get_axis(actions[ACTION_LOOK_RIGHT], actions[ACTION_LOOK_LEFT]);

	if (use_mouse_look && mouse_motion != Vector2() && p_delta > 0.0) {
		// Mouse motion is already a per-frame delta while the stick is per-second,
		// so divide it out — consumers can multiply the axis by dt either way.
		const float scale = mouse_look_sensitivity / (float)p_delta;
		yaw -= mouse_motion.x * scale;
		pitch -= mouse_motion.y * scale;
	}
	mouse_motion = Vector2();

	const Vector3 rotate(pitch, yaw, 0.0f);
	const bool rotate_active = rotate != Vector3();
	if (rotate_active || rotate_was_active) {
		emit_signal("input_rotate", rotate, p_delta);
	}
	rotate_was_active = rotate_active;
}

void ZInputCommon::poll_input(double p_delta) {
	if (!_is_input_enabled()) {
		_release_all();
		return;
	}

	// As an input target the button edges arrive via input_triggered(); only the
	// analog axes still need polling (sticks have no press/release edge).
	if (!is_input_target) {
		_poll_buttons(p_delta);
	} else {
		// A player may be created after this node entered the tree.
		_sync_player_registration();
	}
	_poll_axes(p_delta);
}

void ZInputCommon::input_triggered(ZPlayer *p_player, const StringName &p_action, int p_state, double p_delta) {
	if (!is_input_target || !_is_input_enabled()) {
		return;
	}

	for (int i = 0; i < BUTTON_ACTION_COUNT; i++) {
		const EAction action = BUTTON_ACTIONS[i];
		if (actions[action] != p_action) {
			continue;
		}
		switch ((ZPlayer::EInputState)p_state) {
			case ZPlayer::PRESSED:
				_update_button(action, true, p_delta);
				break;
			case ZPlayer::DOWN:
				// Already latched by PRESSED; this only drives the trigger update.
				held[action] = true;
				if (action == ACTION_TRIGGER_L) {
					emit_signal("input_trigger_update", (int)SIDE_LEFT, p_delta);
				} else if (action == ACTION_TRIGGER_R) {
					emit_signal("input_trigger_update", (int)SIDE_RIGHT, p_delta);
				}
				break;
			case ZPlayer::RELEASED:
				held[action] = false;
				break;
			default:
				break;
		}
	}
}
