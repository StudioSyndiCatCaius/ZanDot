#pragma once

#include "flow_node.h"

namespace godot {

// Default concrete pass-through node (one in, one out).
class FlowBasicNode : public ZFlowNode {
	GDCLASS(FlowBasicNode, ZFlowNode)

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	FlowBasicNode();
	~FlowBasicNode();
};

} // namespace godot
