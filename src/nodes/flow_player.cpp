#include "flow_player.h"

#include "res/flow_graph.h"
#include "res/flow_node.h"
#include "res/flow_to_hub_node.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZFlowPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_flow_graph", "graph"), &ZFlowPlayer::set_flow_graph);
	ClassDB::bind_method(D_METHOD("get_flow_graph"), &ZFlowPlayer::get_flow_graph);

	ClassDB::bind_method(D_METHOD("set_end_flow_on_no_active_nodes", "value"), &ZFlowPlayer::set_end_flow_on_no_active_nodes);
	ClassDB::bind_method(D_METHOD("get_end_flow_on_no_active_nodes"), &ZFlowPlayer::get_end_flow_on_no_active_nodes);

	ClassDB::bind_method(D_METHOD("set_variables", "variables"), &ZFlowPlayer::set_variables);
	ClassDB::bind_method(D_METHOD("get_variables"), &ZFlowPlayer::get_variables);

	ClassDB::bind_method(D_METHOD("set_variable", "name", "value"), &ZFlowPlayer::set_variable);
	ClassDB::bind_method(D_METHOD("get_variable", "name", "default"), &ZFlowPlayer::get_variable, DEFVAL(Variant()));

	ClassDB::bind_method(D_METHOD("play"), &ZFlowPlayer::play);
	ClassDB::bind_method(D_METHOD("stop"), &ZFlowPlayer::stop);

	ClassDB::bind_method(D_METHOD("is_playing"), &ZFlowPlayer::is_playing);

	ClassDB::bind_method(D_METHOD("nodes_get_active"), &ZFlowPlayer::nodes_get_active);
	ClassDB::bind_method(D_METHOD("nodes_get_recorded"), &ZFlowPlayer::nodes_get_recorded);

	ClassDB::bind_method(D_METHOD("activate_node", "node", "pin", "previous", "previous_out_pin"), &ZFlowPlayer::activate_node, DEFVAL(0), DEFVAL(Ref<ZFlowNode>()), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("deactivate_node", "node", "exit_pin"), &ZFlowPlayer::deactivate_node, DEFVAL(-1));

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "flow_graph", PROPERTY_HINT_RESOURCE_TYPE, "ZFlowGraph"), "set_flow_graph", "get_flow_graph");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "end_flow_on_no_active_nodes"), "set_end_flow_on_no_active_nodes", "get_end_flow_on_no_active_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "variables"), "set_variables", "get_variables");

	ADD_SIGNAL(MethodInfo("flow_begin"));
	ADD_SIGNAL(MethodInfo("flow_end"));
	ADD_SIGNAL(MethodInfo("node_enter", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "ZFlowNode")));
	ADD_SIGNAL(MethodInfo("node_exit", PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_RESOURCE_TYPE, "ZFlowNode")));
}

ZFlowPlayer::ZFlowPlayer() {
	end_flow_on_no_active_nodes = true;
	playing = false;
}

ZFlowPlayer::~ZFlowPlayer() {
}

void ZFlowPlayer::set_flow_graph(const Ref<ZFlowGraph> &p_graph) {
	if (playing) {
		stop();
	}
	flow_graph = p_graph;
}

Ref<ZFlowGraph> ZFlowPlayer::get_flow_graph() const {
	return flow_graph;
}

void ZFlowPlayer::set_end_flow_on_no_active_nodes(bool p_value) {
	end_flow_on_no_active_nodes = p_value;
}

bool ZFlowPlayer::get_end_flow_on_no_active_nodes() const {
	return end_flow_on_no_active_nodes;
}

void ZFlowPlayer::set_variables(const Dictionary &p_variables) {
	variables = p_variables;
}

Dictionary ZFlowPlayer::get_variables() const {
	return variables;
}

void ZFlowPlayer::set_variable(const StringName &p_name, const Variant &p_value) {
	variables[p_name] = p_value;
}

Variant ZFlowPlayer::get_variable(const StringName &p_name, const Variant &p_default) const {
	if (variables.has(p_name)) {
		return variables[p_name];
	}
	return p_default;
}

void ZFlowPlayer::_record_node(const Ref<ZFlowNode> &p_node) {
	if (p_node.is_null()) {
		return;
	}
	for (int i = 0; i < recorded_nodes.size(); i++) {
		Ref<ZFlowNode> existing = recorded_nodes[i];
		if (existing == p_node) {
			return;
		}
	}
	recorded_nodes.push_back(p_node);
}

void ZFlowPlayer::_add_active(const Ref<ZFlowNode> &p_node) {
	if (p_node.is_null()) {
		return;
	}
	for (int i = 0; i < active_nodes.size(); i++) {
		Ref<ZFlowNode> existing = active_nodes[i];
		if (existing == p_node) {
			return;
		}
	}
	active_nodes.push_back(p_node);
}

void ZFlowPlayer::_remove_active(const Ref<ZFlowNode> &p_node) {
	if (p_node.is_null()) {
		return;
	}
	for (int i = active_nodes.size() - 1; i >= 0; i--) {
		Ref<ZFlowNode> existing = active_nodes[i];
		if (existing == p_node) {
			active_nodes.remove_at(i);
		}
	}
}

void ZFlowPlayer::_clear_runtime_state() {
	for (int i = 0; i < active_nodes.size(); i++) {
		Ref<ZFlowNode> n = active_nodes[i];
		if (n.is_valid()) {
			n->_set_active_flag(false);
			n->_set_runtime_player(nullptr);
		}
	}
	active_nodes.clear();
	recorded_nodes.clear();
}

void ZFlowPlayer::play() {
	if (flow_graph.is_null()) {
		UtilityFunctions::push_warning("ZFlowPlayer: no ZFlowGraph assigned.");
		return;
	}
	if (playing) {
		stop();
	}

	Ref<ZFlowNode> start = flow_graph->get_start_node();
	if (start.is_null()) {
		UtilityFunctions::push_warning("ZFlowPlayer: ZFlowGraph has no Start node.");
		return;
	}

	playing = true;
	active_nodes.clear();
	recorded_nodes.clear();
	emit_signal("flow_begin");
	activate_node(start, -1, Ref<ZFlowNode>(), -1);
}

void ZFlowPlayer::stop() {
	if (!playing) {
		_clear_runtime_state();
		return;
	}

	// Suppress end-on-empty while we force-exit every active node.
	_begin_transition();
	TypedArray<ZFlowNode> snapshot = active_nodes;
	for (int i = 0; i < snapshot.size(); i++) {
		Ref<ZFlowNode> n = snapshot[i];
		if (n.is_valid() && n->is_active()) {
			n->on_exit(-1, this);
			emit_signal("node_exit", n);
			n->_set_active_flag(false);
			n->_set_runtime_player(nullptr);
		}
	}
	transition_depth = 0;
	playing = false;
	_clear_runtime_state();
	emit_signal("flow_end");
}

bool ZFlowPlayer::is_playing() const {
	return playing;
}

TypedArray<ZFlowNode> ZFlowPlayer::nodes_get_active() const {
	return active_nodes;
}

TypedArray<ZFlowNode> ZFlowPlayer::nodes_get_recorded() const {
	return recorded_nodes;
}

void ZFlowPlayer::activate_node(const Ref<ZFlowNode> &p_node, int32_t p_pin, const Ref<ZFlowNode> &p_previous, int32_t p_previous_out_pin) {
	if (!playing || p_node.is_null()) {
		if (p_node.is_null() && playing) {
			_maybe_end_on_no_active();
		}
		return;
	}

	if (flow_graph.is_valid() && p_node->get_owning_graph() == nullptr) {
		p_node->set_owning_graph(flow_graph.ptr());
	}

	_record_node(p_node);
	_add_active(p_node);
	p_node->_set_runtime_player(this);
	p_node->_set_active_flag(true);

	emit_signal("node_enter", p_node);
	ZFlowNode *prev_ptr = p_previous.is_valid() ? p_previous.ptr() : nullptr;
	p_node->on_enter(p_pin, this, prev_ptr, p_previous_out_pin);

	// Node may have already stop_node()'d or triggered during on_enter.
	if (!playing) {
		return;
	}
	if (!p_node->is_active()) {
		_maybe_end_on_no_active();
		return;
	}

	_post_enter(p_node);
}

void ZFlowPlayer::_begin_transition() {
	transition_depth++;
}

void ZFlowPlayer::_end_transition() {
	if (transition_depth > 0) {
		transition_depth--;
	}
	if (transition_depth == 0 && playing) {
		_maybe_end_on_no_active();
	}
}

void ZFlowPlayer::deactivate_node(const Ref<ZFlowNode> &p_node, int32_t p_exit_pin) {
	if (p_node.is_null() || !p_node->is_active()) {
		return;
	}

	p_node->on_exit(p_exit_pin, this);
	emit_signal("node_exit", p_node);
	p_node->_set_active_flag(false);
	p_node->_set_runtime_player(nullptr);
	_remove_active(p_node);

	if (playing && transition_depth == 0) {
		_maybe_end_on_no_active();
	}
}

void ZFlowPlayer::_post_enter(const Ref<ZFlowNode> &p_node) {
	if (!playing || p_node.is_null() || !p_node->is_active()) {
		return;
	}

	// Terminal nodes end the entire flow after enter.
	if (p_node->is_end_node()) {
		deactivate_node(p_node, -1);
		if (playing) {
			playing = false;
			TypedArray<ZFlowNode> snapshot = active_nodes;
			for (int i = 0; i < snapshot.size(); i++) {
				Ref<ZFlowNode> n = snapshot[i];
				if (n.is_valid() && n->is_active()) {
					n->on_exit(-1, this);
					emit_signal("node_exit", n);
					n->_set_active_flag(false);
					n->_set_runtime_player(nullptr);
				}
			}
			_clear_runtime_state();
			emit_signal("flow_end");
		}
		return;
	}

	// ToHub: jump to matching Hub node.
	{
		Ref<ZFlowToHubNode> to_hub = p_node;
		if (to_hub.is_valid() && flow_graph.is_valid()) {
			Ref<ZFlowNode> hub = flow_graph->find_hub_node(to_hub->get_hub_id());
			deactivate_node(p_node, -1);
			if (hub.is_null()) {
				UtilityFunctions::push_warning("ZFlowPlayer: ToHub target not found: " + String(to_hub->get_hub_id()));
				_maybe_end_on_no_active();
				return;
			}
			activate_node(hub, 0, p_node, -1);
			return;
		}
	}

	// All other continuation is owned by the node (trigger_output from on_enter / external UI).
}

void ZFlowPlayer::_maybe_end_on_no_active() {
	if (!playing || !end_flow_on_no_active_nodes) {
		return;
	}
	if (active_nodes.is_empty()) {
		_finish();
	}
}

void ZFlowPlayer::_finish() {
	if (!playing) {
		return;
	}
	playing = false;
	_clear_runtime_state();
	emit_signal("flow_end");
}
