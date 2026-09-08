#pragma once

#include "../z_general_data.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class ZDataList;

// One list row as a Control. Optional widget links fill from
// source_object.get_z_data(), with override_title / override_icon taking priority.
// Click/hover use select_button when set, otherwise the Control itself.
class ZDataOption : public Control {
	GDCLASS(ZDataOption, Control)

private:
	ObjectID source_object_id;
	ObjectID list_id;
	Button *select_button = nullptr;
	Label *label_title = nullptr;
	TextureRect *img_icon = nullptr;
	RichTextLabel *label_description = nullptr;
	bool use_button_icon = false;
	// When true, select_button.text is filled from title (override / get_z_data).
	bool use_button_text = false;
	bool is_highlighted = false;
	bool is_hovered = false;
	bool button_connected = false;
	bool self_input_connected = false;
	// Manual overrides (non-empty / valid wins over get_z_data()).
	String override_title;
	Ref<Texture2D> override_icon;

	void _disconnect_select_button();
	void _connect_select_button();
	void _disconnect_self_input();
	void _connect_self_input();
	void _refresh_input_wiring();
	void _on_select_button_pressed();
	void _on_mouse_entered();
	void _on_mouse_exited();
	void _apply_source_visuals();
	void _apply_color(const Color &p_color);
	Ref<ZGeneralData> _fetch_z_data() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	// C++ override points (GDScript: on_added_to_list / on_source_object_changed / …).
	virtual void on_added_to_list(ZDataList *p_list, int p_index, Object *p_src_object);
	virtual void on_source_object_changed(Object *p_new_object);
	virtual void on_list_owner_changed(ZDataList *p_list, Object *p_new_owner);
	virtual bool on_get_is_selectable() const;

public:
	// Public so godot-cpp can bind the Control virtual.
	void _gui_input(const Ref<InputEvent> &p_event) override;
	void set_source_object(Object *p_object);
	Object *get_source_object() const;

	void set_select_button(Button *p_button);
	Button *get_select_button() const;

	void set_label_title(Label *p_label);
	Label *get_label_title() const;

	void set_img_icon(TextureRect *p_icon);
	TextureRect *get_img_icon() const;

	void set_label_description(RichTextLabel *p_label);
	RichTextLabel *get_label_description() const;

	void set_use_button_icon(bool p_enabled);
	bool get_use_button_icon() const;

	void set_use_button_text(bool p_enabled);
	bool get_use_button_text() const;

	void set_override_title(const String &p_title);
	String get_override_title() const;

	void set_override_icon(const Ref<Texture2D> &p_icon);
	Ref<Texture2D> get_override_icon() const;

	void set_is_highlighted(bool p_highlighted);
	bool get_is_highlighted() const;

	bool get_is_hovered() const;
	// Used by ZDataList for keyboard/gamepad hover.
	void set_hovered(bool p_hovered);

	// Show/hide this option row (Control visibility).
	void set_option_visible(bool p_visible);

	// Non-overridable: asks the owning list to select this option.
	void select();

	// Selectability for list filters (default true). Override in GDScript as
	// _get_is_selectable().
	bool get_is_selectable() const;

	ZDataList *get_list() const;
	// Called by ZDataList — not for game code.
	void _bind_to_list(ZDataList *p_list);
	void _notify_added_to_list(ZDataList *p_list, int p_index, Object *p_src_object);
	void _notify_list_owner_changed(ZDataList *p_list, Object *p_new_owner);
	void _notify_source_applied();

	ZDataOption();
	~ZDataOption();
};

} // namespace godot
