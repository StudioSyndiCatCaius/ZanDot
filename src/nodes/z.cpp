#include "z.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Z *Z::singleton = nullptr;

namespace {
const char *SETTING_DEFAULT_GAME_MODE = "zandot/default_game_mode";
const char *SETTING_SAVE_GAME_PREFIX = "zandot/save_game_prefix";
const char *SETTING_SAVE_GLOBAL_NAME = "zandot/save_global_name";
const char *SETTING_SAVEGAME_SCRIPT_CLASS = "zandot/savegame_script_class";
const char *SETTING_SAVEGLOBAL_SCRIPT_CLASS = "zandot/saveglobal_script_class";
} // namespace

void Z::register_settings() {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps == nullptr) {
		return;
	}

	if (!ps->has_setting(SETTING_DEFAULT_GAME_MODE)) {
		ps->set_setting(SETTING_DEFAULT_GAME_MODE, String(""));
	}
	ps->set_initial_value(SETTING_DEFAULT_GAME_MODE, String(""));
	ps->add_property_info(PropertyInfo(Variant::STRING, SETTING_DEFAULT_GAME_MODE, PROPERTY_HINT_FILE, "*.tscn,*.scn,*.res,*.tres"));

	if (!ps->has_setting(SETTING_SAVE_GAME_PREFIX)) {
		ps->set_setting(SETTING_SAVE_GAME_PREFIX, String("save_"));
	}
	ps->set_initial_value(SETTING_SAVE_GAME_PREFIX, String("save_"));
	ps->add_property_info(PropertyInfo(Variant::STRING, SETTING_SAVE_GAME_PREFIX));

	if (!ps->has_setting(SETTING_SAVE_GLOBAL_NAME)) {
		ps->set_setting(SETTING_SAVE_GLOBAL_NAME, String("global"));
	}
	ps->set_initial_value(SETTING_SAVE_GLOBAL_NAME, String("global"));
	ps->add_property_info(PropertyInfo(Variant::STRING, SETTING_SAVE_GLOBAL_NAME));

	if (!ps->has_setting(SETTING_SAVEGAME_SCRIPT_CLASS)) {
		ps->set_setting(SETTING_SAVEGAME_SCRIPT_CLASS, String(""));
	}
	ps->set_initial_value(SETTING_SAVEGAME_SCRIPT_CLASS, String(""));
	ps->add_property_info(PropertyInfo(Variant::STRING, SETTING_SAVEGAME_SCRIPT_CLASS));

	if (!ps->has_setting(SETTING_SAVEGLOBAL_SCRIPT_CLASS)) {
		ps->set_setting(SETTING_SAVEGLOBAL_SCRIPT_CLASS, String(""));
	}
	ps->set_initial_value(SETTING_SAVEGLOBAL_SCRIPT_CLASS, String(""));
	ps->add_property_info(PropertyInfo(Variant::STRING, SETTING_SAVEGLOBAL_SCRIPT_CLASS));
}

void Z::create_singleton() {
	if (singleton != nullptr) {
		return;
	}
	singleton = memnew(Z);
	Engine::get_singleton()->register_singleton("Z", singleton);
}

void Z::destroy_singleton() {
	if (singleton == nullptr) {
		return;
	}
	if (Engine::get_singleton()->has_singleton("Z")) {
		Engine::get_singleton()->unregister_singleton("Z");
	}
	memdelete(singleton);
	singleton = nullptr;
}

Z *Z::get_singleton() {
	return singleton;
}

void Z::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_save_game", "save"), &Z::set_save_game);
	ClassDB::bind_method(D_METHOD("get_save_game"), &Z::get_save_game);

	ClassDB::bind_method(D_METHOD("set_save_global", "save"), &Z::set_save_global);
	ClassDB::bind_method(D_METHOD("get_save_global"), &Z::get_save_global);

	ClassDB::bind_method(D_METHOD("set_players", "players"), &Z::set_players);
	ClassDB::bind_method(D_METHOD("get_players"), &Z::get_players);

	ClassDB::bind_method(D_METHOD("get_current_game_mode"), &Z::get_current_game_mode);
	ClassDB::bind_method(D_METHOD("set_current_game_mode", "mode"), &Z::set_current_game_mode);

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
	ClassDB::bind_method(D_METHOD("Player_Get_PawnScene", "player_index"), &Z::player_get_pawn_scene, DEFVAL(0));

	// snake_case aliases (Godot-native style).
	ClassDB::bind_method(D_METHOD("save_game_create"), &Z::save_game_create);
	ClassDB::bind_method(D_METHOD("save_game_load_from_path", "path"), &Z::save_game_load_from_path);
	ClassDB::bind_method(D_METHOD("save_game_load_from_slot", "slot"), &Z::save_game_load_from_slot);
	ClassDB::bind_method(D_METHOD("save_game_save_to_path", "save", "path"), &Z::save_game_save_to_path);
	ClassDB::bind_method(D_METHOD("save_game_save_to_slot", "save", "slot"), &Z::save_game_save_to_slot);

	ClassDB::bind_method(D_METHOD("save_global_create"), &Z::save_global_create);
	ClassDB::bind_method(D_METHOD("save_global_load"), &Z::save_global_load);
	ClassDB::bind_method(D_METHOD("save_global_save", "save"), &Z::save_global_save, DEFVAL(Ref<ZSaveGlobal>()));

	ClassDB::bind_method(D_METHOD("player_create"), &Z::player_create);
	ClassDB::bind_method(D_METHOD("player_remove", "player"), &Z::player_remove);
	ClassDB::bind_method(D_METHOD("player_remove_index", "player_index"), &Z::player_remove_index);
	ClassDB::bind_method(D_METHOD("player_get_pawn_scene", "player_index"), &Z::player_get_pawn_scene, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("on_level_start"), &Z::on_level_start);
	ClassDB::bind_method(D_METHOD("bootstrap"), &Z::bootstrap);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "save_game", PROPERTY_HINT_RESOURCE_TYPE, "ZSaveGame"), "set_save_game", "get_save_game");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "save_global", PROPERTY_HINT_RESOURCE_TYPE, "ZSaveGlobal"), "set_save_global", "get_save_global");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "players", PROPERTY_HINT_ARRAY_TYPE, "ZPlayer"), "set_players", "get_players");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "current_game_mode", PROPERTY_HINT_NODE_TYPE, "ZGameMode", PROPERTY_USAGE_SCRIPT_VARIABLE), "set_current_game_mode", "get_current_game_mode");
}

Z::Z() {
	singleton = this;
}

Z::~Z() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void Z::set_save_game(const Ref<ZSaveGame> &p_save) {
	save_game = p_save;
}

Ref<ZSaveGame> Z::get_save_game() const {
	return save_game;
}

void Z::set_save_global(const Ref<ZSaveGlobal> &p_save) {
	save_global = p_save;
}

Ref<ZSaveGlobal> Z::get_save_global() const {
	return save_global;
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

String Z::_settings_save_prefix() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr && ps->has_setting(SETTING_SAVE_GAME_PREFIX)) {
		return ps->get_setting(SETTING_SAVE_GAME_PREFIX);
	}
	return "save_";
}

String Z::_settings_global_name() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr && ps->has_setting(SETTING_SAVE_GLOBAL_NAME)) {
		return ps->get_setting(SETTING_SAVE_GLOBAL_NAME);
	}
	return "global";
}

String Z::_settings_default_game_mode() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr && ps->has_setting(SETTING_DEFAULT_GAME_MODE)) {
		return ps->get_setting(SETTING_DEFAULT_GAME_MODE);
	}
	return "";
}

String Z::_settings_savegame_script_class() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr && ps->has_setting(SETTING_SAVEGAME_SCRIPT_CLASS)) {
		return ps->get_setting(SETTING_SAVEGAME_SCRIPT_CLASS);
	}
	return "";
}

String Z::_settings_saveglobal_script_class() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps != nullptr && ps->has_setting(SETTING_SAVEGLOBAL_SCRIPT_CLASS)) {
		return ps->get_setting(SETTING_SAVEGLOBAL_SCRIPT_CLASS);
	}
	return "";
}

String Z::_slot_path(int p_slot) const {
	return String("user://") + _settings_save_prefix() + String::num_int64(p_slot) + ".tres";
}

String Z::_global_path() const {
	return String("user://") + _settings_global_name() + ".tres";
}

Ref<ZSaveGame> Z::save_game_create() {
	Ref<ZSaveGame> s;
	String script_class = _settings_savegame_script_class();
	if (!script_class.is_empty()) {
		Ref<Script> script = ResourceLoader::get_singleton()->load(script_class);
		if (script.is_valid()) {
			Object *obj = ClassDB::instantiate(script->get_instance_base_type());
			s = Object::cast_to<ZSaveGame>(obj);
			if (s.is_null()) {
				UtilityFunctions::push_warning("Z.save_game_create: custom script class is not a ZSaveGame subclass; using default.");
				s.instantiate();
			}
		} else {
			UtilityFunctions::push_warning("Z.save_game_create: failed to load custom script class " + script_class + "; using default.");
			s.instantiate();
		}
	} else {
		s.instantiate();
	}
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
	const Error err = ResourceSaver::get_singleton()->save(p_save, p_path);
	if (err != OK) {
		UtilityFunctions::push_error("Z.SaveGame_Save_ToPath: ResourceSaver failed (" + String::num_int64(err) + ") for " + p_path);
		return false;
	}
	save_game = p_save;
	return true;
}

bool Z::save_game_save_to_slot(const Ref<ZSaveGame> &p_save, int p_slot) {
	return save_game_save_to_path(p_save, _slot_path(p_slot));
}

Ref<ZSaveGlobal> Z::save_global_create() {
	Ref<ZSaveGlobal> s;
	String script_class = _settings_saveglobal_script_class();
	if (!script_class.is_empty()) {
		Ref<Script> script = ResourceLoader::get_singleton()->load(script_class);
		if (script.is_valid()) {
			Object *obj = ClassDB::instantiate(script->get_instance_base_type());
			s = Object::cast_to<ZSaveGlobal>(obj);
			if (s.is_null()) {
				UtilityFunctions::push_warning("Z.save_global_create: custom script class is not a ZSaveGlobal subclass; using default.");
				s.instantiate();
			}
		} else {
			UtilityFunctions::push_warning("Z.save_global_create: failed to load custom script class " + script_class + "; using default.");
			s.instantiate();
		}
	} else {
		s.instantiate();
	}
	s->initialize_new();
	save_global = s;
	return s;
}

Ref<ZSaveGlobal> Z::save_global_load() {
	const String path = _global_path();
	if (!ResourceLoader::get_singleton()->exists(path)) {
		return save_global_create();
	}
	Ref<Resource> res = ResourceLoader::get_singleton()->load(path);
	Ref<ZSaveGlobal> s = res;
	if (s.is_null()) {
		UtilityFunctions::push_warning("Z.SaveGlobal_Load: invalid resource at " + path + "; creating new.");
		return save_global_create();
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
	const Error err = ResourceSaver::get_singleton()->save(s, path);
	if (err != OK) {
		UtilityFunctions::push_error("Z.SaveGlobal_Save: failed (" + String::num_int64(err) + ") for " + path);
		return false;
	}
	save_global = s;
	return true;
}

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

Node *Z::player_get_pawn_scene(int p_player_index) const {
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

void Z::bootstrap() {
	if (bootstrapped) {
		if (!Engine::get_singleton()->is_editor_hint()) {
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

	_ensure_tree_hooks();
	// Initial level resolution (main scene may already be loaded).
	on_level_start();
}
