#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/templates/hash_set.hpp>

namespace godot {

// Fires begin/update/end signals for configured InputMap actions.
// Optionally gated by a visibility_root node's visibility in the tree.
class ZInputListener : public Node {
	GDCLASS(ZInputListener, Node)

private:
	NodePath visibility_root;
	PackedStringArray actions;
	HashSet<StringName> held_actions;
	bool process_unhandled = true;

	bool _is_input_enabled() const;
	Node *_resolve_visibility_root() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_visibility_root(const NodePath &p_path);
	NodePath get_visibility_root() const;

	void set_actions(const PackedStringArray &p_actions);
	PackedStringArray get_actions() const;

	void set_process_unhandled(bool p_enabled);
	bool get_process_unhandled() const;

	// Manual poll (also called from _process).
	void poll_input(double p_delta);

	ZInputListener();
	~ZInputListener();
};

} // namespace godot
