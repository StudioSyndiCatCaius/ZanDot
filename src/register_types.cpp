#include "register_types.h"

#include "z_general_data.h"
#include "z_input_defaults.h"
#include "z_resource.h"
#include "nodes/flow_player.h"
#include "nodes/z.h"
#include "nodes/z_ability.h"
#include "nodes/z_bgm.h"
#include "nodes/z_bgm_player.h"
#include "nodes/z_camera_follower_3d.h"
#include "nodes/z_canvas_3d.h"
#include "nodes/z_creature.h"
#include "nodes/z_data_list.h"
#include "nodes/z_data_option.h"
#include "nodes/z_debug_root.h"
#include "nodes/z_event.h"
#include "nodes/z_game_mode.h"
#include "nodes/z_game_state.h"
#include "nodes/z_input_common.h"
#include "nodes/z_input_listener.h"
#include "nodes/z_lagger_3d.h"
#include "nodes/z_math.h"
#include "nodes/z_menu.h"
#include "nodes/z_menu_choice.h"
#include "nodes/z_menu_confirm.h"
#include "nodes/z_menu_info.h"
#include "nodes/z_menu_process.h"
#include "nodes/z_mover.h"
#include "nodes/z_node.h"
#include "nodes/z_obj.h"
#include "nodes/z_parse.h"
#include "nodes/z_player_start_3d.h"
#include "nodes/z_sound.h"
#include "nodes/z_world_query_2d.h"
#include "nodes/z_world_query_3d.h"
#include "nodes/z_world_query_debug_canvas2d.h"
#include "nodes/z_master_scene.h"
#include "nodes/z_scene_transit.h"
#include "nodes/z_skin_model.h"
#include "res/flow_basic_node.h"
#include "res/flow_check_global_conditions_node.h"
#include "res/flow_do_global_script_node.h"
#include "res/flow_end_node.h"
#include "res/flow_graph.h"
#include "res/flow_graph_loader.h"
#include "res/flow_hub_node.h"
#include "res/flow_node.h"
#include "res/flow_node_choice.h"
#include "res/flow_node_choice_hub.h"
#include "res/flow_start_node.h"
#include "res/flow_to_hub_node.h"
#include "res/z_attribute.h"
#include "res/z_creature_data.h"
#include "res/z_leveler.h"
#include "res/z_global_condition.h"
#include "res/z_global_script.h"
#include "res/world_query_result.h"
#include "res/world_query_shape.h"
#include "res/world_query_shape_donut.h"
#include "res/world_query_shape_grid.h"
#include "res/world_query_utility.h"
#include "res/world_query_utility_angle.h"
#include "res/world_query_utility_distance.h"
#include "res/world_query_utility_trace.h"
#include "res/z_move_mode.h"
#include "res/z_player.h"
#include "res/z_save.h"
#include "res/z_save_game.h"
#include "res/z_bgm_audio.h"
#include "res/z_bgm_track.h"
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
		// ZGeneralData first — ZResource.get_z_data() returns it.
		GDREGISTER_CLASS(ZGeneralData);
		GDREGISTER_CLASS(ZResource);

		// ZFlowGraph core.
		// ZFlowNode must remain instantiable so GDScript `class_name X extends ZFlowNode`
		// can construct (engine instantiates the native base, then attaches the script).
		// It is excluded from the add-node palette; use FlowBasicNode / custom subclasses.
		GDREGISTER_CLASS(ZFlowNode);
		GDREGISTER_CLASS(FlowBasicNode);
		GDREGISTER_CLASS(FlowStartNode);
		GDREGISTER_CLASS(FlowEndNode);
		GDREGISTER_CLASS(ZFlowHubNode);
		GDREGISTER_CLASS(ZFlowToHubNode);
		GDREGISTER_CLASS(ZFlowCheckGlobalConditionsNode);
		GDREGISTER_CLASS(ZFlowDoGlobalScriptNode);
		GDREGISTER_CLASS(ZFlowNode_Choice);
		GDREGISTER_CLASS(ZFlowNode_ChoiceHUB);
		GDREGISTER_CLASS(ZFlowGraph);
		GDREGISTER_CLASS(ZFlowPlayer);

		// Gameplay Framework resources.
		GDREGISTER_CLASS(ZGlobalCondition);
		GDREGISTER_CLASS(ZGlobalScript);
		GDREGISTER_CLASS(ZBgmAudio);
		GDREGISTER_CLASS(ZBgmTrack);
		// Creature stat definitions + the data bag ZSave persists.
		GDREGISTER_CLASS(ZAttribute);
		GDREGISTER_CLASS(ZLeveler);
		GDREGISTER_CLASS(ZCreatureData);
		GDREGISTER_CLASS(ZSave);
		GDREGISTER_CLASS(ZSaveGame);
		GDREGISTER_CLASS(ZSaveGlobal);
		GDREGISTER_CLASS(ZPlayer);
		GDREGISTER_CLASS(ZMoveMode);

		// World query (EQS-like). Base classes stay instantiable so GDScript
		// `class_name X extends WorldQueryShape/Utility` subclasses can attach.
		GDREGISTER_CLASS(WorldQueryShape);
		GDREGISTER_CLASS(WorldQueryShape_Grid);
		GDREGISTER_CLASS(WorldQueryShape_Donut);
		GDREGISTER_CLASS(WorldQueryUtility);
		GDREGISTER_CLASS(WorldQueryUtility_Trace);
		GDREGISTER_CLASS(WorldQueryUtility_Distance);
		GDREGISTER_CLASS(WorldQueryUtility_Angle);
		GDREGISTER_CLASS(WorldQueryResult);

		// Gameplay Framework nodes.
		GDREGISTER_CLASS(ZGameState);
		GDREGISTER_CLASS(ZGameMode);
		GDREGISTER_CLASS(ZMasterScene);
		GDREGISTER_CLASS(ZPlayerStart3D);
		GDREGISTER_CLASS(ZSceneTransit2D);
		GDREGISTER_CLASS(ZSceneTransit3D);
		GDREGISTER_CLASS(ZInputListener);
		GDREGISTER_CLASS(ZInputCommon);
		GDREGISTER_CLASS(ZMover);
		GDREGISTER_CLASS(ZLagger3D);
		GDREGISTER_CLASS(ZCameraFollower3D);
		GDREGISTER_CLASS(ZCanvas3D);
		GDREGISTER_CLASS(ZAbility);
		GDREGISTER_CLASS(ZCreature);
		GDREGISTER_CLASS(ZSkinModel);
		// ZDataOption before ZDataList (list owns/creates options).
		GDREGISTER_CLASS(ZDataOption);
		GDREGISTER_CLASS(ZDataList);
		// ZMenu before its subclasses; ZMenu_Choice also drives a ZDataList.
		GDREGISTER_CLASS(ZMenu);
		GDREGISTER_CLASS(ZMenu_Confirm);
		GDREGISTER_CLASS(ZMenu_Info);
		GDREGISTER_CLASS(ZMenu_Process);
		GDREGISTER_CLASS(ZMenu_Choice);
		GDREGISTER_CLASS(ZWorldQuery2D);
		GDREGISTER_CLASS(ZWorldQuery3D);
		GDREGISTER_INTERNAL_CLASS(ZWorldQueryDebugCanvas2D);
		GDREGISTER_CLASS(ZDebugRoot2D);
		GDREGISTER_CLASS(ZDebugRoot3D);
		GDREGISTER_CLASS(ZDebugRootUI);
		GDREGISTER_CLASS(Z);
		GDREGISTER_CLASS(Z_Node);
		GDREGISTER_CLASS(Z_Obj);
		GDREGISTER_CLASS(Z_Parse);
		GDREGISTER_CLASS(Z_Sound);
		GDREGISTER_CLASS(Z_Math);
		// Runtime music: Z_BGM is the API, ZBgmPlayer the node it drives.
		GDREGISTER_INTERNAL_CLASS(ZBgmPlayer);
		GDREGISTER_CLASS(Z_BGM);
		// Global async events (menus); after the ZMenu types it spawns.
		GDREGISTER_CLASS(ZEvent);

		// ProjectSettings: Project → Project Settings → ZanDot (zandot/*).
		Z::register_settings();
		// Default z_* bindings for ZInputCommon; reads a zandot/input/* setting,
		// so it has to follow register_settings().
		ZInputDefaults::register_all();
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
