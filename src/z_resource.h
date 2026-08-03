#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ZResource : public Resource {
	GDCLASS(ZResource, Resource)

private:
	String title;
	Ref<Texture2D> icon;
	String description;

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

	ZResource();
	~ZResource();
};

} // namespace godot
