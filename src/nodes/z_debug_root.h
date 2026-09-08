#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

// Shared runtime policy for ZDebugRoot* nodes.
// - Editor: left alone so designers can place/debug content.
// - Runtime (editor play / debug): hide descendants and disable collisions.
// - Exported game: queue_free self (and children) unless launched with --z-debug.
class ZDebugRootUtil {
public:
	static constexpr const char *CMD_FLAG = "--z-debug";

	// Returns true if the node should remain in the tree at runtime.
	static bool should_keep_at_runtime();
	// Hide all descendants and disable collision shapes / layers under p_root.
	static void apply_disable(Node *p_root);

private:
	static void _apply_node(Node *p_node);
	static bool _has_cmdline_flag(const char *p_flag);
};

// 2D debug organizational root (Node2D).
class ZDebugRoot2D : public Node2D {
	GDCLASS(ZDebugRoot2D, Node2D)

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	ZDebugRoot2D();
	~ZDebugRoot2D();
};

// 3D debug organizational root (Node3D).
class ZDebugRoot3D : public Node3D {
	GDCLASS(ZDebugRoot3D, Node3D)

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	ZDebugRoot3D();
	~ZDebugRoot3D();
};

// UI debug organizational root (Control).
class ZDebugRootUI : public Control {
	GDCLASS(ZDebugRootUI, Control)

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	ZDebugRootUI();
	~ZDebugRootUI();
};

} // namespace godot
