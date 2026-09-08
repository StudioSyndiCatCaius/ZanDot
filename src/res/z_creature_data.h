#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// The serializable state of a ZCreature: stats, XP, bags and gear.
// Kept off the node so it can be written straight into a ZSave and handed back
// to a freshly spawned creature.
class ZCreatureData : public Resource {
	GDCLASS(ZCreatureData, Resource)

private:
	int rank = 0;
	// Dictionary[ZAttribute, float] — current value per attribute.
	Dictionary attributes;
	// Dictionary[ZLeveler, float] — total XP per leveller.
	Dictionary xp;
	// Dictionary[Resource, int] — item resource to held count.
	Dictionary inventory;
	// Dictionary[Resource, Resource] — slot resource to the resource equipped in it.
	Dictionary equipment;
	Dictionary params;

protected:
	static void _bind_methods();

public:
	void set_rank(int p_rank);
	int get_rank() const;

	void set_attributes(const Dictionary &p_attributes);
	Dictionary get_attributes() const;

	void set_xp(const Dictionary &p_xp);
	Dictionary get_xp() const;

	void set_inventory(const Dictionary &p_inventory);
	Dictionary get_inventory() const;

	void set_equipment(const Dictionary &p_equipment);
	Dictionary get_equipment() const;

	void set_params(const Dictionary &p_params);
	Dictionary get_params() const;

	// Deep-ish copy: new dictionaries, same resource keys/values.
	Ref<ZCreatureData> duplicate_data() const;

	// Typed-dictionary hint strings, shared with ZSave's creature_data property.
	static String attributes_hint();
	static String xp_hint();
	static String inventory_hint();
	static String equipment_hint();

	ZCreatureData();
	~ZCreatureData();
};

} // namespace godot
