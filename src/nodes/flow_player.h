#pragma once

#include "res/flow_graph.h"
#include "res/flow_node.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

// Plays a FlowGraph at runtime: walks from Start through connections to End.
class FlowPlayer : public Node {
	GDCLASS(FlowPlayer, Node)

private:
	Ref<FlowGraph> flow_graph;
	Ref<FlowNode> current_node;
	Dictionary variables;
	bool playing = false;
	bool paused = false;
	bool auto_advance = true;

	void _enter_node(const Ref<FlowNode> &p_node);
	void _leave_current();
	void _finish();
	void _step_from_current();

protected:
	static void _bind_methods();

public:
	void set_flow_graph(const Ref<FlowGraph> &p_graph);
	Ref<FlowGraph> get_flow_graph() const;

	void set_auto_advance(bool p_auto);
	bool get_auto_advance() const;

	void set_variables(const Dictionary &p_variables);
	Dictionary get_variables() const;

	void set_variable(const StringName &p_name, const Variant &p_value);
	Variant get_variable(const StringName &p_name, const Variant &p_default = Variant()) const;

	// Start playback from the graph's Start node.
	void play();
	// Stop immediately (emits flow_end if was playing).
	void stop();
	// Resume after a node returned choose_output == -1, or when auto_advance is false.
	void advance();

	bool is_playing() const;
	bool is_paused() const;
	Ref<FlowNode> get_current_node() const;

	FlowPlayer();
	~FlowPlayer();
};

} // namespace godot
