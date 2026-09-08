#pragma once

#include "flow_node.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Flow graph asset (native Godot resource, typically .tres).
// Nodes are nested ZFlowNode sub-resources so script @export vars serialize automatically.
// Node string ids (node_id) are unique within a graph; never two nodes share the same id.
class ZFlowGraph : public Resource {
	GDCLASS(ZFlowGraph, Resource)

private:
	TypedArray<ZFlowNode> nodes;
	// Each connection: { from_node: String, from_port: int, to_node: String, to_port: int }
	TypedArray<Dictionary> connections;
	Dictionary params;
	int64_t next_id = 1;

	void _bind_node_ownership(const Ref<ZFlowNode> &p_node);
	void _unbind_node_ownership(const Ref<ZFlowNode> &p_node);
	void _rebind_all_ownership();
	// Remove later duplicates so each node_id appears once (first wins).
	void _ensure_unique_node_ids();

protected:
	static void _bind_methods();

public:
	void set_nodes(const TypedArray<ZFlowNode> &p_nodes);
	TypedArray<ZFlowNode> get_nodes() const;

	void set_connections(const TypedArray<Dictionary> &p_connections);
	TypedArray<Dictionary> get_connections() const;

	void set_params(const Dictionary &p_params);
	Dictionary get_params() const;

	void set_next_id(int64_t p_next_id);
	int64_t get_next_id() const;

	// Ensure a Start node exists (used for new graphs).
	void ensure_defaults();

	String generate_node_id(const String &p_prefix = "node");

	void add_node(const Ref<ZFlowNode> &p_node);
	void remove_node(const String &p_node_id);
	Ref<ZFlowNode> get_node(const String &p_node_id) const;
	Ref<ZFlowNode> get_start_node() const;
	// First hub node whose hub_id matches, or null.
	Ref<ZFlowNode> find_hub_node(const StringName &p_hub_id) const;
	bool has_node(const String &p_node_id) const;

	// Runtime Object instance ids (unique per live Object; like RID-as-int).
	TypedArray<ZFlowNode> get_nodes_from_ids(const PackedInt64Array &p_ids) const;
	PackedInt64Array get_ids_from_nodes(const TypedArray<ZFlowNode> &p_nodes) const;

	void add_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port);
	void remove_connection(const String &p_from_node, int32_t p_from_port, const String &p_to_node, int32_t p_to_port);
	void clear_connections_for_node(const String &p_node_id);

	// Returns connection dictionaries leaving (from_node, from_port).
	TypedArray<Dictionary> get_connections_from(const String &p_from_node, int32_t p_from_port) const;
	// Connections arriving at (to_node, to_port). to_port < 0 = all inputs on that node.
	TypedArray<Dictionary> get_connections_to(const String &p_to_node, int32_t p_to_port = -1) const;
	// First matching target node id for (from_node, from_port), or empty.
	String get_next_node_id(const String &p_from_node, int32_t p_from_port) const;

	// Legacy .gdflow (JSON) helpers — used only by FlowGraphLoader for migration.
	Dictionary to_dict() const;
	static Ref<ZFlowGraph> from_dict(const Dictionary &p_data);

	ZFlowGraph();
	~ZFlowGraph();
};

} // namespace godot
