#pragma once

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Base class for a menu opened as a global event.
//
// Menus are meant to be spawned through `ZEvent.*` rather than placed by hand:
// each ZEvent helper instantiates a ZMenu, parents it to the root viewport and
// hands back the menu's `tree_exited` signal, so `await ZEvent.Info("hi")`
// resumes only once the menu is *totally* gone (close animation included).
//
// Opening/closing is driven by `animation_player`: `anim_opened` plays on ready,
// `anim_closed` on CloseMenu() — played backwards when `anim_closed_reversed` is
// set, which is the default since both names point at "Open".
// With no animation player the two phases resolve immediately.
class ZMenu : public CanvasLayer {
	GDCLASS(ZMenu, CanvasLayer)

private:
	AnimationPlayer *animation_player = nullptr;
	StringName anim_opened = StringName("Open");
	StringName anim_closed = StringName("Open");
	bool anim_closed_reversed = true;
	bool pause_gameplay = true;

	StringName action_confirm = StringName("ui_select");
	StringName action_close = StringName("ui_cancel");

	bool is_opening = false;
	bool is_closing = false;
	bool is_open = false;
	// This menu is one of the ones currently holding the tree paused.
	bool holds_pause = false;
	bool anim_connected = false;

	// Every ZMenu in the tree, oldest first. Used for Menu_Get_Top / pause refcount.
	static Vector<ObjectID> open_menus;
	static int pause_holders;

	void _register_open();
	void _unregister_open();
	static void _prune_open_menus();

	void _take_pause();
	void _release_pause();

	void _connect_animation_player();
	void _disconnect_animation_player();
	void _on_animation_finished(const StringName &p_anim);

	void _begin_open();
	void _finish_open();
	void _finish_close();

protected:
	static void _bind_methods();
	void _notification(int p_what);

	// C++ override points; GDScript uses _on_menu_opened / _on_menu_closed.
	virtual void on_menu_opened();
	virtual void on_menu_closed();

	GDVIRTUAL0(_on_menu_opened)
	GDVIRTUAL0(_on_menu_closed)

	// Subclass helper: true when p_event is a fresh press of p_action and the
	// menu is settled (not mid open/close). Marks the event handled.
	bool consume_action(const Ref<InputEvent> &p_event, const StringName &p_action);

public:
	void set_animation_player(AnimationPlayer *p_player);
	AnimationPlayer *get_animation_player() const;

	void set_anim_opened(const StringName &p_anim);
	StringName get_anim_opened() const;

	void set_anim_closed(const StringName &p_anim);
	StringName get_anim_closed() const;

	void set_anim_closed_reversed(bool p_reversed);
	bool get_anim_closed_reversed() const;

	void set_pause_gameplay(bool p_pause);
	bool get_pause_gameplay() const;

	void set_action_confirm(const StringName &p_action);
	StringName get_action_confirm() const;

	void set_action_close(const StringName &p_action);
	StringName get_action_close() const;

	bool get_is_opening() const;
	bool get_is_closing() const;
	// True once the open animation finished and before CloseMenu().
	bool get_is_open() const;

	// Starts the close animation, then frees the menu. Safe to call twice.
	void close_menu();

	// Menus currently in the tree, oldest first.
	static TypedArray<ZMenu> get_open_menus();
	// Most recently opened menu, or null.
	static ZMenu *get_top_menu();

	ZMenu();
	~ZMenu();
};

} // namespace godot
