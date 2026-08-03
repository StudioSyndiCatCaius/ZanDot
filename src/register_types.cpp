#include "register_types.h"

#include "z_resource.h"
#include "nodes/flow_player.h"
#include "nodes/z.h"
#include "nodes/z_ability.h"
#include "nodes/z_ability_manager.h"
#include "nodes/z_game_mode.h"
#include "nodes/z_game_state.h"
#include "nodes/z_input_listener.h"
#include "nodes/z_mover.h"
#include "nodes/z_player_start_3d.h"
#include "res/flow_basic_node.h"
#include "res/flow_end_node.h"
#include "res/flow_graph.h"
#include "res/flow_graph_loader.h"
#include "res/flow_node.h"
#include "res/flow_start_node.h"
#include "res/z_move_mode.h"
#include "res/z_player.h"
#include "res/z_save.h"
#include "res/z_save_game.h"
#include "res/z_save_global.h"

#ifdef DEBUG_ENABLED
#include "ed/flow_editor_plugin.h"
#include "ed/z_resource_preview_generator.h"
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#endif

#include <gdextension_interface.h>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

// Legacy .gdflow (JSON) loader only — new assets use Godot's built-in .tres serializer.
static Ref<FlowGraphLoader> flow_graph_loader;

static void _z_bootstrap_deferred() {
	if (Z::get_singleton() != nullptr) {
		Z::get_singleton()->bootstrap();
	}
}

void initialize_zandot_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// GDREGISTER_CLASS so types are available in the editor Create dialog.
		GDREGISTER_CLASS(ZResource);

		// FlowGraph core.
		// FlowNode must remain instantiable so GDScript `class_name X extends FlowNode`
		// can construct (engine instantiates the native base, then attaches the script).
		// It is excluded from the add-node palette; use FlowBasicNode / custom subclasses.
		GDREGISTER_CLASS(FlowNode);
		GDREGISTER_CLASS(FlowBasicNode);
		GDREGISTER_CLASS(FlowStartNode);
		GDREGISTER_CLASS(FlowEndNode);
		GDREGISTER_CLASS(FlowGraph);
		GDREGISTER_CLASS(FlowPlayer);

		// Gameplay Framework resources.
		GDREGISTER_CLASS(ZSave);
		GDREGISTER_CLASS(ZSaveGame);
		GDREGISTER_CLASS(ZSaveGlobal);
		GDREGISTER_CLASS(ZPlayer);
		GDREGISTER_CLASS(ZMoveMode);

		// Gameplay Framework nodes.
		GDREGISTER_CLASS(ZGameState);
		GDREGISTER_CLASS(ZGameMode);
		GDREGISTER_CLASS(ZPlayerStart3D);
		GDREGISTER_CLASS(ZInputListener);
		GDREGISTER_CLASS(ZMover);
		GDREGISTER_CLASS(ZAbility);
		GDREGISTER_CLASS(ZAbilityManager);
		GDREGISTER_CLASS(Z);

		// ProjectSettings: Project → Project Settings → ZanDot (zandot/*).
		Z::register_settings();
		Z::create_singleton();

		// Bootstrap when the main loop exists (deferred — tree may not be ready yet).
		callable_mp_static(_z_bootstrap_deferred).call_deferred();

		// Read-only migration for older JSON .gdflow files. Saves go through Godot .tres/.res.
		GDREGISTER_CLASS(FlowGraphLoader);
		flow_graph_loader.instantiate();
		ResourceLoader::get_singleton()->add_resource_format_loader(flow_graph_loader);
	}

#ifdef DEBUG_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_INTERNAL_CLASS(FlowPaletteList);
		GDREGISTER_INTERNAL_CLASS(FlowGraphCanvas);
		GDREGISTER_INTERNAL_CLASS(ZResourcePreviewGenerator);
		GDREGISTER_CLASS(FlowEditorPlugin);
		EditorPlugins::add_by_type<FlowEditorPlugin>();
	}
#endif
}

void uninitialize_zandot_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (flow_graph_loader.is_valid()) {
			ResourceLoader::get_singleton()->remove_resource_format_loader(flow_graph_loader);
			flow_graph_loader.unref();
		}
		Z::destroy_singleton();
	}

#ifdef DEBUG_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// EditorPlugins cleanup is handled by godot-cpp deinitialize.
	}
#endif
}

extern "C" {
// Initialization entry point referenced by zandot.gdextension
GDExtensionBool GDE_EXPORT zandot_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_zandot_module);
	init_obj.register_terminator(uninitialize_zandot_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
