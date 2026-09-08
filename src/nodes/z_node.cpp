#include "z_node.h"

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
bool _node_matches_class(Node *p_node, const String &p_class) {
	if (p_node == nullptr || p_class.is_empty()) {
		return false;
	}
	if (p_node->is_class(p_class)) {
		return true;
	}
	Ref<Script> script = p_node->get_script();
	while (script.is_valid()) {
		const StringName gn = script->get_global_name();
		if (gn != StringName() && String(gn) == p_class) {
			return true;
		}
		script = script->get_base_script();
	}
	return false;
}
} // namespace

void Z_Node::_bind_methods() {
	ClassDB::bind_static_method("Z_Node", D_METHOD("children_remove_all", "node"), &Z_Node::children_remove_all);
	ClassDB::bind_static_method("Z_Node", D_METHOD("get_overlapping_bodies_of_class", "node", "class_"), &Z_Node::get_overlapping_bodies_of_class);
}

void Z_Node::children_remove_all(Node *p_node) {
	if (p_node == nullptr) {
		return;
	}
	for (int i = p_node->get_child_count() - 1; i >= 0; i--) {
		Node *child = p_node->get_child(i);
		p_node->remove_child(child);
		child->queue_free();
	}
}

Array Z_Node::get_overlapping_bodies_of_class(Node *p_node, const String &p_class) {
	Array result;
	if (p_node == nullptr) {
		return result;
	}

	Area3D *area3d = Object::cast_to<Area3D>(p_node);
	if (area3d != nullptr) {
		TypedArray<Node3D> bodies = area3d->get_overlapping_bodies();
		for (int i = 0; i < bodies.size(); i++) {
			Node *body = Object::cast_to<Node>(bodies[i]);
			if (_node_matches_class(body, p_class)) {
				result.push_back(body);
			}
		}
		return result;
	}

	Area2D *area2d = Object::cast_to<Area2D>(p_node);
	if (area2d != nullptr) {
		TypedArray<Node2D> bodies = area2d->get_overlapping_bodies();
		for (int i = 0; i < bodies.size(); i++) {
			Node *body = Object::cast_to<Node>(bodies[i]);
			if (_node_matches_class(body, p_class)) {
				result.push_back(body);
			}
		}
		return result;
	}

	UtilityFunctions::push_warning("Z_Node.get_overlapping_bodies_of_class: node is not an Area2D/Area3D.");
	return result;
}
