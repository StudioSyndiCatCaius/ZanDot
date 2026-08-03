#include "flow_graph.h"

#include "flow_basic_node.h"
#include "flow_end_node.h"
#include "flow_start_node.h"

#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

Vector2 _variant_to_vector2(const Variant &p_value) {
	switch (p_value.get_type()) {
		case Variant::VECTOR2:
			return p_value;
		case Variant::ARRAY: {
			Array a = p_value;
			if (a.size() >= 2) {
				return Vector2(a[0], a[1]);
			}
			break;
		}
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return Vector2(d.get("x", 0.0), d.get("y", 0.0));
		}
		case Variant::STRING: {
			// Legacy JSON stringify of Vector2: "(x, y)"
			String s = p_value;
			s = s.strip_edges();
			if (s.begins_with("(") && s.ends_with(")")) {
				s = s.substr(1, s.length() - 2);
			}
			PackedStringArray parts = s.split(",");
			if (parts.size() >= 2) {
				return Vector2(parts[0].strip_edges().to_float(), parts[1].strip_edges().to_float());
			}
			break;
		}
		default:
			break;
	}
	return Vector2();
}

PackedStringArray _variant_to_string_array(const Variant &p_value) {
	if (p_value.get_type() == Variant::PACKED_STRING_ARRAY) {
		return p_value;
	}
	PackedStringArray out;
	if (p_value.get_type() == Variant::ARRAY) {
		Array a = p_value;
		for (int i = 0; i < a.size(); i++) {
			out.push_back(a[i]);
		}
	}
	return out;
}

Array _vector2_to_json(const Vector2 &p_pos) {
	Array a;
	a.push_back(p_pos.x);
	a.push_back(p_pos.y);
	return a;
}

Array _string_array_to_json(const PackedStringArray &p_arr) {
	Array a;
	for (int i = 0; i < p_arr.size(); i++) {
		a.push_back(p_arr[i]);
	}
	return a;
}

} // namespace

void FlowGraph::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_nodes", "nodes"), &FlowGraph::set_nodes);
	ClassDB::bind_method(D_METHOD("get_nodes"), &FlowGraph::get_nodes);

	ClassDB::bind_method(D_METHOD("set_connections", "connections"), &FlowGraph::set_connections);
	ClassDB::bind_method(D_METHOD("get_connections"), &FlowGraph::get_connections);

	ClassDB::bind_method(D_METHOD("set_params", "params"), &FlowGraph::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &FlowGraph::get_params);

	ClassDB::bind_method(D_METHOD("set_next_id", "next_id"), &FlowGraph::set_next_id);
	ClassDB::bind_method(D_METHOD("get_next_id"), &FlowGraph::get_next_id);

	ClassDB::bind_method(D_METHOD("ensure_defaults"), &FlowGraph::ensure_defaults);
	ClassDB::bind_method(D_METHOD("generate_node_id", "prefix"), &FlowGraph::generate_node_id, DEFVAL("node"));

	ClassDB::bind_method(D_METHOD("add_node", "node"), &FlowGraph::add_node);
	ClassDB::bind_method(D_METHOD("remove_node", "node_id"), &FlowGraph::remove_node);
	ClassDB::bind_method(D_METHOD("get_node", "node_id"), &FlowGraph::get_node);
	ClassDB::bind_method(D_METHOD("get_start_node"), &FlowGraph::get_start_node);
	ClassDB::bind_method(D_METHOD("has_node", "node_id"), &FlowGraph::has_node);

	ClassDB::bind_method(D_METHOD("add_connection", "from_node", "from_port", "to_node", "to_port"), &FlowGraph::add_connection);
	ClassDB::bind_method(D_METHOD("remove_connection", "from_node", "from_port", "to_node", "to_port"), &FlowGraph::remove_connection);
	ClassDB::bind_method(D_METHOD("clear_connections_for_node", "node_id"), &FlowGraph::clear_connections_for_node);
	ClassDB::bind_method(D_METHOD("get_connections_from", "from_node", "from_port"), &FlowGraph::get_connections_from);
	ClassDB::bind_method(D_METHOD("get_next_node_id", "from_node", "from_port"), &FlowGraph::get_next_node_id);

	ClassDB::bind_method(D_METHOD("to_dict"), &FlowGraph::to_dict);
	ClassDB::bind_static_method("FlowGraph", D_METHOD("from_dict", "data"), &FlowGraph::from_dict);

	// Topology is edited in the Flow main screen — hidden in inspector but stored in .tres.
	// Hint ensures nested FlowNode resources (incl. script @exports) serialize as sub-resources.
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "nodes", PROPERTY_HINT_ARRAY_TYPE, "FlowNode", PROPERTY_USAGE_NO_EDITOR), "set_nodes", "get_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "connections", PROPERTY_HINT_ARRAY_TYPE, "Dictionary", PROPERTY_USAGE_NO_EDITOR), "set_connections", "get_connections");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "next_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_next_id", "get_next_id");
	// User-facing graph params stay visible.
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "params"), "set_params", "get_params");
}

FlowGraph::FlowGraph() {
	next_id = 1;
}

FlowGraph::~FlowGraph() {
}

void FlowGraph::set_nodes(const TypedArray<FlowNode> &p_nodes) {
	nodes = p_nodes;
	emit_changed();
}

TypedArray<FlowNode> FlowGraph::get_nodes() const {
	return nodes;
}

void FlowGraph::set_connections(const TypedArray<Dictionary> &p_connections) {
	connections = p_connections;
	emit_changed();
}

TypedArray<Dictionary> FlowGraph::get_connections() const {
	return connections;
}

void FlowGraph::set_params(const Dictionary &p_params) {
	params = p_params;
	emit_changed();
}

Dictionary FlowGraph::get_params() const {
	return params;
}

void FlowGraph::set_next_id(int64_t p_next_id) {
	next_id = p_next_id;
}

int64_t FlowGraph::get_next_id() const {
	return next_id;
}

void FlowGraph::ensure_defaults() {
	if (get_start_node().is_valid()) {
		return;
	}
	Ref<FlowStartNode> start;
	start.instantiate();
	start->set_node_id("start");
	start->set_position(Vector2(120, 120));
	add_node(start);
}

String FlowGraph::generate_node_id(const String &p_prefix) {
	String id;
	do {
		id = p_prefix + String("_") + String::num_int64(next_id++);
	} while (has_node(id));
	return id;
}

void FlowGraph::add_node(const Ref<FlowNode> &p_node) {
	ERR_FAIL_COND(p_node.is_null());
	if (p_node->get_node_id().is_empty()) {
		p_node->set_node_id(generate_node_id("node"));
	}
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> existing = nodes[i];
		if (existing.is_valid() && existing->get_node_id() == p_node->get_node_id()) {
			nodes[i] = p_node;
			emit_changed();
			return;
		}
	}
	nodes.push_back(p_node);
	emit_changed();
}

void FlowGraph::remove_node(const String &p_node_id) {
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> existing = nodes[i];
		if (existing.is_valid() && existing->get_node_id() == p_node_id) {
			nodes.remove_at(i);
			clear_connections_for_node(p_node_id);
			emit_changed();
			return;
		}
	}
}

Ref<FlowNode> FlowGraph::get_node(const String &p_node_id) const {
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> existing = nodes[i];
		if (existing.is_valid() && existing->get_node_id() == p_node_id) {
			return existing;
		}
	}
	return Ref<FlowNode>();
}

Ref<FlowNode> FlowGraph::get_start_node() const {
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> existing = nodes[i];
		if (existing.is_valid() && existing->is_start_node()) {
			return existing;
		}
	}
	return Ref<FlowNode>();
}

bool FlowGraph::has_node(const String &p_node_id) const {
	return get_node(p_node_id).is_valid();
}

void FlowGraph::add_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port) {
	for (int i = 0; i < connections.size(); i++) {
		Dictionary c = connections[i];
		if (String(c.get("from_node", "")) == p_from_node &&
				int32_t(c.get("from_port", -1)) == p_from_port &&
				String(c.get("to_node", "")) == p_to_node &&
				int32_t(c.get("to_port", -1)) == p_to_port) {
			return;
		}
	}
	Dictionary c;
	c["from_node"] = p_from_node;
	c["from_port"] = p_from_port;
	c["to_node"] = p_to_node;
	c["to_port"] = p_to_port;
	connections.push_back(c);
	emit_changed();
}

void FlowGraph::remove_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port) {
	for (int i = connections.size() - 1; i >= 0; i--) {
		Dictionary c = connections[i];
		if (String(c.get("from_node", "")) == p_from_node &&
				int32_t(c.get("from_port", -1)) == p_from_port &&
				String(c.get("to_node", "")) == p_to_node &&
				int32_t(c.get("to_port", -1)) == p_to_port) {
			connections.remove_at(i);
			emit_changed();
			return;
		}
	}
}

void FlowGraph::clear_connections_for_node(const String &p_node_id) {
	bool changed = false;
	for (int i = connections.size() - 1; i >= 0; i--) {
		Dictionary c = connections[i];
		if (String(c.get("from_node", "")) == p_node_id || String(c.get("to_node", "")) == p_node_id) {
			connections.remove_at(i);
			changed = true;
		}
	}
	if (changed) {
		emit_changed();
	}
}

TypedArray<Dictionary> FlowGraph::get_connections_from(const String &p_from_node, int32_t p_from_port) const {
	TypedArray<Dictionary> result;
	for (int i = 0; i < connections.size(); i++) {
		Dictionary c = connections[i];
		if (String(c.get("from_node", "")) == p_from_node && int32_t(c.get("from_port", -1)) == p_from_port) {
			result.push_back(c);
		}
	}
	return result;
}

String FlowGraph::get_next_node_id(const String &p_from_node, int32_t p_from_port) const {
	TypedArray<Dictionary> list = get_connections_from(p_from_node, p_from_port);
	if (list.is_empty()) {
		return String();
	}
	Dictionary c = list[0];
	return c.get("to_node", "");
}

Dictionary FlowGraph::to_dict() const {
	Dictionary root;
	root["format"] = 2;
	root["params"] = params;
	root["next_id"] = next_id;

	Array node_array;
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> node = nodes[i];
		if (node.is_null()) {
			continue;
		}
		Dictionary nd;
		// Prefer script global class name so reloads stay typed (get_class() is always FlowNode for scripts).
		nd["class"] = node->get_type_name();
		nd["node_id"] = node->get_node_id();
		nd["display_name"] = node->get_display_name();
		// Explicit array so JSON round-trips (Vector2 becomes a string otherwise).
		nd["position"] = _vector2_to_json(node->get_position());
		nd["inputs"] = _string_array_to_json(node->get_inputs());
		nd["outputs"] = _string_array_to_json(node->get_outputs());
		nd["params"] = node->get_params();
		Variant script_var = node->get_script();
		if (script_var.get_type() == Variant::OBJECT) {
			Ref<Script> script = script_var;
			if (script.is_valid() && !script->get_path().is_empty()) {
				nd["script"] = script->get_path();
			}
		}
		node_array.push_back(nd);
	}
	root["nodes"] = node_array;

	Array conn_array;
	for (int i = 0; i < connections.size(); i++) {
		conn_array.push_back(connections[i]);
	}
	root["connections"] = conn_array;
	return root;
}

Ref<FlowGraph> FlowGraph::from_dict(const Dictionary &p_data) {
	Ref<FlowGraph> graph;
	graph.instantiate();

	graph->set_params(p_data.get("params", Dictionary()));
	graph->set_next_id((int64_t)p_data.get("next_id", 1));

	Array node_array = p_data.get("nodes", Array());
	TypedArray<FlowNode> loaded_nodes;
	for (int i = 0; i < node_array.size(); i++) {
		Dictionary nd = node_array[i];
		String class_name = nd.get("class", "FlowBasicNode");
		String script_path = nd.get("script", "");

		Ref<FlowNode> node;

		// Prefer script attachment path for GDScript / class_name types.
		if (!script_path.is_empty() && ResourceLoader::get_singleton()->exists(script_path)) {
			Ref<Resource> script_res = ResourceLoader::get_singleton()->load(script_path, "Script");
			Ref<Script> script = script_res;
			if (script.is_valid()) {
				node.instantiate();
				node->set_script(script);
			}
		}

		// Native ClassDB types (Start/End/Basic/C++ subclasses) without a script path.
		if (node.is_null() && !class_name.is_empty() && class_name != "FlowNode" && ClassDB::can_instantiate(class_name)) {
			Variant inst = ClassDB::instantiate(class_name);
			Object *obj = inst;
			FlowNode *flow_node = Object::cast_to<FlowNode>(obj);
			if (flow_node) {
				node = Ref<FlowNode>(flow_node);
			}
		}

		if (node.is_null()) {
			if (class_name != "FlowBasicNode" && class_name != "FlowNode") {
				UtilityFunctions::push_warning("FlowGraph: could not instantiate '", class_name, "', using FlowBasicNode.");
			}
			Ref<FlowBasicNode> basic;
			basic.instantiate();
			node = basic;
		}

		// Ports from type definition (script/C++ _define), not stale serialized lists.
		node->rebuild_ports();

		node->set_node_id(nd.get("node_id", ""));
		node->set_display_name(nd.get("display_name", ""));
		node->set_position(_variant_to_vector2(nd.get("position", Array())));
		node->set_params(nd.get("params", Dictionary()));

		// If define left ports empty but file has ports (legacy), restore them.
		if (node->get_inputs().is_empty() && node->get_outputs().is_empty()) {
			node->set_inputs(_variant_to_string_array(nd.get("inputs", Array())));
			node->set_outputs(_variant_to_string_array(nd.get("outputs", Array())));
		}

		loaded_nodes.push_back(node);
	}
	graph->set_nodes(loaded_nodes);

	Array conn_array = p_data.get("connections", Array());
	TypedArray<Dictionary> loaded_conns;
	for (int i = 0; i < conn_array.size(); i++) {
		loaded_conns.push_back(conn_array[i]);
	}
	graph->set_connections(loaded_conns);

	return graph;
}
