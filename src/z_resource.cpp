#include "z_resource.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_title", "title"), &ZResource::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &ZResource::get_title);

	ClassDB::bind_method(D_METHOD("set_icon", "icon"), &ZResource::set_icon);
	ClassDB::bind_method(D_METHOD("get_icon"), &ZResource::get_icon);

	ClassDB::bind_method(D_METHOD("set_description", "description"), &ZResource::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &ZResource::get_description);

	// snake_case property names (inspector shows them as Title / Icon / Description).
	// Capitalized names like "Title" are less reliable for script subclasses + .tres.
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
}

bool ZResource::_set(const StringName &p_name, const Variant &p_value) {
	// Accept both new snake_case and the old "Title" name for existing .tres files.
	if (p_name == StringName("title") || p_name == StringName("Title")) {
		set_title(p_value);
		return true;
	}
	if (p_name == StringName("icon")) {
		set_icon(p_value);
		return true;
	}
	if (p_name == StringName("description")) {
		set_description(p_value);
		return true;
	}
	return false;
}

bool ZResource::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == StringName("title") || p_name == StringName("Title")) {
		r_ret = get_title();
		return true;
	}
	if (p_name == StringName("icon")) {
		r_ret = get_icon();
		return true;
	}
	if (p_name == StringName("description")) {
		r_ret = get_description();
		return true;
	}
	return false;
}

ZResource::ZResource() {
	title = "";
	description = "";
}

ZResource::~ZResource() {
}

void ZResource::set_title(const String &p_title) {
	if (title == p_title) {
		return;
	}
	title = p_title;
	emit_changed();
}

String ZResource::get_title() const {
	return title;
}

void ZResource::set_icon(const Ref<Texture2D> &p_icon) {
	if (icon == p_icon) {
		return;
	}
	icon = p_icon;
	emit_changed();
}

Ref<Texture2D> ZResource::get_icon() const {
	return icon;
}

void ZResource::set_description(const String &p_description) {
	if (description == p_description) {
		return;
	}
	description = p_description;
	emit_changed();
}

String ZResource::get_description() const {
	return description;
}
