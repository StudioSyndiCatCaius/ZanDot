#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ZGameMode;

// Spawnable scene used by ZGameMode (explore, combat, pause, dialogue, …).
class ZGameState : public Node {
	GDCLASS(ZGameState, Node)

private:
	// Last context from ZGameMode::change_state*. Set before enter (and before
	// add_child) so both _ready and _on_state_enter can read it.
	Variant context;

protected:
	static void _bind_methods();

	virtual void on_state_enter(const Variant &p_context);
	virtual void on_state_exit();

	GDVIRTUAL1(_on_state_enter, Variant)
	GDVIRTUAL0(_on_state_exit)

public:
	// Called by ZGameMode when this state becomes / stops being active.
	// p_context is an arbitrary payload passed through from ZGameMode::change_state*.
	void state_enter(const Variant &p_context = Variant());
	void state_exit();

	void set_context(const Variant &p_context);
	Variant get_context() const;

	ZGameMode *get_game_mode() const;

	ZGameState();
	~ZGameState();
};

} // namespace godot
