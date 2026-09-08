#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Forward-declared, not included: res/flow_node.h includes this header so its
// _on_enter / _on_exit virtuals can be typed as ZFlowPlayer instead of Object.
// Including flow_node.h back would close that cycle.
class ZFlowGraph;
class ZFlowNode;

// Plays a ZFlowGraph at runtime. Multiple nodes may be active at once.
// Nodes become active on enter; they leave via stop_node() / trigger_output(..., stop_node=true).
// Recorded nodes are those entered at least once during this playback.
class ZFlowPlayer : public Node {
	GDCLASS(ZFlowPlayer, Node)

private:
	Ref<ZFlowGraph> flow_graph;
	TypedArray<ZFlowNode> active_nodes;
	TypedArray<ZFlowNode> recorded_nodes;
	Dictionary variables;
	bool playing = false;
	// When no nodes remain active (e.g. last node exited without firing a valid output), end flow.
	bool end_flow_on_no_active_nodes = true;
	// Nested activate/deactivate (e.g. trigger_output stop+activate) must not end mid-batch.
	int32_t transition_depth = 0;

	void _record_node(const Ref<ZFlowNode> &p_node);
	void _add_active(const Ref<ZFlowNode> &p_node);
	void _remove_active(const Ref<ZFlowNode> &p_node);
	void _clear_runtime_state();
	void _finish();
	void _maybe_end_on_no_active();
	// After enter: End / ToHub special cases only. Nodes continue via trigger_output themselves.
	void _post_enter(const Ref<ZFlowNode> &p_node);

	friend class ZFlowNode;
	void _begin_transition();
	void _end_transition();

protected:
	static void _bind_methods();

public:
	void set_flow_graph(const Ref<ZFlowGraph> &p_graph);
	Ref<ZFlowGraph> get_flow_graph() const;

	void set_end_flow_on_no_active_nodes(bool p_value);
	bool get_end_flow_on_no_active_nodes() const;

	void set_variables(const Dictionary &p_variables);
	Dictionary get_variables() const;

	void set_variable(const StringName &p_name, const Variant &p_value);
	Variant get_variable(const StringName &p_name, const Variant &p_default = Variant()) const;

	// Start playback from the graph's Start node.
	void play();
	// Stop immediately (emits flow_end if was playing). Deactivates all active nodes.
	void stop();

	bool is_playing() const;

	TypedArray<ZFlowNode> nodes_get_active() const;
	TypedArray<ZFlowNode> nodes_get_recorded() const;

	// Activate a node (enter). Used by play() and by ZFlowNode::trigger_output*.
	void activate_node(const Ref<ZFlowNode> &p_node, int32_t p_pin = 0, const Ref<ZFlowNode> &p_previous = Ref<ZFlowNode>(), int32_t p_previous_out_pin = -1);
	// Deactivate a node (exit). exit_pin is passed to on_exit (-1 if stop_node).
	void deactivate_node(const Ref<ZFlowNode> &p_node, int32_t p_exit_pin = -1);

	ZFlowPlayer();
	~ZFlowPlayer();
};

} // namespace godot
