#include "z.h"

#include "z_master_scene.h"
#include "z_scene_transit.h"
#include "z_settings.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Z *Z::singleton = nullptr;

void Z::register_settings() {
	ZSettings::register_all();
}

void Z::create_singleton() {
	if (singleton != nullptr) {
		return;
	}
	singleton = memnew(Z);
	singleton->set_name("Z");
	Engine::get_singleton()->register_singleton("Z", singleton);
}

void Z::destroy_singleton() {
	// May already be gone: once attached, the SceneTree frees its children
	// (and so runs ~Z) before the extension is uninitialized.
	if (singleton == nullptr) {
		return;
	}
	Z *z = singleton;
	singleton = nullptr;
	if (Engine::get_singleton()->has_singleton("Z")) {
		Engine::get_singleton()->unregister_singleton("Z");
	}
	if (z->get_parent() != nullptr) {
		z->get_parent()->remove_child(z);
	}
	memdelete(z);
}

Z *Z::get_singleton() {
	return singleton;
}

void Z::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_save_game", "save"), &Z::set_save_game);
	ClassDB::bind_method(D_METHOD("get_save_game"), &Z::get_save_game);

	ClassDB::bind_method(D_METHOD("set_save_global", "save"), &Z::set_save_global);
	ClassDB::bind_method(D_METHOD("get_save_global"), &Z::get_save_global);

	ClassDB::bind_method(D_METHOD("set_save_location", "location"), &Z::set_save_location);
	ClassDB::bind_method(D_METHOD("get_save_location"), &Z::get_save_location);

	ClassDB::bind_method(D_METHOD("set_players", "players"), &Z::set_players);
	ClassDB::bind_method(D_METHOD("get_players"), &Z::get_players);

	ClassDB::bind_method(D_METHOD("get_current_game_mode"), &Z::get_current_game_mode);
	ClassDB::bind_method(D_METHOD("set_current_game_mode", "mode"), &Z::set_current_game_mode);

	ClassDB::bind_method(D_METHOD("set_master_scene", "scene"), &Z::set_master_scene);
	ClassDB::bind_method(D_METHOD("get_master_scene"), &Z::get_master_scene);

	ClassDB::bind_method(D_METHOD("get_scene_transit_state"), &Z::get_scene_transit_state);
	ClassDB::bind_method(D_METHOD("is_scene_transiting"), &Z::is_scene_transiting);

	// Spec-style names (PascalCase with underscores as in Agents.md).
	ClassDB::bind_method(D_METHOD("SaveGame_Create"), &Z::save_game_create);
	ClassDB::bind_method(D_METHOD("SaveGame_Load_FromPath", "path"), &Z::save_game_load_from_path);
	ClassDB::bind_method(D_METHOD("SaveGame_Load_FromSlot", "slot"), &Z::save_game_load_from_slot);
	ClassDB::bind_method(D_METHOD("SaveGame_Save_ToPath", "save", "path"), &Z::save_game_save_to_path);
	ClassDB::bind_method(D_METHOD("SaveGame_Save_ToSlot", "save", "slot"), &Z::save_game_save_to_slot);

	ClassDB::bind_method(D_METHOD("SaveGlobal_Create"), &Z::save_global_create);
	ClassDB::bind_method(D_METHOD("SaveGlobal_Load"), &Z::save_global_load);
	ClassDB::bind_method(D_METHOD("SaveGlobal_Save", "save"), &Z::save_global_save, DEFVAL(Ref<ZSaveGlobal>()));

	ClassDB::bind_method(D_METHOD("Player_Create"), &Z::player_create);
	ClassDB::bind_method(D_METHOD("Player_Remove", "player"), &Z::player_remove);
	ClassDB::bind_method(D_METHOD("Player_RemoveIndex", "player_index"), &Z::player_remove_index);
	ClassDB::bind_method(D_METHOD("Player_Get_Pawn", "player_index"), &Z::player_get_pawn, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("Scene_Transit", "new_scene", "skip_fade_out", "skip_fade_in"), &Z::scene_transit, DEFVAL(false), DEFVAL(false));

	// snake_case aliases (Godot-native style).
	ClassDB::bind_method(D_METHOD("save_game_create"), &Z::save_game_create);
	ClassDB::bind_method(D_METHOD("save_game_load_from_path", "path"), &Z::save_game_load_from_path);
	ClassDB::bind_method(D_METHOD("save_game_load_from_slot", "slot"), &Z::save_game_load_from_slot);
	ClassDB::bind_method(D_METHOD("save_game_save_to_path", "save", "path"), &Z::save_game_save_to_path);
	ClassDB::bind_method(D_METHOD("save_game_save_to_slot", "save", "slot"), &Z::save_game_save_to_slot);
	ClassDB::bind_method(D_METHOD("save_game_slot_dir", "slot"), &Z::save_game_slot_dir);
	ClassDB::bind_method(D_METHOD("save_game_slot_path", "slot"), &Z::save_game_slot_path);
	ClassDB::bind_method(D_METHOD("save_game_screenshot_path", "slot"), &Z::save_game_screenshot_path);

	ClassDB::bind_method(D_METHOD("save_global_create"), &Z::save_global_create);
	ClassDB::bind_method(D_METHOD("save_global_load"), &Z::save_global_load);
	ClassDB::bind_method(D_METHOD("save_global_save", "save"), &Z::save_global_save, DEFVAL(Ref<ZSaveGlobal>()));

	ClassDB::bind_method(D_METHOD("player_create"), &Z::player_create);
	ClassDB::bind_method(D_METHOD("player_remove", "player"), &Z::player_remove);
	ClassDB::bind_method(D_METHOD("player_remove_index", "player_index"), &Z::player_remove_index);
	ClassDB::bind_method(D_METHOD("player_get_pawn", "player_index"), &Z::player_get_pawn, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("global_condition_check", "condition"), &Z::global_condition_check);
	ClassDB::bind_method(D_METHOD("global_condition_check_list", "conditions", "if_one_true"), &Z::global_condition_check_list, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("global_script_run", "gscript"), &Z::global_script_run);
	ClassDB::bind_method(D_METHOD("global_script_run_list", "gscripts"), &Z::global_script_run_list);

	ClassDB::bind_method(D_METHOD("gEvent", "event", "meta"), &Z::g_event, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("g_event", "event", "meta"), &Z::g_event, DEFVAL(Dictionary()));

	ClassDB::bind_method(D_METHOD("hook_play_scene_transit", "fade_out"), &Z::hook_play_scene_transit);
	ClassDB::bind_method(D_METHOD("scene_transit", "new_scene", "skip_fade_out", "skip_fade_in"), &Z::scene_transit, DEFVAL(false), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("on_level_start"), &Z::on_level_start);
	ClassDB::bind_method(D_METHOD("bootstrap"), &Z::bootstrap);
	ClassDB::bind_method(D_METHOD("object_name_vars_cache_clear"), &Z::object_name_vars_cache_clear);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "save_game", PROPERTY_HINT_RESOURCE_TYPE, "ZSaveGame"), "set_save_game", "get_save_game");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "save_global", PROPERTY_HINT_RESOURCE_TYPE, "ZSaveGlobal"), "set_save_global", "get_save_global");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "save_location", PROPERTY_HINT_ENUM, "User,GameDir"), "set_save_location", "get_save_location");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "players", PROPERTY_HINT_ARRAY_TYPE, "ZPlayer"), "set_players", "get_players");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "current_game_mode", PROPERTY_HINT_NODE_TYPE, "ZGameMode", PROPERTY_USAGE_SCRIPT_VARIABLE), "set_current_game_mode", "get_current_game_mode");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "master_scene", PROPERTY_HINT_NODE_TYPE, "ZMasterScene", PROPERTY_USAGE_SCRIPT_VARIABLE), "set_master_scene", "get_master_scene");

	BIND_ENUM_CONSTANT(SAVE_LOCATION_USER);
	BIND_ENUM_CONSTANT(SAVE_LOCATION_GAMEDIR);

	BIND_ENUM_CONSTANT(SCENE_TRANSIT_NONE);
	BIND_ENUM_CONSTANT(SCENE_TRANSIT_FADE_OUT);
	BIND_ENUM_CONSTANT(SCENE_TRANSIT_TRANSITING);
	BIND_ENUM_CONSTANT(SCENE_TRANSIT_FADE_IN);

	ADD_SIGNAL(MethodInfo("on_game_mode_ready", PropertyInfo(Variant::OBJECT, "game_mode", PROPERTY_HINT_NODE_TYPE, "ZGameMode")));
	ADD_SIGNAL(MethodInfo("on_scene_transit_state_change", PropertyInfo(Variant::INT, "state")));
	ADD_SIGNAL(MethodInfo("on_gEvent", PropertyInfo(Variant::STRING, "event"), PropertyInfo(Variant::DICTIONARY, "meta")));
}

Z::Z() {
	// Now that Z is a Node it can be added from the editor's Create Node dialog.
	// Only the one create_singleton() makes is the real "Z"; a stray instance
	// must not hijack the pointer.
	if (singleton == nullptr) {
		singleton = this;
	} else {
		UtilityFunctions::push_warning("Z: extra instance created. Z is a global singleton — use the existing root/Z node, not a new one.");
	}
}

Z::~Z() {
	if (singleton == this) {
		// Freed by the SceneTree rather than destroy_singleton — drop the
		// Engine singleton entry here so it never dangles.
		if (Engine::get_singleton()->has_singleton("Z")) {
			Engine::get_singleton()->unregister_singleton("Z");
		}
		singleton = nullptr;
	}
}

void Z::set_save_game(const Ref<ZSaveGame> &p_save) {
	save_game = p_save;
}

Ref<ZSaveGame> Z::get_save_game() {
	// A SaveGame always exists — create a blank one on first access.
	if (save_game.is_null()) {
		save_game_create();
	}
	return save_game;
}

void Z::set_save_global(const Ref<ZSaveGlobal> &p_save) {
	save_global = p_save;
	// Assigning null would break the "always exists" guarantee for any reader
	// that holds a Z* directly rather than going through get_save_global().
	if (save_global.is_null() && !loading_save_global) {
		save_global_load();
	}
}

Ref<ZSaveGlobal> Z::get_save_global() {
	// A SaveGlobal always exists. bootstrap() normally loads it, but that is a
	// deferred call — anything reading Z.save_global from the first scene's
	// _ready() can beat it, so load on demand here too.
	if (save_global.is_null()) {
		save_global_load();
	}
	return save_global;
}

void Z::set_save_location(SaveLocation p_location) {
	save_location = p_location;
}

Z::SaveLocation Z::get_save_location() const {
	return save_location;
}

void Z::set_players(const TypedArray<ZPlayer> &p_players) {
	players = p_players;
}

TypedArray<ZPlayer> Z::get_players() const {
	return players;
}

ZGameMode *Z::get_current_game_mode() const {
	if (current_game_mode_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<ZGameMode>(ObjectDB::get_instance(current_game_mode_id));
}

void Z::set_current_game_mode(ZGameMode *p_mode) {
	if (p_mode == nullptr) {
		current_game_mode_id = ObjectID();
	} else {
		current_game_mode_id = p_mode->get_instance_id();
	}
}

// ==========================================================================================
// MASTER SCENE
// ==========================================================================================

void Z::set_master_scene(ZMasterScene *p_scene) {
	if (p_scene == nullptr) {
		master_scene_id = ObjectID();
	} else {
		master_scene_id = p_scene->get_instance_id();
	}
}

ZMasterScene *Z::get_master_scene() const {
	if (master_scene_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<ZMasterScene>(ObjectDB::get_instance(master_scene_id));
}

// ==========================================================================================
// SCENE TRANSIT
// ==========================================================================================


Z::SceneTransitState Z::get_scene_transit_state() const {
	return scene_transit_state;
}

bool Z::is_scene_transiting() const {
	return scene_transit_state != SCENE_TRANSIT_NONE;
}

void Z::set_pending_scene_transit(const ZSceneTransitPending &p_pending) {
	pending_scene_transit_match = p_pending;
}

ZSceneTransitPending Z::get_pending_scene_transit() const {
	return pending_scene_transit_match;
}

void Z::clear_pending_scene_transit() {
	pending_scene_transit_match = ZSceneTransitPending();
}

bool Z::should_block_default_state() const {
	return is_scene_transiting() && _settings_block_default_state_during_scene_transit();
}

void Z::note_deferred_starting_state() {
	deferred_starting_state_after_transit = true;
}

// ==========================================================================================
// OBJECT NAME VARS CACHE
// ==========================================================================================

bool Z::settings_cache_object_name_vars() const {
	return (bool)ZSettings::get(ZSettings::CACHE_OBJECT_NAME_VARS, true);
}

bool Z::object_name_vars_cache_get(uint64_t p_instance_id, PackedStringArray &r_out) const {
	const Variant key = (int64_t)p_instance_id;
	if (!object_name_vars_cache.has(key)) {
		return false;
	}
	r_out = object_name_vars_cache[key];
	return true;
}

void Z::object_name_vars_cache_set(uint64_t p_instance_id, const PackedStringArray &p_vars) {
	object_name_vars_cache[(int64_t)p_instance_id] = p_vars;
}

void Z::object_name_vars_cache_clear() {
	object_name_vars_cache.clear();
}

// ==========================================================================================
// SETTINGS
// ==========================================================================================

String Z::_settings_save_prefix() const {
	return ZSettings::get(ZSettings::SAVE_GAME_PREFIX, String("save_"));
}

String Z::_settings_global_name() const {
	return ZSettings::get(ZSettings::SAVE_GLOBAL_NAME, String("global"));
}

String Z::_settings_default_game_mode() const {
	return ZSettings::get(ZSettings::DEFAULT_GAME_MODE, String(""));
}

String Z::_settings_savegame_script_class() const {
	return ZSettings::get(ZSettings::SAVEGAME_SCRIPT, String(""));
}

String Z::_settings_saveglobal_script_class() const {
	return ZSettings::get(ZSettings::SAVEGLOBAL_SCRIPT, String(""));
}

bool Z::_settings_block_default_state_during_scene_transit() const {
	return (bool)ZSettings::get(ZSettings::BLOCK_DEFAULT_STATE_DURING_SCENE_TRANSIT, true);
}

String Z::_game_name() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr) {
		const String name = ps->get_setting("application/config/name", "");
		if (!name.is_empty()) {
			return name;
		}
	}
	return "Game";
}

// ==========================================================================================
// SAVE
// ==========================================================================================


String Z::_save_root_dir() const {
	if (save_location == SAVE_LOCATION_GAMEDIR) {
		return OS::get_singleton()->get_executable_path().get_base_dir() + "/save/";
	}

	// USER: OS "Saved Games" folder; godot-cpp has no dedicated SystemDir for
	// it, so resolve it from the user's profile dir directly.
	OS *os = OS::get_singleton();
	String home;
	if (os->has_environment("USERPROFILE")) {
		home = os->get_environment("USERPROFILE");
	} else if (os->has_environment("HOME")) {
		home = os->get_environment("HOME");
	}
	if (home.is_empty()) {
		return "user://";
	}
	return home + "/Saved Games/Zandot/" + _game_name() + "/";
}

String Z::_slot_dir(int p_slot) const {
	return _save_root_dir() + _settings_save_prefix() + String::num_int64(p_slot).pad_zeros(3) + "/";
}

String Z::_slot_path(int p_slot) const {
	// .tres, not a custom extension: ResourceSaver/ResourceLoader pick their
	// serializer by extension and have no handler for anything else.
	return _slot_dir(p_slot) + _settings_save_prefix() + String::num_int64(p_slot).pad_zeros(3) + ".tres";
}

String Z::_slot_screenshot_path(int p_slot) const {
	return _slot_dir(p_slot) + _settings_save_prefix() + String::num_int64(p_slot).pad_zeros(3) + ".png";
}

String Z::_global_path() const {
	return _save_root_dir() + _settings_global_name() + ".tres";
}


void Z::_save_slot_screenshot(int p_slot) const {
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr) {
		return;
	}
	Window *root = tree->get_root();
	if (root == nullptr) {
		return;
	}
	Ref<ViewportTexture> vt = root->get_texture();
	if (vt.is_null()) {
		return;
	}
	Ref<Image> img = vt->get_image();
	if (img.is_null()) {
		return;
	}
	img->save_png(_slot_screenshot_path(p_slot));
}

// Instantiates T, attaching the script at p_script_path when one is configured.
// Falls back to a plain T (with a warning) if the script is missing or does not
// extend T.
template <typename T>
static Ref<T> _create_save_from_script(const String &p_script_path, const char *p_context) {
	Ref<T> out;
	out.instantiate();
	if (p_script_path.is_empty()) {
		return out;
	}

	Ref<Script> script = ResourceLoader::get_singleton()->load(p_script_path, "Script");
	if (script.is_null()) {
		UtilityFunctions::push_warning(String(p_context) + ": failed to load custom save script " + p_script_path + "; using default.");
		return out;
	}

	const StringName base_type = script->get_instance_base_type();
	if (base_type != T::get_class_static() && !ClassDB::is_parent_class(base_type, T::get_class_static())) {
		UtilityFunctions::push_warning(String(p_context) + ": " + p_script_path + " does not extend " + T::get_class_static() + "; using default.");
		return out;
	}

	// Construct the native base above and attach the script here, rather than
	// ClassDB::instantiate(base_type) — that is unreliable for GDExtension bases
	// (same reason as FlowEditorPlugin::_make_node). Without the script attached,
	// the save's own vars would never exist.
	out->set_script(script);
	return out;
}

Ref<ZSaveGame> Z::save_game_create() {
	Ref<ZSaveGame> s = _create_save_from_script<ZSaveGame>(_settings_savegame_script_class(), "Z.save_game_create");
	s->initialize_new();
	save_game = s;
	return s;
}

Ref<ZSaveGame> Z::save_game_load_from_path(const String &p_path) {
	if (p_path.is_empty()) {
		UtilityFunctions::push_error("Z.SaveGame_Load_FromPath: empty path.");
		return Ref<ZSaveGame>();
	}
	Ref<Resource> res = ResourceLoader::get_singleton()->load(p_path);
	Ref<ZSaveGame> s = res;
	if (s.is_null()) {
		UtilityFunctions::push_error("Z.SaveGame_Load_FromPath: failed to load ZSaveGame from " + p_path);
		return Ref<ZSaveGame>();
	}
	save_game = s;
	return s;
}

Ref<ZSaveGame> Z::save_game_load_from_slot(int p_slot) {
	return save_game_load_from_path(_slot_path(p_slot));
}

bool Z::save_game_save_to_path(const Ref<ZSaveGame> &p_save, const String &p_path) {
	if (p_save.is_null()) {
		UtilityFunctions::push_error("Z.SaveGame_Save_ToPath: save is null.");
		return false;
	}
	if (p_path.is_empty()) {
		UtilityFunctions::push_error("Z.SaveGame_Save_ToPath: empty path.");
		return false;
	}
	p_save->touch_saved();
	const String dir = p_path.get_base_dir();
	if (!dir.is_empty()) {
		DirAccess::make_dir_recursive_absolute(dir);
	}
	const Error err = ResourceSaver::get_singleton()->save(p_save, p_path);
	if (err != OK) {
		UtilityFunctions::push_error("Z.SaveGame_Save_ToPath: ResourceSaver failed (" + String::num_int64(err) + ") for " + p_path);
		return false;
	}
	save_game = p_save;
	return true;
}

String Z::save_game_slot_dir(int p_slot) const {
	return _slot_dir(p_slot);
}

String Z::save_game_slot_path(int p_slot) const {
	return _slot_path(p_slot);
}

String Z::save_game_screenshot_path(int p_slot) const {
	return _slot_screenshot_path(p_slot);
}

bool Z::save_game_save_to_slot(const Ref<ZSaveGame> &p_save, int p_slot) {
	const bool ok = save_game_save_to_path(p_save, _slot_path(p_slot));
	if (ok) {
		_save_slot_screenshot(p_slot);
	}
	return ok;
}

Ref<ZSaveGlobal> Z::save_global_create() {
	Ref<ZSaveGlobal> s = _create_save_from_script<ZSaveGlobal>(_settings_saveglobal_script_class(), "Z.save_global_create");
	s->initialize_new();
	save_global = s;
	return s;
}

// First launch (or a corrupt file): make the global save and write it out, so
// there is always one on disk as well as in memory. Skipped in the editor —
// running the editor should not author the player's save data.
Ref<ZSaveGlobal> Z::_save_global_create_and_persist() {
	Ref<ZSaveGlobal> s = save_global_create();
	if (s.is_valid() && !Engine::get_singleton()->is_editor_hint()) {
		save_global_save(s);
	}
	return s;
}

Ref<ZSaveGlobal> Z::save_global_load() {
	if (loading_save_global) {
		// Re-entered from inside the load below. Hand back whatever exists now
		// rather than recursing into ResourceLoader on the same path.
		return save_global;
	}
	loading_save_global = true;
	Ref<ZSaveGlobal> out = _save_global_load_inner();
	loading_save_global = false;
	return out;
}

Ref<ZSaveGlobal> Z::_save_global_load_inner() {
	const String path = _global_path();
	if (!ResourceLoader::get_singleton()->exists(path)) {
		return _save_global_create_and_persist();
	}
	Ref<Resource> res = ResourceLoader::get_singleton()->load(path);
	Ref<ZSaveGlobal> s = res;
	if (s.is_null()) {
		UtilityFunctions::push_warning("Z.SaveGlobal_Load: invalid resource at " + path + "; creating new.");
		return _save_global_create_and_persist();
	}
	save_global = s;
	return s;
}

bool Z::save_global_save(const Ref<ZSaveGlobal> &p_save) {
	Ref<ZSaveGlobal> s = p_save.is_valid() ? p_save : save_global;
	if (s.is_null()) {
		UtilityFunctions::push_error("Z.SaveGlobal_Save: no global save to write.");
		return false;
	}
	s->touch_saved();
	const String path = _global_path();
	const String dir = path.get_base_dir();
	if (!dir.is_empty()) {
		DirAccess::make_dir_recursive_absolute(dir);
	}
	const Error err = ResourceSaver::get_singleton()->save(s, path);
	if (err != OK) {
		UtilityFunctions::push_error("Z.SaveGlobal_Save: failed (" + String::num_int64(err) + ") for " + path);
		return false;
	}
	save_global = s;
	return true;
}

// ==========================================================================================
// PLAYER
// ==========================================================================================


Ref<ZPlayer> Z::player_create() {
	Ref<ZPlayer> p;
	p.instantiate();
	p->set_player_index(players.size());
	players.push_back(p);
	return p;
}

void Z::player_remove(const Ref<ZPlayer> &p_player) {
	if (p_player.is_null()) {
		return;
	}
	for (int i = 0; i < players.size(); i++) {
		Ref<ZPlayer> cur = players[i];
		if (cur == p_player) {
			player_remove_index(i);
			return;
		}
	}
}

void Z::player_remove_index(int p_player_index) {
	if (p_player_index < 0 || p_player_index >= players.size()) {
		return;
	}
	players.remove_at(p_player_index);
	for (int i = 0; i < players.size(); i++) {
		Ref<ZPlayer> p = players[i];
		if (p.is_valid()) {
			p->set_player_index(i);
		}
	}
}

Node *Z::player_get_pawn(int p_player_index) const {
	if (p_player_index < 0 || p_player_index >= players.size()) {
		return nullptr;
	}
	Ref<ZPlayer> p = players[p_player_index];
	if (p.is_null()) {
		return nullptr;
	}
	return p->get_possessed_scene();
}

ZGameMode *Z::_find_game_mode_in(Node *p_root, int &r_count) const {
	ZGameMode *found = nullptr;
	r_count = 0;
	if (p_root == nullptr) {
		return nullptr;
	}

	// Prefer group membership (includes script subclasses that join the group).
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree != nullptr) {
		TypedArray<Node> nodes = tree->get_nodes_in_group("z_game_mode");
		for (int i = 0; i < nodes.size(); i++) {
			ZGameMode *mode = Object::cast_to<ZGameMode>(nodes[i]);
			if (mode == nullptr) {
				continue;
			}
			// Only count modes under the current scene (or the root itself).
			if (p_root == mode || p_root->is_ancestor_of(mode)) {
				r_count++;
				if (found == nullptr) {
					found = mode;
				}
			}
		}
		if (r_count > 0) {
			return found;
		}
	}

	// Fallback: recursive class walk (covers modes not yet in tree group timing).
	if (Object::cast_to<ZGameMode>(p_root) != nullptr) {
		r_count = 1;
		return Object::cast_to<ZGameMode>(p_root);
	}
	TypedArray<Node> children = p_root->get_children();
	for (int i = 0; i < children.size(); i++) {
		int sub = 0;
		ZGameMode *sub_found = _find_game_mode_in(Object::cast_to<Node>(children[i]), sub);
		r_count += sub;
		if (found == nullptr && sub_found != nullptr) {
			found = sub_found;
		}
	}
	return found;
}

// ==========================================================================================
// MISC
// ==========================================================================================


void Z::_resolve_game_mode_for_current_scene() {
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr) {
		return;
	}
	Node *current = tree->get_current_scene();
	if (current == nullptr) {
		set_current_game_mode(nullptr);
		return;
	}

	int count = 0;
	ZGameMode *mode = _find_game_mode_in(current, count);
	if (count > 1) {
		UtilityFunctions::push_warning("Z: scene has multiple ZGameMode nodes (" + String::num_int64(count) + "); using the first found.");
	}

	if (mode == nullptr) {
		const String path = _settings_default_game_mode();
		if (!path.is_empty() && ResourceLoader::get_singleton()->exists(path)) {
			Ref<PackedScene> ps = ResourceLoader::get_singleton()->load(path);
			if (ps.is_valid()) {
				Node *inst = ps->instantiate();
				mode = Object::cast_to<ZGameMode>(inst);
				if (mode == nullptr) {
					UtilityFunctions::push_warning("Z: default_game_mode root is not a ZGameMode: " + path);
					if (inst != nullptr) {
						inst->queue_free();
					}
				} else {
					current->add_child(mode);
				}
			} else {
				UtilityFunctions::push_warning("Z: failed to load default_game_mode: " + path);
			}
		}
	}

	set_current_game_mode(mode);
	if (mode != nullptr) {
		mode->setup();
		emit_signal("on_game_mode_ready", mode);
	}
}

void Z::on_level_start() {
	_resolve_game_mode_for_current_scene();
}

void Z::_on_scene_changed() {
	// Defer so current_scene and children are fully ready.
	callable_mp(this, &Z::on_level_start).call_deferred();
}

void Z::_ensure_tree_hooks() {
	if (listening_tree) {
		return;
	}
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr) {
		return;
	}
	if (!tree->is_connected("tree_changed", callable_mp(this, &Z::_on_scene_changed))) {
		// Prefer scene_changed when available; fall back handled below.
	}
	// Godot 4 SceneTree has scene_changed when current scene changes.
	if (tree->has_signal("scene_changed")) {
		if (!tree->is_connected("scene_changed", callable_mp(this, &Z::_on_scene_changed))) {
			tree->connect("scene_changed", callable_mp(this, &Z::_on_scene_changed));
		}
	}
	listening_tree = true;
}

void Z::_attach_to_tree() {
	if (is_inside_tree()) {
		return;
	}
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr || tree->get_root() == nullptr) {
		return;
	}
	// Global state must keep running while the game is paused, and parenting to
	// the root viewport means scene changes leave it alone.
	set_process_mode(Node::PROCESS_MODE_ALWAYS);
	tree->get_root()->add_child(this);
}

void Z::bootstrap() {
	if (bootstrapped) {
		if (!Engine::get_singleton()->is_editor_hint()) {
			_attach_to_tree();
			_ensure_tree_hooks();
		}
		return;
	}
	bootstrapped = true;

	// Load or create global save once per session (runtime + editor tooling).
	if (save_global.is_null()) {
		save_global_load();
	}

	// Do not auto-resolve game modes while editing scenes.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// A SaveGame always exists too, from the start of the session.
	if (save_game.is_null()) {
		save_game_create();
	}

	_attach_to_tree();
	_ensure_tree_hooks();
	// Initial level resolution (main scene may already be loaded).
	on_level_start();
}

// ==========================================================================================
// GLOBAL SCRIPT / CONDITION RUN
// ==========================================================================================


bool Z::global_condition_check(const Ref<ZGlobalCondition> &p_condition) {
	if (p_condition.is_null()) {
		return false;
	}
	return p_condition->check_bind();
}

bool Z::global_condition_check_list(const TypedArray<ZGlobalCondition> &p_conditions, bool p_if_one_true) {
	if (p_conditions.is_empty()) {
		// Empty list: all-true vacuously, any-true never.
		return !p_if_one_true;
	}
	if (p_if_one_true) {
		for (int i = 0; i < p_conditions.size(); i++) {
			Ref<ZGlobalCondition> c = p_conditions[i];
			if (c.is_valid() && c->check_bind()) {
				return true;
			}
		}
		return false;
	}
	for (int i = 0; i < p_conditions.size(); i++) {
		Ref<ZGlobalCondition> c = p_conditions[i];
		if (c.is_null() || !c->check_bind()) {
			return false;
		}
	}
	return true;
}

void Z::global_script_run(const Ref<ZGlobalScript> &p_script) {
	if (p_script.is_null()) {
		return;
	}
	p_script->run_bind();
}

void Z::global_script_run_list(const TypedArray<ZGlobalScript> &p_scripts) {
	for (int i = 0; i < p_scripts.size(); i++) {
		Ref<ZGlobalScript> s = p_scripts[i];
		if (s.is_valid()) {
			s->run_bind();
		}
	}
}

// ==========================================================================================
// GLOBAL EVENTS
// ==========================================================================================

void Z::g_event(const String &p_event, const Dictionary &p_meta) {
	emit_signal("on_gEvent", p_event, p_meta);
}

// --- Scene transit ---

Signal Z::hook_play_scene_transit(bool p_fade_out) {
	ZMasterScene *master = get_master_scene();
	if (master == nullptr) {
		return Signal();
	}
	return master->hook_play_scene_transit(p_fade_out);
}

bool Z::_connect_one_shot_signal(const Signal &p_signal, const Callable &p_callable) {
	Object *obj = p_signal.get_object();
	const StringName signal_name = p_signal.get_name();
	if (obj == nullptr || signal_name == StringName()) {
		return false;
	}

	// The hook's Signal usually carries arguments the transit doesn't care about
	// (AnimationPlayer.animation_finished passes anim_name; Tween.finished passes none).
	// connect() does not check arity — the mismatch only fails when the signal fires,
	// which would strand the transit mid-fade. Drop the extra args up front.
	int argc = 0;
	const TypedArray<Dictionary> signal_list = obj->get_signal_list();
	for (int i = 0; i < signal_list.size(); i++) {
		const Dictionary info = signal_list[i];
		if (StringName(info.get("name", StringName())) != signal_name) {
			continue;
		}
		const Array args = info.get("args", Array());
		argc = args.size();
		break;
	}

	Callable target = p_callable;
	if (argc > 0) {
		target = target.unbind(argc);
	}

	// CONNECT_ONE_SHOT so fade completion only advances transit once.
	Signal sig = p_signal;
	if (sig.is_connected(target)) {
		return true;
	}
	const Error err = (Error)sig.connect(target, Object::CONNECT_ONE_SHOT);
	return err == OK;
}

void Z::_set_scene_transit_state(SceneTransitState p_state) {
	if (scene_transit_state == p_state) {
		return;
	}
	scene_transit_state = p_state;
	emit_signal("on_scene_transit_state_change", (int)p_state);
}

void Z::scene_transit(const Ref<PackedScene> &p_new_scene, bool p_skip_fade_out, bool p_skip_fade_in) {
	if (p_new_scene.is_null()) {
		UtilityFunctions::push_error("Z.Scene_Transit: new_scene is null.");
		return;
	}
	if (scene_transit_state != SCENE_TRANSIT_NONE) {
		UtilityFunctions::push_warning("Z.Scene_Transit: already in progress; ignoring.");
		return;
	}
	if (Engine::get_singleton()->is_editor_hint()) {
		UtilityFunctions::push_warning("Z.Scene_Transit: ignored in editor.");
		return;
	}

	pending_transit_scene = p_new_scene;
	pending_skip_fade_out = p_skip_fade_out;
	pending_skip_fade_in = p_skip_fade_in;

	if (p_skip_fade_out) {
		_scene_transit_do_change();
	} else {
		_scene_transit_begin_fade_out();
	}
}

void Z::_scene_transit_begin_fade_out() {
	_set_scene_transit_state(SCENE_TRANSIT_FADE_OUT);
	const Signal sig = hook_play_scene_transit(true);
	if (!_connect_one_shot_signal(sig, callable_mp(this, &Z::_scene_transit_on_fade_out_done))) {
		_scene_transit_on_fade_out_done();
	}
}

void Z::_scene_transit_on_fade_out_done() {
	_scene_transit_do_change();
}

void Z::_scene_transit_do_change() {
	_set_scene_transit_state(SCENE_TRANSIT_TRANSITING);

	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree == nullptr || pending_transit_scene.is_null()) {
		UtilityFunctions::push_error("Z.Scene_Transit: no SceneTree or pending scene.");
		_scene_transit_finish();
		return;
	}

	const Error err = tree->change_scene_to_packed(pending_transit_scene);
	if (err != OK) {
		UtilityFunctions::push_error("Z.Scene_Transit: change_scene_to_packed failed (" + String::num_int64(err) + ").");
		_scene_transit_finish();
		return;
	}

	// Wait until the new scene is ready (after Z.on_level_start deferred setup).
	callable_mp(this, &Z::_scene_transit_after_scene_ready).call_deferred();
}

void Z::_scene_transit_after_scene_ready() {
	// Spawn is handled by ZGameMode.spawn_players via ZPlayerStart (linked_map / link_id).
	// Keep pending_scene_transit_match until spawn_players consumes it.
	if (pending_skip_fade_in) {
		_scene_transit_finish();
	} else {
		_scene_transit_begin_fade_in();
	}
}

void Z::_scene_transit_begin_fade_in() {
	_set_scene_transit_state(SCENE_TRANSIT_FADE_IN);
	const Signal sig = hook_play_scene_transit(false);
	if (!_connect_one_shot_signal(sig, callable_mp(this, &Z::_scene_transit_on_fade_in_done))) {
		_scene_transit_on_fade_in_done();
	}
}

void Z::_scene_transit_on_fade_in_done() {
	_scene_transit_finish();
}

void Z::_scene_transit_finish() {
	pending_transit_scene.unref();
	pending_skip_fade_in = false;
	pending_skip_fade_out = false;
	// Do not clear pending_scene_transit_match here — ZGameMode.spawn_players (possibly delayed)
	// consumes it for ZPlayerStart linked_map / link_id selection.
	_set_scene_transit_state(SCENE_TRANSIT_NONE);

	// Enter starting/default state if it was deferred during transit.
	if (deferred_starting_state_after_transit) {
		deferred_starting_state_after_transit = false;
		ZGameMode *mode = get_current_game_mode();
		if (mode != nullptr && mode->get_active_state() == nullptr) {
			mode->change_to_starting_state();
		}
	}
}
