#include "z_creature_data.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {

String resource_key_hint(const String &p_key_class, Variant::Type p_value_type, PropertyHint p_value_hint, const String &p_value_class) {
	return String::num_int64(Variant::OBJECT) + "/" + String::num_int64(PROPERTY_HINT_RESOURCE_TYPE) + ":" + p_key_class + ";" +
			String::num_int64(p_value_type) + "/" + String::num_int64(p_value_hint) + ":" + p_value_class;
}

Dictionary as_typed(const Dictionary &p_from, const StringName &p_key_class, Variant::Type p_value_type, const StringName &p_value_class) {
	return Dictionary(p_from, Variant::OBJECT, p_key_class, Variant(), p_value_type, p_value_class, Variant());
}

} // namespace

String ZCreatureData::attributes_hint() {
	return resource_key_hint("ZAttribute", Variant::FLOAT, PROPERTY_HINT_NONE, "");
}

String ZCreatureData::xp_hint() {
	return resource_key_hint("ZLeveler", Variant::FLOAT, PROPERTY_HINT_NONE, "");
}

String ZCreatureData::inventory_hint() {
	return resource_key_hint("Resource", Variant::INT, PROPERTY_HINT_NONE, "");
}

String ZCreatureData::equipment_hint() {
	return resource_key_hint("Resource", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "Resource");
}

void ZCreatureData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rank", "rank"), &ZCreatureData::set_rank);
	ClassDB::bind_method(D_METHOD("get_rank"), &ZCreatureData::get_rank);

	ClassDB::bind_method(D_METHOD("set_attributes", "attributes"), &ZCreatureData::set_attributes);
	ClassDB::bind_method(D_METHOD("get_attributes"), &ZCreatureData::get_attributes);

	ClassDB::bind_method(D_METHOD("set_xp", "xp"), &ZCreatureData::set_xp);
	ClassDB::bind_method(D_METHOD("get_xp"), &ZCreatureData::get_xp);

	ClassDB::bind_method(D_METHOD("set_inventory", "inventory"), &ZCreatureData::set_inventory);
	ClassDB::bind_method(D_METHOD("get_inventory"), &ZCreatureData::get_inventory);

	ClassDB::bind_method(D_METHOD("set_equipment", "equipment"), &ZCreatureData::set_equipment);
	ClassDB::bind_method(D_METHOD("get_equipment"), &ZCreatureData::get_equipment);

	ClassDB::bind_method(D_METHOD("set_params", "params"), &ZCreatureData::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &ZCreatureData::get_params);

	ClassDB::bind_method(D_METHOD("duplicate_data"), &ZCreatureData::duplicate_data);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "rank"), "set_rank", "get_rank");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "attributes", PROPERTY_HINT_DICTIONARY_TYPE, attributes_hint()), "set_attributes", "get_attributes");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "xp", PROPERTY_HINT_DICTIONARY_TYPE, xp_hint()), "set_xp", "get_xp");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "inventory", PROPERTY_HINT_DICTIONARY_TYPE, inventory_hint()), "set_inventory", "get_inventory");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "equipment", PROPERTY_HINT_DICTIONARY_TYPE, equipment_hint()), "set_equipment", "get_equipment");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "params"), "set_params", "get_params");
}

ZCreatureData::ZCreatureData() {
	attributes = as_typed(Dictionary(), StringName("ZAttribute"), Variant::FLOAT, StringName());
	xp = as_typed(Dictionary(), StringName("ZLeveler"), Variant::FLOAT, StringName());
	inventory = as_typed(Dictionary(), StringName("Resource"), Variant::INT, StringName());
	equipment = as_typed(Dictionary(), StringName("Resource"), Variant::OBJECT, StringName("Resource"));
}

ZCreatureData::~ZCreatureData() {
}

void ZCreatureData::set_rank(int p_rank) {
	if (rank == p_rank) {
		return;
	}
	rank = p_rank;
	emit_changed();
}

int ZCreatureData::get_rank() const {
	return rank;
}

void ZCreatureData::set_attributes(const Dictionary &p_attributes) {
	attributes = as_typed(p_attributes, StringName("ZAttribute"), Variant::FLOAT, StringName());
	emit_changed();
}

Dictionary ZCreatureData::get_attributes() const {
	return attributes;
}

void ZCreatureData::set_xp(const Dictionary &p_xp) {
	xp = as_typed(p_xp, StringName("ZLeveler"), Variant::FLOAT, StringName());
	emit_changed();
}

Dictionary ZCreatureData::get_xp() const {
	return xp;
}

void ZCreatureData::set_inventory(const Dictionary &p_inventory) {
	inventory = as_typed(p_inventory, StringName("Resource"), Variant::INT, StringName());
	emit_changed();
}

Dictionary ZCreatureData::get_inventory() const {
	return inventory;
}

void ZCreatureData::set_equipment(const Dictionary &p_equipment) {
	equipment = as_typed(p_equipment, StringName("Resource"), Variant::OBJECT, StringName("Resource"));
	emit_changed();
}

Dictionary ZCreatureData::get_equipment() const {
	return equipment;
}

void ZCreatureData::set_params(const Dictionary &p_params) {
	params = p_params;
	emit_changed();
}

Dictionary ZCreatureData::get_params() const {
	return params;
}

Ref<ZCreatureData> ZCreatureData::duplicate_data() const {
	Ref<ZCreatureData> copy;
	copy.instantiate();
	copy->set_rank(rank);
	copy->set_attributes(attributes.duplicate(false));
	copy->set_xp(xp.duplicate(false));
	copy->set_inventory(inventory.duplicate(false));
	copy->set_equipment(equipment.duplicate(false));
	copy->set_params(params.duplicate(true));
	return copy;
}
