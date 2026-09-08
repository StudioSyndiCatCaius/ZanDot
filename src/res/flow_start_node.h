#pragma once

#include "flow_node.h"

namespace godot {

class FlowStartNode : public ZFlowNode {
	GDCLASS(FlowStartNode, ZFlowNode)

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	bool is_start_node() const override { return true; }
	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	FlowStartNode();
	~FlowStartNode();
};

} // namespace godot
