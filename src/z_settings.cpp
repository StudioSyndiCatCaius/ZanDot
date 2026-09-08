#include "z_settings.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace godot {
namespace ZSettings {

const char *DEFAULT_GAME_MODE = "zandot/general/default_game_mode";
const char *DEFAULT_STARTING_STATE = "zandot/general/default_starting_state";
const char *BLOCK_DEFAULT_STATE_DURING_SCENE_TRANSIT = "zandot/general/block_default_state_during_scene_transit";

const char *SAVE_GAME_PREFIX = "zandot/saves/save_game_prefix";
const char *SAVE_GLOBAL_NAME = "zandot/saves/save_global_name";
const char *SAVEGAME_SCRIPT = "zandot/saves/savegame_script";
const char *SAVEGLOBAL_SCRIPT = "zandot/saves/saveglobal_script";

const char *BGM_BUS = "zandot/audio/bgm_bus";

const char *MENU_CONFIRM = "zandot/menus/menu_confirm";
const char *MENU_INFO = "zandot/menus/menu_info";
const char *MENU_PROCESS = "zandot/menus/menu_process";
const char *MENU_CHOICE = "zandot/menus/menu_choice";

const char *REGISTER_DEFAULT_INPUT_ACTIONS = "zandot/input/register_default_actions";

const char *CACHE_OBJECT_NAME_VARS = "zandot/performance/cache_object_name_vars";

} // namespace ZSettings
} // namespace godot

namespace {

// Pre-subcategory key names, kept only so existing project.godot files migrate.
const char *LEGACY_DEFAULT_GAME_MODE = "zandot/default_game_mode";
const char *LEGACY_DEFAULT_STARTING_STATE = "zandot/default_starting_state";
const char *LEGACY_BLOCK_DEFAULT_STATE = "zandot/block_default_state_during_scene_transit";
const char *LEGACY_SAVE_GAME_PREFIX = "zandot/save_game_prefix";
const char *LEGACY_SAVE_GLOBAL_NAME = "zandot/save_global_name";
const char *LEGACY_SAVEGAME_SCRIPT = "zandot/savegame_script_class";
const char *LEGACY_SAVEGLOBAL_SCRIPT = "zandot/saveglobal_script_class";
const char *LEGACY_CACHE_OBJECT_NAME_VARS = "zandot/cache_object_name_vars";

bool migrated_any = false;

// Declares p_key with its default + inspector hint. When p_legacy_key still
// holds a value and p_key does not, the value moves across and the old key is
// dropped.
void def_setting(const char *p_key, const Variant &p_default, PropertyHint p_hint, const String &p_hint_string, const char *p_legacy_key) {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps == nullptr) {
		return;
	}

	if (p_legacy_key != nullptr && ps->has_setting(p_legacy_key)) {
		if (!ps->has_setting(p_key)) {
			ps->set_setting(p_key, ps->get_setting(p_legacy_key));
		}
		ps->clear(p_legacy_key);
		migrated_any = true;
	}

	if (!ps->has_setting(p_key)) {
		ps->set_setting(p_key, p_default);
	}
	ps->set_initial_value(p_key, p_default);

	// Built by hand rather than from PropertyInfo: that always carries a "usage"
	// entry, which add_property_info rejects with a warning on every launch.
	Dictionary info;
	info["name"] = String(p_key);
	info["type"] = p_default.get_type();
	info["hint"] = p_hint;
	info["hint_string"] = p_hint_string;
	ps->add_property_info(info);
}

} // namespace

void ZSettings::register_all() {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps == nullptr) {
		return;
	}

	migrated_any = false;

	const String scene_filter = "*.tscn,*.scn,*.res,*.tres";
	const String script_filter = "*.gd,*.cs";

	def_setting(DEFAULT_GAME_MODE, String(""), PROPERTY_HINT_FILE, scene_filter, LEGACY_DEFAULT_GAME_MODE);
	def_setting(DEFAULT_STARTING_STATE, String(""), PROPERTY_HINT_FILE, scene_filter, LEGACY_DEFAULT_STARTING_STATE);
	def_setting(BLOCK_DEFAULT_STATE_DURING_SCENE_TRANSIT, true, PROPERTY_HINT_NONE, String(), LEGACY_BLOCK_DEFAULT_STATE);

	def_setting(SAVE_GAME_PREFIX, String("save_"), PROPERTY_HINT_NONE, String(), LEGACY_SAVE_GAME_PREFIX);
	def_setting(SAVE_GLOBAL_NAME, String("global"), PROPERTY_HINT_NONE, String(), LEGACY_SAVE_GLOBAL_NAME);
	// Script attached to saves created by Z.SaveGame_Create / Z.SaveGlobal_Create;
	// pick the .gd/.cs file directly instead of typing a res:// path.
	def_setting(SAVEGAME_SCRIPT, String(""), PROPERTY_HINT_FILE, script_filter, LEGACY_SAVEGAME_SCRIPT);
	def_setting(SAVEGLOBAL_SCRIPT, String(""), PROPERTY_HINT_FILE, script_filter, LEGACY_SAVEGLOBAL_SCRIPT);

	def_setting(BGM_BUS, String("Master"), PROPERTY_HINT_NONE, String(), nullptr);

	// Scenes opened by the ZEvent.* menu helpers. Each scene root must be the
	// matching ZMenu subclass; left empty (or wrong type), ZEvent builds a plain
	// programmatic fallback so the call still does something.
	const String menu_scene_filter = "*.tscn,*.scn";
	def_setting(MENU_CONFIRM, String(""), PROPERTY_HINT_FILE, menu_scene_filter, nullptr);
	def_setting(MENU_INFO, String(""), PROPERTY_HINT_FILE, menu_scene_filter, nullptr);
	def_setting(MENU_PROCESS, String(""), PROPERTY_HINT_FILE, menu_scene_filter, nullptr);
	def_setting(MENU_CHOICE, String(""), PROPERTY_HINT_FILE, menu_scene_filter, nullptr);

	// Off = ZInputCommon's defaults stay unbound until the project defines them itself.
	def_setting(REGISTER_DEFAULT_INPUT_ACTIONS, true, PROPERTY_HINT_NONE, String(), nullptr);

	def_setting(CACHE_OBJECT_NAME_VARS, true, PROPERTY_HINT_NONE, String(), LEGACY_CACHE_OBJECT_NAME_VARS);

	// Persist the moved keys so project.godot stops carrying the flat names.
	// Only in the editor — an exported game cannot write its project file.
	if (migrated_any && OS::get_singleton()->has_feature("editor")) {
		if (ps->save() == OK) {
			UtilityFunctions::print("ZanDot: migrated legacy zandot/* project settings into subcategories.");
		}
	}
}

Variant ZSettings::get(const char *p_key, const Variant &p_default) {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps == nullptr || !ps->has_setting(p_key)) {
		return p_default;
	}
	return ps->get_setting(p_key, p_default);
}
