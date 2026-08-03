#include "z_save.h"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void ZSave::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_params", "params"), &ZSave::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &ZSave::get_params);

	ClassDB::bind_method(D_METHOD("set_date_created", "date"), &ZSave::set_date_created);
	ClassDB::bind_method(D_METHOD("get_date_created"), &ZSave::get_date_created);

	ClassDB::bind_method(D_METHOD("set_date_last_saved", "date"), &ZSave::set_date_last_saved);
	ClassDB::bind_method(D_METHOD("get_date_last_saved"), &ZSave::get_date_last_saved);

	ClassDB::bind_method(D_METHOD("set_save_seed", "seed"), &ZSave::set_save_seed);
	ClassDB::bind_method(D_METHOD("get_save_seed"), &ZSave::get_save_seed);

	ClassDB::bind_method(D_METHOD("initialize_new"), &ZSave::initialize_new);
	ClassDB::bind_method(D_METHOD("touch_saved"), &ZSave::touch_saved);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "params"), "set_params", "get_params");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "date_created"), "set_date_created", "get_date_created");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "date_last_saved"), "set_date_last_saved", "get_date_last_saved");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "save_seed"), "set_save_seed", "get_save_seed");
}

ZSave::ZSave() {
}

ZSave::~ZSave() {
}

void ZSave::set_params(const Dictionary &p_params) {
	params = p_params;
	emit_changed();
}

Dictionary ZSave::get_params() const {
	return params;
}

void ZSave::set_date_created(const String &p_date) {
	if (date_created == p_date) {
		return;
	}
	date_created = p_date;
	emit_changed();
}

String ZSave::get_date_created() const {
	return date_created;
}

void ZSave::set_date_last_saved(const String &p_date) {
	if (date_last_saved == p_date) {
		return;
	}
	date_last_saved = p_date;
	emit_changed();
}

String ZSave::get_date_last_saved() const {
	return date_last_saved;
}

void ZSave::set_save_seed(int64_t p_seed) {
	if (save_seed == p_seed) {
		return;
	}
	save_seed = p_seed;
	emit_changed();
}

int64_t ZSave::get_save_seed() const {
	return save_seed;
}

void ZSave::initialize_new() {
	const String now = Time::get_singleton()->get_datetime_string_from_system(true, true);
	date_created = now;
	date_last_saved = now;
	save_seed = UtilityFunctions::randi();
	emit_changed();
}

void ZSave::touch_saved() {
	date_last_saved = Time::get_singleton()->get_datetime_string_from_system(true, true);
	emit_changed();
}
