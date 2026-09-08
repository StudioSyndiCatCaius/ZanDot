#pragma once

#include "z_menu.h"

#include "res/flow_node_choice.h"

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/signal.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Global async events — right now, menus. Static like Z_BGM: call
// `ZEvent.Confirm(...)` from anywhere, no autoload.
//
// Every helper spawns a ZMenu under the root viewport and returns that menu's
// `tree_exited` signal, so awaiting one resumes when the menu is *totally*
// closed (after its close animation), not when it was answered:
//
//     await ZEvent.Info("Saved.")
//     await ZEvent.Confirm("Quit to title?", _on_yes, _on_no)
//
// The scene each helper opens comes from Project Settings → ZanDot → Menus.
// When a slot is empty (or its scene root is not the right ZMenu subclass) a
// plain built-in fallback is used instead, so the call never silently does
// nothing.
class ZEvent : public RefCounted {
	GDCLASS(ZEvent, RefCounted)

private:
	// Instantiates the scene named by p_setting_key; null when unset, unloadable,
	// or the root is not p_class_name.
	static ZMenu *_menu_from_setting(const char *p_setting_key, const StringName &p_class_name);
	// Parents p_menu to the root viewport; returns its tree_exited signal.
	static Signal _spawn(ZMenu *p_menu);

	static void _on_confirm_result(bool p_confirmed, Callable p_on_yes, Callable p_on_no);
	static void _on_choice_result(int p_index, Callable p_on_select);

protected:
	static void _bind_methods();

public:
	// Yes/no prompt. on_yes / on_no are optional and take no arguments.
	static Signal confirm(const String &p_text, const Callable &p_on_yes = Callable(), const Callable &p_on_no = Callable(),
			const String &p_yes_text = "Yes", const String &p_no_text = "No");

	// Message with a single dismiss.
	static Signal info(const String &p_text);

	// Progress menu; p_update is polled each frame and must return 0–1.
	// Closes itself once it returns >= 1.
	static Signal process(const String &p_text, const Callable &p_update);

	// Choice list. p_on_select receives the index into p_choices.
	// p_override_menu swaps in a custom ZMenu_Choice scene for this call only.
	static Signal choice(const TypedArray<ZFlowNode_Choice> &p_choices, const Callable &p_on_select = Callable(),
			const Ref<PackedScene> &p_override_menu = Ref<PackedScene>());

	// Opens any ZMenu scene and hands back the node (await its `tree_exited`).
	static ZMenu *open_menu(const Ref<PackedScene> &p_scene);

	static TypedArray<ZMenu> get_open_menus();
	static ZMenu *get_top_menu();
	static bool is_menu_open();
};

} // namespace godot
