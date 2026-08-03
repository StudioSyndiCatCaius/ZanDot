#pragma once

#include "flow_node.h"

namespace godot {

class FlowEndNode : public FlowNode {
	GDCLASS(FlowEndNode, FlowNode)

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	bool is_end_node() const override { return true; }

	FlowEndNode();
	~FlowEndNode();
};

} // namespace godot
