#include "z_menu.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

Vector<ObjectID> ZMenu::open_menus;
int ZMenu::pause_holders = 0;

void ZMenu::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_animation_player", "player"), &ZMenu::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &ZMenu::get_animation_player);

	ClassDB::bind_method(D_METHOD("set_anim_opened", "anim"), &ZMenu::set_anim_opened);
	ClassDB::bind_method(D_METHOD("get_anim_opened"), &ZMenu::get_anim_opened);

	ClassDB::bind_method(D_METHOD("set_anim_closed", "anim"), &ZMenu::set_anim_closed);
	ClassDB::bind_method(D_METHOD("get_anim_closed"), &ZMenu::get_anim_closed);

	ClassDB::bind_method(D_METHOD("set_anim_closed_reversed", "reversed"), &ZMenu::set_anim_closed_reversed);
	ClassDB::bind_method(D_METHOD("get_anim_closed_reversed"), &ZMenu::get_anim_closed_reversed);

	ClassDB::bind_method(D_METHOD("set_pause_gameplay", "pause"), &ZMenu::set_pause_gameplay);
	ClassDB::bind_method(D_METHOD("get_pause_gameplay"), &ZMenu::get_pause_gameplay);

	ClassDB::bind_method(D_METHOD("set_action_confirm", "action"), &ZMenu::set_action_confirm);
	ClassDB::bind_method(D_METHOD("get_action_confirm"), &ZMenu::get_action_confirm);

	ClassDB::bind_method(D_METHOD("set_action_close", "action"), &ZMenu::set_action_close);
	ClassDB::bind_method(D_METHOD("get_action_close"), &ZMenu::get_action_close);

	ClassDB::bind_method(D_METHOD("get_is_opening"), &ZMenu::get_is_opening);
	ClassDB::bind_method(D_METHOD("get_is_closing"), &ZMenu::get_is_closing);
	ClassDB::bind_method(D_METHOD("get_is_open"), &ZMenu::get_is_open);

	ClassDB::bind_method(D_METHOD("CloseMenu"), &ZMenu::close_menu);
	ClassDB::bind_method(D_METHOD("close_menu"), &ZMenu::close_menu);

	ClassDB::bind_static_method("ZMenu", D_METHOD("Menus_Get_Open"), &ZMenu::get_open_menus);
	ClassDB::bind_static_method("ZMenu", D_METHOD("get_open_menus"), &ZMenu::get_open_menus);
	ClassDB::bind_static_method("ZMenu", D_METHOD("Menu_Get_Top"), &ZMenu::get_top_menu);
	ClassDB::bind_static_method("ZMenu", D_METHOD("get_top_menu"), &ZMenu::get_top_menu);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "animation_player", PROPERTY_HINT_NODE_TYPE, "AnimationPlayer"), "set_animation_player", "get_animation_player");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "anim_opened"), "set_anim_opened", "get_anim_opened");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "anim_closed"), "set_anim_closed", "get_anim_closed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "anim_closed_reversed"), "set_anim_closed_reversed", "get_anim_closed_reversed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pause_gameplay"), "set_pause_gameplay", "get_pause_gameplay");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action_confirm"), "set_action_confirm", "get_action_confirm");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action_close"), "set_action_close", "get_action_close");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_opening", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_is_opening");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_closing", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_is_closing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_open", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_READ_ONLY), "", "get_is_open");

	ADD_SIGNAL(MethodInfo("on_menu_opened"));
	ADD_SIGNAL(MethodInfo("on_menu_closed"));

	GDVIRTUAL_BIND(_on_menu_opened);
	GDVIRTUAL_BIND(_on_menu_closed);
}

ZMenu::ZMenu() {
}

ZMenu::~ZMenu() {
}

void ZMenu::_notification(int p_what) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (pause_gameplay) {
				// "Naturally excludes self" — the menu keeps running while the
				// tree it paused is frozen.
				set_process_mode(Node::PROCESS_MODE_ALWAYS);
			}
			_register_open();
			_take_pause();
		} break;

		case NOTIFICATION_READY: {
			_begin_open();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			_disconnect_animation_player();
			_release_pause();
			_unregister_open();
		} break;

		default:
			break;
	}
}

// --- open menu registry -----------------------------------------------------

void ZMenu::_prune_open_menus() {
	for (int i = open_menus.size() - 1; i >= 0; i--) {
		if (Object::cast_to<ZMenu>(ObjectDB::get_instance(open_menus[i])) == nullptr) {
			open_menus.remove_at(i);
		}
	}
}

void ZMenu::_register_open() {
	_prune_open_menus();
	const ObjectID id = ObjectID(get_instance_id());
	if (open_menus.find(id) == -1) {
		open_menus.push_back(id);
	}
}

void ZMenu::_unregister_open() {
	const int idx = open_menus.find(ObjectID(get_instance_id()));
	if (idx >= 0) {
		open_menus.remove_at(idx);
	}
	_prune_open_menus();
}

TypedArray<ZMenu> ZMenu::get_open_menus() {
	_prune_open_menus();
	TypedArray<ZMenu> result;
	for (int i = 0; i < open_menus.size(); i++) {
		ZMenu *menu = Object::cast_to<ZMenu>(ObjectDB::get_instance(open_menus[i]));
		if (menu != nullptr) {
			result.push_back(menu);
		}
	}
	return result;
}

ZMenu *ZMenu::get_top_menu() {
	_prune_open_menus();
	for (int i = open_menus.size() - 1; i >= 0; i--) {
		ZMenu *menu = Object::cast_to<ZMenu>(ObjectDB::get_instance(open_menus[i]));
		if (menu != nullptr && !menu->is_closing) {
			return menu;
		}
	}
	return nullptr;
}

// --- pause ------------------------------------------------------------------

void ZMenu::_take_pause() {
	if (!pause_gameplay || holds_pause) {
		return;
	}
	holds_pause = true;
	pause_holders++;
	SceneTree *tree = get_tree();
	if (pause_holders == 1 && tree != nullptr) {
		tree->set_pause(true);
	}
}

void ZMenu::_release_pause() {
	if (!holds_pause) {
		return;
	}
	holds_pause = false;
	pause_holders--;
	if (pause_holders <= 0) {
		pause_holders = 0;
		SceneTree *tree = get_tree();
		if (tree != nullptr) {
			tree->set_pause(false);
		}
	}
}

// --- animation --------------------------------------------------------------

void ZMenu::_connect_animation_player() {
	if (anim_connected || animation_player == nullptr) {
		return;
	}
	const Callable cb = callable_mp(this, &ZMenu::_on_animation_finished);
	if (!animation_player->is_connected("animation_finished", cb)) {
		animation_player->connect("animation_finished", cb);
	}
	anim_connected = true;
}

void ZMenu::_disconnect_animation_player() {
	if (!anim_connected || animation_player == nullptr) {
		anim_connected = false;
		return;
	}
	const Callable cb = callable_mp(this, &ZMenu::_on_animation_finished);
	if (animation_player->is_connected("animation_finished", cb)) {
		animation_player->disconnect("animation_finished", cb);
	}
	anim_connected = false;
}

void ZMenu::_on_animation_finished(const StringName &p_anim) {
	if (is_closing) {
		if (anim_closed.is_empty() || p_anim == anim_closed) {
			_finish_close();
		}
		return;
	}
	if (is_opening && (anim_opened.is_empty() || p_anim == anim_opened)) {
		_finish_open();
	}
}

// --- open / close -----------------------------------------------------------

void ZMenu::_begin_open() {
	_connect_animation_player();

	if (animation_player != nullptr && !anim_opened.is_empty() && animation_player->has_animation(anim_opened)) {
		is_opening = true;
		animation_player->play(anim_opened);
		return;
	}
	_finish_open();
}

void ZMenu::_finish_open() {
	if (is_open || is_closing) {
		return;
	}
	is_opening = false;
	is_open = true;

	on_menu_opened();
	if (has_method("_on_menu_opened")) {
		call("_on_menu_opened");
	} else {
		GDVIRTUAL_CALL(_on_menu_opened);
	}
	emit_signal("on_menu_opened");
}

void ZMenu::close_menu() {
	if (is_closing || !is_inside_tree()) {
		return;
	}
	// Closing before the open animation settled still counts as having opened.
	is_opening = false;
	is_closing = true;
	is_open = false;

	on_menu_closed();
	if (has_method("_on_menu_closed")) {
		call("_on_menu_closed");
	} else {
		GDVIRTUAL_CALL(_on_menu_closed);
	}
	emit_signal("on_menu_closed");

	_connect_animation_player();
	if (animation_player != nullptr && !anim_closed.is_empty() && animation_player->has_animation(anim_closed)) {
		if (anim_closed_reversed) {
			animation_player->play_backwards(anim_closed);
		} else {
			animation_player->play(anim_closed);
		}
		return;
	}
	_finish_close();
}

void ZMenu::_finish_close() {
	_disconnect_animation_player();
	_release_pause();
	// tree_exited fires from here — that is what the ZEvent.* helpers await.
	queue_free();
}

void ZMenu::on_menu_opened() {
}

void ZMenu::on_menu_closed() {
}

bool ZMenu::consume_action(const Ref<InputEvent> &p_event, const StringName &p_action) {
	if (p_event.is_null() || p_action.is_empty() || is_opening || is_closing) {
		return false;
	}
	// Only the front-most menu reacts; stacked menus stay inert underneath.
	if (get_top_menu() != this) {
		return false;
	}
	InputMap *map = InputMap::get_singleton();
	if (map == nullptr || !map->has_action(p_action)) {
		return false;
	}
	if (!p_event->is_action_pressed(p_action)) {
		return false;
	}
	Viewport *vp = get_viewport();
	if (vp != nullptr) {
		vp->set_input_as_handled();
	}
	return true;
}

// --- properties -------------------------------------------------------------

void ZMenu::set_animation_player(AnimationPlayer *p_player) {
	if (animation_player == p_player) {
		return;
	}
	_disconnect_animation_player();
	animation_player = p_player;
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint() && (is_opening || is_closing)) {
		_connect_animation_player();
	}
}

AnimationPlayer *ZMenu::get_animation_player() const {
	return animation_player;
}

void ZMenu::set_anim_opened(const StringName &p_anim) {
	anim_opened = p_anim;
}

StringName ZMenu::get_anim_opened() const {
	return anim_opened;
}

void ZMenu::set_anim_closed(const StringName &p_anim) {
	anim_closed = p_anim;
}

StringName ZMenu::get_anim_closed() const {
	return anim_closed;
}

void ZMenu::set_anim_closed_reversed(bool p_reversed) {
	anim_closed_reversed = p_reversed;
}

bool ZMenu::get_anim_closed_reversed() const {
	return anim_closed_reversed;
}

void ZMenu::set_pause_gameplay(bool p_pause) {
	if (pause_gameplay == p_pause) {
		return;
	}
	pause_gameplay = p_pause;
	if (!is_inside_tree() || Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (pause_gameplay) {
		set_process_mode(Node::PROCESS_MODE_ALWAYS);
		_take_pause();
	} else {
		_release_pause();
	}
}

bool ZMenu::get_pause_gameplay() const {
	return pause_gameplay;
}

void ZMenu::set_action_confirm(const StringName &p_action) {
	action_confirm = p_action;
}

StringName ZMenu::get_action_confirm() const {
	return action_confirm;
}

void ZMenu::set_action_close(const StringName &p_action) {
	action_close = p_action;
}

StringName ZMenu::get_action_close() const {
	return action_close;
}

bool ZMenu::get_is_opening() const {
	return is_opening;
}

bool ZMenu::get_is_closing() const {
	return is_closing;
}

bool ZMenu::get_is_open() const {
	return is_open;
}
