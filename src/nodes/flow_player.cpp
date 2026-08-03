#include "flow_player.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FlowPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_flow_graph", "graph"), &FlowPlayer::set_flow_graph);
	ClassDB::bind_method(D_METHOD("get_flow_graph"), &FlowPlayer::get_flow_graph);

	ClassDB::bind_method(D_METHOD("set_auto_advance", "auto_advance"), &FlowPlayer::set_auto_advance);
	ClassDB::bind_method(D_METHOD("get_auto_advance"), &FlowPlayer::get_auto_advance);

	ClassDB::bind_method(D_METHOD("set_variables", "variables"), &FlowPlayer::set_variables);
	ClassDB::bind_method(D_METHOD("get_variables"), &FlowPlayer::get_variables);

	ClassDB::bind_method(D_METHOD("set_variable", "name", "value"), &FlowPlayer::set_variable);
	ClassDB::bind_method(D_METHOD("get_variable", "name", "default"), &FlowPlayer::get_variable, DEFVAL(Variant()));

	ClassDB::bind_method(D_METHOD("play"), &FlowPlayer::play);
	ClassDB::bind_method(D_METHOD("stop"), &FlowPlayer::stop);
	ClassDB::bind_method(D_METHOD("advance"), &FlowPlayer::advance);

	ClassDB::bind_method(D_METHOD("is_playing"), &FlowPlayer::is_playing);
	ClassDB::bind_method(D_METHOD("is_paused"), &FlowPlayer::is_paused);
	ClassDB::bind_method(D_METHOD("get_current_node"), &FlowPlayer::get_current_node);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "flow_graph", PROPERTY_HINT_RESOURCE_TYPE, "FlowGraph"), "set_flow_graph", "get_flow_graph");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_advance"), "set_auto_advance", "get_auto_advance");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "variables"), "set_variables", "get_variables");

	ADD_SIGNAL(MethodInfo("flow_begin"));
	ADD_SIGNAL(MethodInfo("flow_end"));
	ADD_SIGNAL(MethodInfo("node_enter", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "FlowNode")));
	ADD_SIGNAL(MethodInfo("node_exit", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "FlowNode")));
}

FlowPlayer::FlowPlayer() {
	auto_advance = true;
	playing = false;
	paused = false;
}

FlowPlayer::~FlowPlayer() {
}

void FlowPlayer::set_flow_graph(const Ref<FlowGraph> &p_graph) {
	if (playing) {
		stop();
	}
	flow_graph = p_graph;
}

Ref<FlowGraph> FlowPlayer::get_flow_graph() const {
	return flow_graph;
}

void FlowPlayer::set_auto_advance(bool p_auto) {
	auto_advance = p_auto;
}

bool FlowPlayer::get_auto_advance() const {
	return auto_advance;
}

void FlowPlayer::set_variables(const Dictionary &p_variables) {
	variables = p_variables;
}

Dictionary FlowPlayer::get_variables() const {
	return variables;
}

void FlowPlayer::set_variable(const StringName &p_name, const Variant &p_value) {
	variables[p_name] = p_value;
}

Variant FlowPlayer::get_variable(const StringName &p_name, const Variant &p_default) const {
	if (variables.has(p_name)) {
		return variables[p_name];
	}
	return p_default;
}

void FlowPlayer::play() {
	if (flow_graph.is_null()) {
		UtilityFunctions::push_warning("FlowPlayer: no FlowGraph assigned.");
		return;
	}
	if (playing) {
		stop();
	}

	Ref<FlowNode> start = flow_graph->get_start_node();
	if (start.is_null()) {
		UtilityFunctions::push_warning("FlowPlayer: FlowGraph has no Start node.");
		return;
	}

	playing = true;
	paused = false;
	emit_signal("flow_begin");
	_enter_node(start);
}

void FlowPlayer::stop() {
	if (!playing) {
		current_node.unref();
		paused = false;
		return;
	}
	_leave_current();
	playing = false;
	paused = false;
	current_node.unref();
	if (has_meta("_pending_out_port")) {
		remove_meta("_pending_out_port");
	}
	emit_signal("flow_end");
}

void FlowPlayer::advance() {
	if (!playing || current_node.is_null()) {
		return;
	}
	paused = false;
	_step_from_current();
}

bool FlowPlayer::is_playing() const {
	return playing;
}

bool FlowPlayer::is_paused() const {
	return paused;
}

Ref<FlowNode> FlowPlayer::get_current_node() const {
	return current_node;
}

void FlowPlayer::_enter_node(const Ref<FlowNode> &p_node) {
	current_node = p_node;
	if (current_node.is_null()) {
		_finish();
		return;
	}

	emit_signal("node_enter", current_node);
	current_node->on_enter(this);

	// Terminal nodes end the flow after enter/exit.
	if (current_node->is_end_node()) {
		_leave_current();
		_finish();
		return;
	}

	int64_t out_port = current_node->choose_output(this);
	if (out_port < 0) {
		paused = true;
		return;
	}

	if (!auto_advance) {
		paused = true;
		set_meta("_pending_out_port", out_port);
		return;
	}

	// Auto-advance along the chosen output.
	String from_id = current_node->get_node_id();
	_leave_current();

	String next_id = flow_graph->get_next_node_id(from_id, (int32_t)out_port);
	if (next_id.is_empty()) {
		_finish();
		return;
	}

	_enter_node(flow_graph->get_node(next_id));
}

void FlowPlayer::_leave_current() {
	if (current_node.is_valid()) {
		current_node->on_exit(this);
		emit_signal("node_exit", current_node);
	}
}

void FlowPlayer::_finish() {
	playing = false;
	paused = false;
	current_node.unref();
	if (has_meta("_pending_out_port")) {
		remove_meta("_pending_out_port");
	}
	emit_signal("flow_end");
}

void FlowPlayer::_step_from_current() {
	if (current_node.is_null() || flow_graph.is_null()) {
		_finish();
		return;
	}

	// If we already ran on_enter and paused, don't re-enter — just follow output.
	int64_t out_port = 0;
	if (has_meta("_pending_out_port")) {
		out_port = get_meta("_pending_out_port");
		remove_meta("_pending_out_port");
	} else {
		out_port = current_node->choose_output(this);
		if (out_port < 0) {
			paused = true;
			return;
		}
	}

	if (current_node->is_end_node()) {
		_leave_current();
		_finish();
		return;
	}

	String from_id = current_node->get_node_id();
	_leave_current();

	String next_id = flow_graph->get_next_node_id(from_id, (int32_t)out_port);
	if (next_id.is_empty()) {
		_finish();
		return;
	}

	_enter_node(flow_graph->get_node(next_id));
}
