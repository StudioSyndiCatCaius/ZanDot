#pragma once

#include "flow_node.h"
#include "z_global_script.h"

#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Runs an array of ZGlobalScript resources, then continues on "out".
class ZFlowDoGlobalScriptNode : public ZFlowNode {
	GDCLASS(ZFlowDoGlobalScriptNode, ZFlowNode)

private:
	TypedArray<ZGlobalScript> scripts;

protected:
	static void _bind_methods();
	void define() override;
	String get_caption() const override;
	String get_category() const override;
	Color get_color() const override;

public:
	void set_scripts(const TypedArray<ZGlobalScript> &p_scripts);
	TypedArray<ZGlobalScript> get_scripts() const;

	void on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) override;

	ZFlowDoGlobalScriptNode();
	~ZFlowDoGlobalScriptNode();
};

} // namespace godot
