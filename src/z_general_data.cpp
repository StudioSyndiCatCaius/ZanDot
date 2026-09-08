#include "z_general_data.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZGeneralData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_title", "title"), &ZGeneralData::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &ZGeneralData::get_title);

	ClassDB::bind_method(D_METHOD("set_icon", "icon"), &ZGeneralData::set_icon);
	ClassDB::bind_method(D_METHOD("get_icon"), &ZGeneralData::get_icon);

	ClassDB::bind_method(D_METHOD("set_description", "description"), &ZGeneralData::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &ZGeneralData::get_description);

	ClassDB::bind_method(D_METHOD("set_tags", "tags"), &ZGeneralData::set_tags);
	ClassDB::bind_method(D_METHOD("get_tags"), &ZGeneralData::get_tags);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &ZGeneralData::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &ZGeneralData::get_color);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_tags", "get_tags");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
}

ZGeneralData::ZGeneralData() {
	title = "";
	description = "";
	color = Color(1, 1, 1, 1);
}

ZGeneralData::~ZGeneralData() {
}

void ZGeneralData::set_title(const String &p_title) {
	title = p_title;
}

String ZGeneralData::get_title() const {
	return title;
}

void ZGeneralData::set_icon(const Ref<Texture2D> &p_icon) {
	icon = p_icon;
}

Ref<Texture2D> ZGeneralData::get_icon() const {
	return icon;
}

void ZGeneralData::set_description(const String &p_description) {
	description = p_description;
}

String ZGeneralData::get_description() const {
	return description;
}

void ZGeneralData::set_tags(const TypedArray<StringName> &p_tags) {
	tags = p_tags;
}

TypedArray<StringName> ZGeneralData::get_tags() const {
	return tags;
}

void ZGeneralData::set_color(const Color &p_color) {
	color = p_color;
}

Color ZGeneralData::get_color() const {
	return color;
}
