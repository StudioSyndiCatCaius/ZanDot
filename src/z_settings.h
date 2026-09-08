#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Central registry for the `zandot/*` ProjectSettings.
//
// Keys are grouped into subcategories (general / saves / audio / performance) so
// Project Settings → ZanDot renders as a tree instead of one flat list. Values
// written against the older flat keys (`zandot/default_game_mode`, …) are
// migrated on load, so existing projects keep working.
namespace ZSettings {

extern const char *DEFAULT_GAME_MODE;
extern const char *DEFAULT_STARTING_STATE;
extern const char *BLOCK_DEFAULT_STATE_DURING_SCENE_TRANSIT;

extern const char *SAVE_GAME_PREFIX;
extern const char *SAVE_GLOBAL_NAME;
extern const char *SAVEGAME_SCRIPT;
extern const char *SAVEGLOBAL_SCRIPT;

extern const char *BGM_BUS;

extern const char *MENU_CONFIRM;
extern const char *MENU_INFO;
extern const char *MENU_PROCESS;
extern const char *MENU_CHOICE;

extern const char *REGISTER_DEFAULT_INPUT_ACTIONS;

extern const char *CACHE_OBJECT_NAME_VARS;

// Declares every setting with its default + inspector hint, and migrates legacy
// flat keys. Called once from initialize_zandot_module.
void register_all();

// Reads a setting, falling back to p_default when unset.
Variant get(const char *p_key, const Variant &p_default);

} // namespace ZSettings

} // namespace godot
