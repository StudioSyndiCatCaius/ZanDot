#pragma once

#include "flow_node.h"

#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Jump target hub: no flow input; identified by hub_id. Single output continues the flow.
class ZFlowHubNode : public ZFlowNode {
	GDCLASS(ZFlowHubNode, ZFlowNode)

private:
	StringName hub_id;

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	void set_hub_id(const StringName &p_id);
	StringName get_hub_id() const;

	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	ZFlowHubNode();
	~ZFlowHubNode();
};

} // namespace godot
