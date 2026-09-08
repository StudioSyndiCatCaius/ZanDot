#include "z_obj.h"

#include "z.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Z_Obj::_bind_methods() {
	// Spec-style names.
	ClassDB::bind_static_method("Z_Obj", D_METHOD("getNameVars", "obj", "delimiter", "force_recache"), &Z_Obj::get_name_vars, DEFVAL("_"), DEFVAL(false));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("gNameVar_string", "obj", "nvar", "default"), &Z_Obj::g_name_var_string, DEFVAL(String()));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("gNameVar_bool", "obj", "nvar", "true_str", "default"), &Z_Obj::g_name_var_bool, DEFVAL("true"), DEFVAL(false));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("gNameVar_int", "obj", "nvar", "default"), &Z_Obj::g_name_var_int, DEFVAL(0));

	// snake_case aliases.
	ClassDB::bind_static_method("Z_Obj", D_METHOD("get_name_vars", "obj", "delimiter", "force_recache"), &Z_Obj::get_name_vars, DEFVAL("_"), DEFVAL(false));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("g_name_var_string", "obj", "nvar", "default"), &Z_Obj::g_name_var_string, DEFVAL(String()));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("g_name_var_bool", "obj", "nvar", "true_str", "default"), &Z_Obj::g_name_var_bool, DEFVAL("true"), DEFVAL(false));
	ClassDB::bind_static_method("Z_Obj", D_METHOD("g_name_var_int", "obj", "nvar", "default"), &Z_Obj::g_name_var_int, DEFVAL(0));

	ClassDB::bind_static_method("Z_Obj", D_METHOD("resolve_object_name", "obj"), &Z_Obj::resolve_object_name);
}

Z_Obj::Z_Obj() {
}

Z_Obj::~Z_Obj() {
}

String Z_Obj::resolve_object_name(Object *p_obj) {
	if (p_obj == nullptr) {
		return String();
	}

	if (Resource *res = Object::cast_to<Resource>(p_obj)) {
		const String res_name = res->get_name();
		if (!res_name.is_empty()) {
			return res_name;
		}
		const String path = res->get_path();
		if (!path.is_empty()) {
			// "res://skills/skill_magatk_fire_one_1.tres" -> "skill_magatk_fire_one_1"
			return path.get_file().get_basename();
		}
	}

	if (Node *node = Object::cast_to<Node>(p_obj)) {
		return String(node->get_name());
	}

	// Generic Object with a "name" property (e.g. some RefCounteds).
	if (p_obj->get("name").get_type() == Variant::STRING) {
		const String n = p_obj->get("name");
		if (!n.is_empty()) {
			return n;
		}
	}

	return String();
}

PackedStringArray Z_Obj::get_name_vars(Object *p_obj, const String &p_delimiter, bool p_force_recache) {
	PackedStringArray empty;
	if (p_obj == nullptr) {
		return empty;
	}

	Z *z = Z::get_singleton();
	const bool use_cache = (z != nullptr) && z->settings_cache_object_name_vars();
	const uint64_t id = p_obj->get_instance_id();

	if (use_cache && !p_force_recache) {
		PackedStringArray cached;
		if (z->object_name_vars_cache_get(id, cached)) {
			return cached;
		}
	}

	const String name = resolve_object_name(p_obj);
	PackedStringArray parts;
	if (name.is_empty()) {
		if (use_cache) {
			z->object_name_vars_cache_set(id, parts);
		}
		return parts;
	}

	const String delim = p_delimiter.is_empty() ? String("_") : p_delimiter;
	// Godot String::split keeps empty segments by default when allow_empty is true.
	parts = name.split(delim, false);

	if (use_cache) {
		z->object_name_vars_cache_set(id, parts);
	}
	return parts;
}

String Z_Obj::g_name_var_string(Object *p_obj, int p_nvar, const String &p_default) {
	if (p_nvar < 0) {
		return p_default;
	}
	const PackedStringArray vars = get_name_vars(p_obj);
	if (p_nvar >= vars.size()) {
		return p_default;
	}
	return vars[p_nvar];
}

bool Z_Obj::g_name_var_bool(Object *p_obj, int p_nvar, const String &p_true_str, bool p_default) {
	if (p_nvar < 0) {
		return p_default;
	}
	const PackedStringArray vars = get_name_vars(p_obj);
	if (p_nvar >= vars.size()) {
		return p_default;
	}

	const String raw = String(vars[p_nvar]).strip_edges();
	if (raw.is_empty()) {
		return p_default;
	}
	const String s = raw.to_lower();
	const String true_str = p_true_str.strip_edges().to_lower();

	// Explicit true: matches true_str, or common TRUE tokens.
	if ((!true_str.is_empty() && s == true_str) || s == "true" || s == "1" || s == "yes" || s == "y" || s == "on") {
		return true;
	}
	// Explicit false tokens.
	if (s == "false" || s == "0" || s == "no" || s == "n" || s == "off") {
		return false;
	}
	return p_default;
}

int64_t Z_Obj::g_name_var_int(Object *p_obj, int p_nvar, int64_t p_default) {
	if (p_nvar < 0) {
		return p_default;
	}
	const PackedStringArray vars = get_name_vars(p_obj);
	if (p_nvar >= vars.size()) {
		return p_default;
	}
	const String raw = String(vars[p_nvar]).strip_edges();
	if (raw.is_empty()) {
		return p_default;
	}
	// is_valid_int rejects floats; to_int is fine for whole-number name segments.
	if (!raw.is_valid_int()) {
		return p_default;
	}
	return raw.to_int();
}
