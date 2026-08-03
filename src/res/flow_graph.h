#pragma once

#include "flow_node.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Flow graph asset (native Godot resource, typically .tres).
// Nodes are nested FlowNode sub-resources so script @export vars serialize automatically.
class FlowGraph : public Resource {
	GDCLASS(FlowGraph, Resource)

private:
	TypedArray<FlowNode> nodes;
	// Each connection: { from_node: String, from_port: int, to_node: String, to_port: int }
	TypedArray<Dictionary> connections;
	Dictionary params;
	int64_t next_id = 1;

protected:
	static void _bind_methods();

public:
	void set_nodes(const TypedArray<FlowNode> &p_nodes);
	TypedArray<FlowNode> get_nodes() const;

	void set_connections(const TypedArray<Dictionary> &p_connections);
	TypedArray<Dictionary> get_connections() const;

	void set_params(const Dictionary &p_params);
	Dictionary get_params() const;

	void set_next_id(int64_t p_next_id);
	int64_t get_next_id() const;

	// Ensure a Start node exists (used for new graphs).
	void ensure_defaults();

	String generate_node_id(const String &p_prefix = "node");

	void add_node(const Ref<FlowNode> &p_node);
	void remove_node(const String &p_node_id);
	Ref<FlowNode> get_node(const String &p_node_id) const;
	Ref<FlowNode> get_start_node() const;
	bool has_node(const String &p_node_id) const;

	void add_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port);
	void remove_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port);
	void clear_connections_for_node(const String &p_node_id);

	// Returns connection dictionaries leaving (from_node, from_port).
	TypedArray<Dictionary> get_connections_from(const String &p_from_node, int32_t p_from_port) const;
	// First matching target node id for (from_node, from_port), or empty.
	String get_next_node_id(const String &p_from_node, int32_t p_from_port) const;

	// Legacy .gdflow (JSON) helpers — used only by FlowGraphLoader for migration.
	Dictionary to_dict() const;
	static Ref<FlowGraph> from_dict(const Dictionary &p_data);

	FlowGraph();
	~FlowGraph();
};

} // namespace godot
