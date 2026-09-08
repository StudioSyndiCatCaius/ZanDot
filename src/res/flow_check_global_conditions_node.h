#pragma once

#include "flow_node.h"
#include "z_global_condition.h"

#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Evaluates ZGlobalCondition list; outputs "true" (port 0) or "false" (port 1).
class ZFlowCheckGlobalConditionsNode : public ZFlowNode {
	GDCLASS(ZFlowCheckGlobalConditionsNode, ZFlowNode)

private:
	TypedArray<ZGlobalCondition> conditions;
	// When true, pass if any condition is true; when false, all must be true.
	bool if_one_true = false;

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	void set_conditions(const TypedArray<ZGlobalCondition> &p_conditions);
	TypedArray<ZGlobalCondition> get_conditions() const;

	void set_if_one_true(bool p_value);
	bool get_if_one_true() const;

	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	ZFlowCheckGlobalConditionsNode();
	~ZFlowCheckGlobalConditionsNode();
};

} // namespace godot
