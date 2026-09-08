#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

// Lightweight presentational blob (title / icon / description / tags / color).
// Not a Resource — `ZGeneralData.new()` in scripts / the inspector.
// Any object can expose one via `get_z_data() -> ZGeneralData`; ZResource does this from its params.
class ZGeneralData : public RefCounted {
	GDCLASS(ZGeneralData, RefCounted)

private:
	String title;
	Ref<Texture2D> icon;
	String description;
	TypedArray<StringName> tags;
	Color color = Color(1, 1, 1, 1);

protected:
	static void _bind_methods();

public:
	void set_title(const String &p_title);
	String get_title() const;

	void set_icon(const Ref<Texture2D> &p_icon);
	Ref<Texture2D> get_icon() const;

	void set_description(const String &p_description);
	String get_description() const;

	void set_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_tags() const;

	void set_color(const Color &p_color);
	Color get_color() const;

	ZGeneralData();
	~ZGeneralData();
};

} // namespace godot
