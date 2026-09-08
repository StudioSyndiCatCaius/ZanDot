#pragma once

#include "flow_node.h"
#include "z_global_condition.h"

#include "../z_general_data.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// One selectable option under a ZFlowNode_ChoiceHUB.
//
// The hub gathers the choice nodes wired to its output, drops the ones whose
// `conditions` fail, and shows the rest in a ZMenu_Choice. Picking a choice
// enters that node, which immediately continues down its own "out".
//
// Exposes get_z_data() so ZDataOption can render `text` with no glue code.
class ZFlowNode_Choice : public ZFlowNode {
	GDCLASS(ZFlowNode_Choice, ZFlowNode)

private:
	String text;
	TypedArray<ZGlobalCondition> conditions;

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;
	PackedStringArray get_graph_exposed_params() const override;

public:
	void set_text(const String &p_text);
	String get_text() const;

	void set_conditions(const TypedArray<ZGlobalCondition> &p_conditions);
	TypedArray<ZGlobalCondition> get_conditions() const;

	// True when every condition passes (or there are none).
	bool is_available() const;

	// Presentational blob for ZDataList / ZDataOption.
	Ref<ZGeneralData> get_z_data() const;

	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	ZFlowNode_Choice();
	~ZFlowNode_Choice();
};

} // namespace godot
