#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {

class ZGameMode;

// Spawnable scene used by ZGameMode (explore, combat, pause, dialogue, …).
class ZGameState : public Node {
	GDCLASS(ZGameState, Node)

protected:
	static void _bind_methods();

	virtual void on_state_enter();
	virtual void on_state_exit();

	GDVIRTUAL0(_on_state_enter)
	GDVIRTUAL0(_on_state_exit)

public:
	// Called by ZGameMode when this state becomes / stops being active.
	void state_enter();
	void state_exit();

	ZGameMode *get_game_mode() const;

	ZGameState();
	~ZGameState();
};

} // namespace godot
