#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

// Stateless helper functions for operating on Node instances.
class Z_Node : public RefCounted {
	GDCLASS(Z_Node, RefCounted)

protected:
	static void _bind_methods();

public:
	// Removes and frees every direct child of p_node.
	static void children_remove_all(Node *p_node);
	// p_node must be an Area2D/Area3D. Returns overlapping bodies whose native
	// class or script class_name (walking the base_script chain) matches p_class.
	static Array get_overlapping_bodies_of_class(Node *p_node, const String &p_class);
};

} // namespace godot
