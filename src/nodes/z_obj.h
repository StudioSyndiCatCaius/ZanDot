#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// Stateless object helpers (name-var parsing, etc.).
class Z_Obj : public RefCounted {
	GDCLASS(Z_Obj, RefCounted)

protected:
	static void _bind_methods();

public:
	// Resolve a display/file name for an Object (Resource path basename, Node name, …).
	static String resolve_object_name(Object *p_obj);

	// Split object name by delimiter into vars. Optionally cached on Z (project setting).
	static PackedStringArray get_name_vars(Object *p_obj, const String &p_delimiter = "_", bool p_force_recache = false);

	// Typed accessors for a single name-var index (0-based). Out-of-range → default.
	static String g_name_var_string(Object *p_obj, int p_nvar, const String &p_default = String());
	static bool g_name_var_bool(Object *p_obj, int p_nvar, const String &p_true_str = "true", bool p_default = false);
	static int64_t g_name_var_int(Object *p_obj, int p_nvar, int64_t p_default = 0);

	Z_Obj();
	~Z_Obj();
};

} // namespace godot
