#pragma once

#include "z_creature_data.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Abstract base for ZanDot save resources (game + global).
class ZSave : public Resource {
	GDCLASS(ZSave, Resource)

private:
	Dictionary params;
	String date_created;
	String date_last_saved;
	int64_t save_seed = 0;
	// Dictionary[StringName, ZCreatureData] — persisted creature state by id.
	Dictionary creature_data;

protected:
	static void _bind_methods();

public:
	void set_params(const Dictionary &p_params);
	Dictionary get_params() const;

	void set_date_created(const String &p_date);
	String get_date_created() const;

	void set_date_last_saved(const String &p_date);
	String get_date_last_saved() const;

	void set_save_seed(int64_t p_seed);
	int64_t get_save_seed() const;

	void set_creature_data(const Dictionary &p_creature_data);
	Dictionary get_creature_data() const;

	// Convenience accessors for the creature_data dictionary.
	Ref<ZCreatureData> creature_data_get(const StringName &p_id) const;
	void creature_data_set(const StringName &p_id, const Ref<ZCreatureData> &p_data);

	// Initialize dates + random seed for a brand-new save.
	void initialize_new();
	// Stamp date_last_saved to now (call before writing to disk).
	void touch_saved();

	ZSave();
	~ZSave();
};

} // namespace godot
