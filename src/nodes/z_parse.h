#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Stateless helpers for converting Dictionaries to/from common text formats.
class Z_Parse : public RefCounted {
	GDCLASS(Z_Parse, RefCounted)

protected:
	static void _bind_methods();

public:
	// Dictionary → one-row table. Array[Dictionary] → multi-row table.
	// Nested Array/Dictionary cells are JSON.
	static String to_csv(const Variant &p_data);
	static TypedArray<Dictionary> from_csv(const String &p_str);

	static String to_yaml(const Dictionary &p_data);
	static Dictionary from_yaml(const String &p_str);

	static String to_ini(const Dictionary &p_data);
	static Dictionary from_ini(const String &p_str);

	static String to_toml(const Dictionary &p_data);
	static Dictionary from_toml(const String &p_str);
};

} // namespace godot
