#pragma once

#include "flow_node.h"

#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Jumps to a ZFlowHubNode with matching hub_id. No outputs (jump replaces outgoing edges).
class ZFlowToHubNode : public ZFlowNode {
	GDCLASS(ZFlowToHubNode, ZFlowNode)

private:
	StringName hub_id;

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;
	PackedStringArray get_graph_exposed_params() const override;

public:
	void set_hub_id(const StringName &p_id);
	StringName get_hub_id() const;

	ZFlowToHubNode();
	~ZFlowToHubNode();
};

} // namespace godot
