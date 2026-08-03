#pragma once

#include "flow_node.h"

namespace godot {

// Default concrete pass-through node (one in, one out).
class FlowBasicNode : public FlowNode {
	GDCLASS(FlowBasicNode, FlowNode)

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	FlowBasicNode();
	~FlowBasicNode();
};

} // namespace godot
