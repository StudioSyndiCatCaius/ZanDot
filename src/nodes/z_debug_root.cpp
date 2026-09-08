#include "z_debug_root.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/collision_object2d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/collision_polygon2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

using namespace godot;

bool ZDebugRootUtil::_has_cmdline_flag(const char *p_flag) {
	OS *os = OS::get_singleton();
	if (os == nullptr) {
		return false;
	}
	const String flag(p_flag);
	PackedStringArray args = os->get_cmdline_args();
	for (int i = 0; i < args.size(); i++) {
		if (String(args[i]) == flag) {
			return true;
		}
	}
	args = os->get_cmdline_user_args();
	for (int i = 0; i < args.size(); i++) {
		if (String(args[i]) == flag) {
			return true;
		}
	}
	return false;
}

bool ZDebugRootUtil::should_keep_at_runtime() {
	// Exported game: only keep when launched with --z-debug.
	OS *os = OS::get_singleton();
	if (os != nullptr && !os->has_feature("editor")) {
		return _has_cmdline_flag(CMD_FLAG);
	}
	// Editor play / tools: keep (content is then hidden + collision-disabled).
	return true;
}

void ZDebugRootUtil::_apply_node(Node *p_node) {
	if (p_node == nullptr) {
		return;
	}

	if (CanvasItem *ci = Object::cast_to<CanvasItem>(p_node)) {
		ci->set_visible(false);
	}
	if (Node3D *n3 = Object::cast_to<Node3D>(p_node)) {
		n3->set_visible(false);
	}

	if (CollisionShape2D *shape2d = Object::cast_to<CollisionShape2D>(p_node)) {
		shape2d->set_disabled(true);
	}
	if (CollisionPolygon2D *poly2d = Object::cast_to<CollisionPolygon2D>(p_node)) {
		poly2d->set_disabled(true);
	}
	if (CollisionShape3D *shape3d = Object::cast_to<CollisionShape3D>(p_node)) {
		shape3d->set_disabled(true);
	}

	if (CollisionObject2D *co2d = Object::cast_to<CollisionObject2D>(p_node)) {
		co2d->set_collision_layer(0);
		co2d->set_collision_mask(0);
	}
	if (CollisionObject3D *co3d = Object::cast_to<CollisionObject3D>(p_node)) {
		co3d->set_collision_layer(0);
		co3d->set_collision_mask(0);
	}

	const int count = p_node->get_child_count();
	for (int i = 0; i < count; i++) {
		_apply_node(p_node->get_child(i));
	}
}

void ZDebugRootUtil::apply_disable(Node *p_root) {
	if (p_root == nullptr) {
		return;
	}
	const int count = p_root->get_child_count();
	for (int i = 0; i < count; i++) {
		_apply_node(p_root->get_child(i));
	}
}

// --- ZDebugRoot2D ---

void ZDebugRoot2D::_bind_methods() {
}

ZDebugRoot2D::ZDebugRoot2D() {
}

ZDebugRoot2D::~ZDebugRoot2D() {
}

void ZDebugRoot2D::_notification(int p_what) {
	if (p_what != NOTIFICATION_ENTER_TREE) {
		return;
	}
	if (Engine::get_singleton() != nullptr && Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!ZDebugRootUtil::should_keep_at_runtime()) {
		queue_free();
		return;
	}
	ZDebugRootUtil::apply_disable(this);
}

// --- ZDebugRoot3D ---

void ZDebugRoot3D::_bind_methods() {
}

ZDebugRoot3D::ZDebugRoot3D() {
}

ZDebugRoot3D::~ZDebugRoot3D() {
}

void ZDebugRoot3D::_notification(int p_what) {
	if (p_what != NOTIFICATION_ENTER_TREE) {
		return;
	}
	if (Engine::get_singleton() != nullptr && Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!ZDebugRootUtil::should_keep_at_runtime()) {
		queue_free();
		return;
	}
	ZDebugRootUtil::apply_disable(this);
}

// --- ZDebugRootUI ---

void ZDebugRootUI::_bind_methods() {
}

ZDebugRootUI::ZDebugRootUI() {
}

ZDebugRootUI::~ZDebugRootUI() {
}

void ZDebugRootUI::_notification(int p_what) {
	if (p_what != NOTIFICATION_ENTER_TREE) {
		return;
	}
	if (Engine::get_singleton() != nullptr && Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!ZDebugRootUtil::should_keep_at_runtime()) {
		queue_free();
		return;
	}
	ZDebugRootUtil::apply_disable(this);
}
