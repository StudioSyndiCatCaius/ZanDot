#pragma once

#include "z_general_data.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Presentable data resource. Params (title / icon / description / tags / color)
// are exposed both directly and via get_z_data() -> ZGeneralData.
// `meta` is a free-form Dictionary for game-specific payload.
class ZResource : public Resource {
	GDCLASS(ZResource, Resource)

private:
	String title;
	Ref<Texture2D> icon;
	String description;
	TypedArray<StringName> tags;
	Color color = Color(1, 1, 1, 1);
	Dictionary meta;

protected:
	static void _bind_methods();

	// Fallback path so property get/set still works when a GDScript (or other
	// extension class) extends ZResource and the inspector routes through
	// the extension class set/get hooks instead of ClassDB method binds.
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;

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

	void set_meta_data(const Dictionary &p_meta);
	Dictionary get_meta_data() const;

	// Snapshot of this resource's presentational params as a ZGeneralData.
	// Convention: any Object may implement get_z_data() the same way.
	Ref<ZGeneralData> get_z_data() const;

	ZResource();
	~ZResource();
};

} // namespace godot
