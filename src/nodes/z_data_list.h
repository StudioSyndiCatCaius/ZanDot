#pragma once

#include "z_data_option.h"

#include <godot_cpp/classes/box_container.hpp>
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class ZPlayer;

// Dynamic list widget (Control). Options are laid out with real Godot containers
// (VBox / HBox / Grid), not manual positioning.
// Selection is per-ZPlayer (one at a time); many options may be highlighted.
class ZDataList : public Control {
	GDCLASS(ZDataList, Control)

public:
	enum EListLayout {
		LIST_LAYOUT_VERTICAL = 0,
		LIST_LAYOUT_HORIZONTAL = 1,
	};

	// Mirrors BoxContainer::AlignmentMode.
	enum EAlign {
		ALIGN_BEGIN = 0,
		ALIGN_CENTER = 1,
		ALIGN_END = 2,
	};

private:
	bool clear_list_on_ready = false;
	ObjectID list_owner_id;
	Ref<PackedScene> option_scene;
	int player_index = 0;

	EListLayout list_layout = LIST_LAYOUT_VERTICAL;
	// Columns when vertical, rows when horizontal.
	int sections = 1;
	bool scale_sections_to_size = false;
	bool is_scrollable = false;
	Array default_options;

	EAlign horizontal_alignment = ALIGN_BEGIN;
	EAlign vertical_alignment = ALIGN_BEGIN;
	bool expand_horizontal = false;
	bool expand_vertical = false;

	Callable option_is_selectable;
	Callable option_is_visible;

	Vector<ObjectID> option_ids;
	ObjectID hovered_option_id;

	ScrollContainer *scroll_host = nullptr;
	// Options parent under content_host (VBox, HBox, or Grid).
	Container *content_host = nullptr;
	int cached_content_kind = -1; // 0=VBox 1=HBox 2=Grid 3=HBox-of-VBoxes
	int cached_grid_columns = -1;
	bool layout_queued = false;

	Node *_resolve_list_root();
	void _detach_all_mounts(Vector<Control *> &r_mounts);
	void _parent_mounts_in_order(Node *p_parent, const Vector<Control *> &p_mounts);
	Container *_create_box_or_grid(int p_kind, int p_columns);
	void _set_content_host(Container *p_new_host, bool p_under_scroll);
	void _ensure_structure(int p_effective_sections, int p_visible_count);
	void _apply_box_alignment();
	void _apply_mount_size_flags(Control *p_mount) const;
	void _apply_all_mount_size_flags();
	ZPlayer *_resolve_player() const;
	void _prune_options();
	int _index_of(ZDataOption *p_option) const;
	ZDataOption *_option_at(int p_index) const;
	Control *_mount_control_for(ZDataOption *p_option) const;
	bool _eval_selectable(ZDataOption *p_option, int p_index) const;
	bool _eval_visible(ZDataOption *p_option, int p_index) const;
	void _refresh_option_filters(ZDataOption *p_option, int p_index);
	void _refresh_all_filters();
	void _register_existing_children();
	void _unlink_option(ZDataOption *p_option, int p_index, bool p_free);
	int _effective_sections(const Vector2 &p_avail, const Vector2 &p_sample) const;
	Vector2 _sample_option_size() const;
	int _count_visible_mounts() const;
	void _queue_relayout();
	void _relayout();
	void _apply_default_options_if_empty();
	void _clear_preview_options();
	void _refresh_editor_preview();
	static bool _is_preview_option(ZDataOption *p_option);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_clear_list_on_ready(bool p_clear);
	bool get_clear_list_on_ready() const;

	void set_list_owner(Object *p_owner);
	Object *get_list_owner() const;

	void set_option_scene(const Ref<PackedScene> &p_scene);
	Ref<PackedScene> get_option_scene() const;

	void set_player_index(int p_index);
	int get_player_index() const;

	void set_list_layout(EListLayout p_layout);
	EListLayout get_list_layout() const;

	void set_sections(int p_sections);
	int get_sections() const;

	void set_scale_sections_to_size(bool p_enabled);
	bool get_scale_sections_to_size() const;

	void set_is_scrollable(bool p_scrollable);
	bool get_is_scrollable() const;

	void set_default_options(const Array &p_options);
	Array get_default_options() const;

	void set_horizontal_alignment(EAlign p_align);
	EAlign get_horizontal_alignment() const;

	void set_vertical_alignment(EAlign p_align);
	EAlign get_vertical_alignment() const;

	void set_expand_horizontal(bool p_expand);
	bool get_expand_horizontal() const;

	void set_expand_vertical(bool p_expand);
	bool get_expand_vertical() const;

	void set_option_is_selectable(const Callable &p_callable);
	Callable get_option_is_selectable() const;

	void set_option_is_visible(const Callable &p_callable);
	Callable get_option_is_visible() const;

	ZDataOption *option_add(Object *p_option);
	void option_add_list(const Array &p_options, bool p_clear_first = false);
	void option_clear_all();
	void option_remove(int p_index);

	void option_select(int p_index);
	void option_hover(int p_index);
	void option_highlight(int p_index, bool p_is_highlighted);

	int get_option_count() const;
	ZDataOption *get_option(int p_index) const;
	TypedArray<ZDataOption> get_options() const;
	int find_option_index(ZDataOption *p_option) const;

	void select_option_node(ZDataOption *p_option);

	ZDataList();
	~ZDataList();
};

} // namespace godot

VARIANT_ENUM_CAST(ZDataList::EListLayout);
VARIANT_ENUM_CAST(ZDataList::EAlign);
