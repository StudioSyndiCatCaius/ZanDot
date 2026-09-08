#include "z_data_list.h"

#include "z.h"
#include "../res/z_player.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/grid_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
const char *const PREVIEW_META = "z_data_list_preview";

enum ContentKind {
	KIND_VBOX = 0,
	KIND_HBOX = 1,
	KIND_GRID = 2,
	KIND_HBOX_OF_VBOX = 3,
};
} // namespace

void ZDataList::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_clear_list_on_ready", "clear"), &ZDataList::set_clear_list_on_ready);
	ClassDB::bind_method(D_METHOD("get_clear_list_on_ready"), &ZDataList::get_clear_list_on_ready);

	ClassDB::bind_method(D_METHOD("set_list_owner", "owner"), &ZDataList::set_list_owner);
	ClassDB::bind_method(D_METHOD("get_list_owner"), &ZDataList::get_list_owner);

	ClassDB::bind_method(D_METHOD("set_option_scene", "scene"), &ZDataList::set_option_scene);
	ClassDB::bind_method(D_METHOD("get_option_scene"), &ZDataList::get_option_scene);

	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZDataList::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZDataList::get_player_index);

	ClassDB::bind_method(D_METHOD("set_list_layout", "layout"), &ZDataList::set_list_layout);
	ClassDB::bind_method(D_METHOD("get_list_layout"), &ZDataList::get_list_layout);

	ClassDB::bind_method(D_METHOD("set_sections", "sections"), &ZDataList::set_sections);
	ClassDB::bind_method(D_METHOD("get_sections"), &ZDataList::get_sections);

	ClassDB::bind_method(D_METHOD("set_scale_sections_to_size", "enabled"), &ZDataList::set_scale_sections_to_size);
	ClassDB::bind_method(D_METHOD("get_scale_sections_to_size"), &ZDataList::get_scale_sections_to_size);

	ClassDB::bind_method(D_METHOD("set_is_scrollable", "scrollable"), &ZDataList::set_is_scrollable);
	ClassDB::bind_method(D_METHOD("get_is_scrollable"), &ZDataList::get_is_scrollable);

	ClassDB::bind_method(D_METHOD("set_default_options", "options"), &ZDataList::set_default_options);
	ClassDB::bind_method(D_METHOD("get_default_options"), &ZDataList::get_default_options);

	ClassDB::bind_method(D_METHOD("set_horizontal_alignment", "alignment"), &ZDataList::set_horizontal_alignment);
	ClassDB::bind_method(D_METHOD("get_horizontal_alignment"), &ZDataList::get_horizontal_alignment);

	ClassDB::bind_method(D_METHOD("set_vertical_alignment", "alignment"), &ZDataList::set_vertical_alignment);
	ClassDB::bind_method(D_METHOD("get_vertical_alignment"), &ZDataList::get_vertical_alignment);

	ClassDB::bind_method(D_METHOD("set_expand_horizontal", "expand"), &ZDataList::set_expand_horizontal);
	ClassDB::bind_method(D_METHOD("get_expand_horizontal"), &ZDataList::get_expand_horizontal);

	ClassDB::bind_method(D_METHOD("set_expand_vertical", "expand"), &ZDataList::set_expand_vertical);
	ClassDB::bind_method(D_METHOD("get_expand_vertical"), &ZDataList::get_expand_vertical);

	ClassDB::bind_method(D_METHOD("set_option_is_selectable", "callable"), &ZDataList::set_option_is_selectable);
	ClassDB::bind_method(D_METHOD("get_option_is_selectable"), &ZDataList::get_option_is_selectable);
	ClassDB::bind_method(D_METHOD("set_option_is_visible", "callable"), &ZDataList::set_option_is_visible);
	ClassDB::bind_method(D_METHOD("get_option_is_visible"), &ZDataList::get_option_is_visible);

	ClassDB::bind_method(D_METHOD("option_add", "option"), &ZDataList::option_add);
	ClassDB::bind_method(D_METHOD("option_add_list", "options", "clear_first"), &ZDataList::option_add_list, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("option_clear_all"), &ZDataList::option_clear_all);
	ClassDB::bind_method(D_METHOD("option_remove", "index"), &ZDataList::option_remove);

	ClassDB::bind_method(D_METHOD("option_select", "index"), &ZDataList::option_select);
	ClassDB::bind_method(D_METHOD("option_hover", "index"), &ZDataList::option_hover);
	ClassDB::bind_method(D_METHOD("option_highlight", "index", "is_highlighted"), &ZDataList::option_highlight);

	ClassDB::bind_method(D_METHOD("get_option_count"), &ZDataList::get_option_count);
	ClassDB::bind_method(D_METHOD("get_option", "index"), &ZDataList::get_option);
	ClassDB::bind_method(D_METHOD("get_options"), &ZDataList::get_options);
	ClassDB::bind_method(D_METHOD("find_option_index", "option"), &ZDataList::find_option_index);

	ClassDB::bind_method(D_METHOD("Option_Add", "option"), &ZDataList::option_add);
	ClassDB::bind_method(D_METHOD("Option_Add_List", "options", "clear_first"), &ZDataList::option_add_list, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("Option_Clear_All"), &ZDataList::option_clear_all);
	ClassDB::bind_method(D_METHOD("Option_Select", "index"), &ZDataList::option_select);
	ClassDB::bind_method(D_METHOD("Option_Hover", "index"), &ZDataList::option_hover);
	ClassDB::bind_method(D_METHOD("Option_Highlight", "index", "is_highlighted"), &ZDataList::option_highlight);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clear_list_on_ready"), "set_clear_list_on_ready", "get_clear_list_on_ready");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "list_owner"), "set_list_owner", "get_list_owner");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "option_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_option_scene", "get_option_scene");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index", PROPERTY_HINT_RANGE, "0,7,1"), "set_player_index", "get_player_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "list_layout", PROPERTY_HINT_ENUM, "Vertical,Horizontal"), "set_list_layout", "get_list_layout");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sections", PROPERTY_HINT_RANGE, "1,64,1"), "set_sections", "get_sections");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scale_sections_to_size"), "set_scale_sections_to_size", "get_scale_sections_to_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_scrollable"), "set_is_scrollable", "get_is_scrollable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "horizontal_alignment", PROPERTY_HINT_ENUM, "Begin,Center,End"), "set_horizontal_alignment", "get_horizontal_alignment");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_alignment", PROPERTY_HINT_ENUM, "Begin,Center,End"), "set_vertical_alignment", "get_vertical_alignment");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "expand_horizontal"), "set_expand_horizontal", "get_expand_horizontal");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "expand_vertical"), "set_expand_vertical", "get_expand_vertical");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "default_options", PROPERTY_HINT_ARRAY_TYPE, "Resource"), "set_default_options", "get_default_options");

	BIND_ENUM_CONSTANT(LIST_LAYOUT_VERTICAL);
	BIND_ENUM_CONSTANT(LIST_LAYOUT_HORIZONTAL);
	BIND_ENUM_CONSTANT(ALIGN_BEGIN);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_END);

	ADD_SIGNAL(MethodInfo("on_option_selected", PropertyInfo(Variant::OBJECT, "option", PROPERTY_HINT_NODE_TYPE, "ZDataOption"), PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("on_option_hovered", PropertyInfo(Variant::OBJECT, "option", PROPERTY_HINT_NODE_TYPE, "ZDataOption"), PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("on_option_highlighted", PropertyInfo(Variant::OBJECT, "option", PROPERTY_HINT_NODE_TYPE, "ZDataOption"), PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::BOOL, "is_highlighted")));
	ADD_SIGNAL(MethodInfo("on_option_added", PropertyInfo(Variant::OBJECT, "option", PROPERTY_HINT_NODE_TYPE, "ZDataOption"), PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("on_option_removed", PropertyInfo(Variant::OBJECT, "option", PROPERTY_HINT_NODE_TYPE, "ZDataOption"), PropertyInfo(Variant::INT, "index")));
}

ZDataList::ZDataList() {
	set_clip_contents(true);
}

ZDataList::~ZDataList() {
}

void ZDataList::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_queue_relayout();
			if (Engine::get_singleton()->is_editor_hint()) {
				// Structure first so adoption has a real VBox/HBox parent.
				_relayout();
				_register_existing_children();
				_refresh_editor_preview();
				_relayout();
			} else {
				_relayout();
				if (clear_list_on_ready) {
					option_clear_all();
				} else {
					_register_existing_children();
				}
				_apply_default_options_if_empty();
				_refresh_all_filters();
				_relayout();
			}
		} break;
		case NOTIFICATION_RESIZED: {
			if (scale_sections_to_size) {
				_queue_relayout();
			}
		} break;
		default:
			break;
	}
}

void ZDataList::_detach_all_mounts(Vector<Control *> &r_mounts) {
	r_mounts.clear();
	_prune_options();
	for (int i = 0; i < option_ids.size(); i++) {
		Control *m = _mount_control_for(_option_at(i));
		if (m == nullptr) {
			continue;
		}
		if (m->get_parent() != nullptr) {
			m->get_parent()->remove_child(m);
		}
		r_mounts.push_back(m);
	}
	// Also strip leftover children from content (column vboxes etc.).
	if (content_host != nullptr) {
		while (content_host->get_child_count() > 0) {
			Node *c = content_host->get_child(0);
			content_host->remove_child(c);
			// Only free non-option helper nodes.
			bool is_mount = false;
			for (int i = 0; i < r_mounts.size(); i++) {
				if (r_mounts[i] == c) {
					is_mount = true;
					break;
				}
			}
			if (!is_mount) {
				// Might still be a mount we missed — if it contains ZDataOption keep it.
				ZDataOption *opt = Object::cast_to<ZDataOption>(c);
				if (opt == nullptr) {
					TypedArray<Node> found = c->find_children("*", "ZDataOption", true, false);
					if (found.size() > 0) {
						opt = Object::cast_to<ZDataOption>(found[0]);
					}
				}
				if (opt != nullptr) {
					Control *ctrl = Object::cast_to<Control>(c);
					if (ctrl != nullptr) {
						r_mounts.push_back(ctrl);
					}
				} else {
					c->queue_free();
				}
			}
		}
	}
}

void ZDataList::_parent_mounts_in_order(Node *p_parent, const Vector<Control *> &p_mounts) {
	if (p_parent == nullptr) {
		return;
	}
	for (int i = 0; i < p_mounts.size(); i++) {
		Control *m = p_mounts[i];
		if (m == nullptr) {
			continue;
		}
		if (m->get_parent() != p_parent) {
			if (m->get_parent() != nullptr) {
				m->get_parent()->remove_child(m);
			}
			p_parent->add_child(m);
		}
		_apply_mount_size_flags(m);
	}
}

Container *ZDataList::_create_box_or_grid(int p_kind, int p_columns) {
	Container *c = nullptr;
	if (p_kind == KIND_VBOX || p_kind == KIND_HBOX_OF_VBOX) {
		// HBOX_OF_VBOX outer shell is an HBox; columns added later.
		if (p_kind == KIND_HBOX_OF_VBOX) {
			HBoxContainer *box = memnew(HBoxContainer);
			box->set_name("_z_content");
			c = box;
		} else {
			VBoxContainer *box = memnew(VBoxContainer);
			box->set_name("_z_content");
			c = box;
		}
	} else if (p_kind == KIND_HBOX) {
		HBoxContainer *box = memnew(HBoxContainer);
		box->set_name("_z_content");
		c = box;
	} else {
		GridContainer *grid = memnew(GridContainer);
		grid->set_name("_z_content");
		grid->set_columns(Math::max(1, p_columns));
		c = grid;
	}
	c->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	c->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	return c;
}

void ZDataList::_set_content_host(Container *p_new_host, bool p_under_scroll) {
	if (content_host != nullptr) {
		if (content_host->get_parent() != nullptr) {
			content_host->get_parent()->remove_child(content_host);
		}
		content_host->queue_free();
		content_host = nullptr;
	}
	content_host = p_new_host;
	if (p_under_scroll) {
		if (scroll_host == nullptr) {
			scroll_host = memnew(ScrollContainer);
			scroll_host->set_name("_z_scroll");
			scroll_host->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
			scroll_host->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_host->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			add_child(scroll_host, false, Node::INTERNAL_MODE_BACK);
		}
		scroll_host->add_child(content_host);
	} else {
		if (scroll_host != nullptr) {
			if (scroll_host->get_parent() == this) {
				remove_child(scroll_host);
			}
			scroll_host->queue_free();
			scroll_host = nullptr;
		}
		content_host->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		add_child(content_host, false, Node::INTERNAL_MODE_BACK);
	}
}

void ZDataList::_ensure_structure(int p_effective_sections, int p_visible_count) {
	const int sec = Math::max(1, p_effective_sections);

	int want_kind = KIND_VBOX;
	int want_cols = 1;

	if (list_layout == LIST_LAYOUT_VERTICAL) {
		if (sec <= 1) {
			want_kind = KIND_VBOX;
		} else {
			want_kind = KIND_GRID;
			want_cols = sec;
		}
	} else {
		if (sec <= 1) {
			want_kind = KIND_HBOX;
		} else {
			// Horizontal multi-row: HBox of VBoxes (column-major).
			want_kind = KIND_HBOX_OF_VBOX;
			want_cols = p_visible_count > 0 ? Math::max(1, (p_visible_count + sec - 1) / sec) : 1;
		}
	}

	const bool scroll_ok = (scroll_host != nullptr) == is_scrollable;
	const bool kind_ok = content_host != nullptr && cached_content_kind == want_kind &&
			(want_kind != KIND_GRID || cached_grid_columns == want_cols);

	if (scroll_ok && kind_ok) {
		_apply_box_alignment();
		if (is_scrollable && scroll_host != nullptr) {
			if (list_layout == LIST_LAYOUT_VERTICAL) {
				scroll_host->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
				scroll_host->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_AUTO);
			} else {
				scroll_host->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_AUTO);
				scroll_host->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			}
		}
		return;
	}

	Vector<Control *> mounts;
	_detach_all_mounts(mounts);

	// Drop old scroll/content.
	if (scroll_host != nullptr && scroll_host->get_parent() == this) {
		remove_child(scroll_host);
		scroll_host->queue_free();
		scroll_host = nullptr;
	}
	if (content_host != nullptr) {
		if (content_host->get_parent() != nullptr) {
			content_host->get_parent()->remove_child(content_host);
		}
		content_host->queue_free();
		content_host = nullptr;
	}

	Container *fresh = _create_box_or_grid(want_kind, want_cols);
	_set_content_host(fresh, is_scrollable);
	cached_content_kind = want_kind;
	cached_grid_columns = want_cols;

	if (is_scrollable && scroll_host != nullptr) {
		if (list_layout == LIST_LAYOUT_VERTICAL) {
			scroll_host->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			scroll_host->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_AUTO);
		} else {
			scroll_host->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_AUTO);
			scroll_host->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
		}
	}

	if (want_kind == KIND_HBOX_OF_VBOX) {
		HBoxContainer *hbox = Object::cast_to<HBoxContainer>(content_host);
		// Split visible mounts into columns of `sec` rows; keep hidden after.
		Vector<Control *> visible;
		Vector<Control *> hidden;
		for (int i = 0; i < mounts.size(); i++) {
			if (mounts[i] != nullptr && mounts[i]->is_visible()) {
				visible.push_back(mounts[i]);
			} else if (mounts[i] != nullptr) {
				hidden.push_back(mounts[i]);
			}
		}
		const int n = visible.size();
		const int cols = n > 0 ? Math::max(1, (n + sec - 1) / sec) : 0;
		for (int col = 0; col < cols; col++) {
			VBoxContainer *col_box = memnew(VBoxContainer);
			col_box->set_name(String("_z_col_") + String::num_int64(col));
			col_box->set_alignment(static_cast<BoxContainer::AlignmentMode>(vertical_alignment));
			col_box->set_h_size_flags(expand_horizontal ? Control::SIZE_EXPAND_FILL : Control::SIZE_SHRINK_BEGIN);
			col_box->set_v_size_flags(expand_vertical ? Control::SIZE_EXPAND_FILL : Control::SIZE_SHRINK_BEGIN);
			hbox->add_child(col_box);
			for (int row = 0; row < sec; row++) {
				const int idx = col * sec + row;
				if (idx >= n) {
					break;
				}
				col_box->add_child(visible[idx]);
				_apply_mount_size_flags(visible[idx]);
			}
		}
		for (int i = 0; i < hidden.size(); i++) {
			hbox->add_child(hidden[i]);
		}
	} else {
		_parent_mounts_in_order(content_host, mounts);
		if (want_kind == KIND_GRID) {
			GridContainer *grid = Object::cast_to<GridContainer>(content_host);
			if (grid != nullptr) {
				grid->set_columns(Math::max(1, want_cols));
			}
		}
	}

	_apply_box_alignment();
}

void ZDataList::_apply_box_alignment() {
	if (content_host == nullptr) {
		return;
	}
	if (VBoxContainer *vbox = Object::cast_to<VBoxContainer>(content_host)) {
		vbox->set_alignment(static_cast<BoxContainer::AlignmentMode>(vertical_alignment));
		return;
	}
	if (HBoxContainer *hbox = Object::cast_to<HBoxContainer>(content_host)) {
		hbox->set_alignment(static_cast<BoxContainer::AlignmentMode>(horizontal_alignment));
		// Nested column VBoxes (horizontal multi-section).
		for (int i = 0; i < hbox->get_child_count(); i++) {
			if (VBoxContainer *col = Object::cast_to<VBoxContainer>(hbox->get_child(i))) {
				col->set_alignment(static_cast<BoxContainer::AlignmentMode>(vertical_alignment));
			}
		}
	}
}

void ZDataList::_apply_mount_size_flags(Control *p_mount) const {
	if (p_mount == nullptr) {
		return;
	}
	BitField<Control::SizeFlags> h = Control::SIZE_SHRINK_BEGIN;
	BitField<Control::SizeFlags> v = Control::SIZE_SHRINK_BEGIN;

	if (expand_horizontal) {
		h = Control::SIZE_EXPAND_FILL;
	} else if (horizontal_alignment == ALIGN_CENTER) {
		h = Control::SIZE_SHRINK_CENTER;
	} else if (horizontal_alignment == ALIGN_END) {
		h = Control::SIZE_SHRINK_END;
	} else {
		h = Control::SIZE_SHRINK_BEGIN;
	}

	if (expand_vertical) {
		v = Control::SIZE_EXPAND_FILL;
	} else if (vertical_alignment == ALIGN_CENTER) {
		v = Control::SIZE_SHRINK_CENTER;
	} else if (vertical_alignment == ALIGN_END) {
		v = Control::SIZE_SHRINK_END;
	} else {
		v = Control::SIZE_SHRINK_BEGIN;
	}

	p_mount->set_h_size_flags(h);
	p_mount->set_v_size_flags(v);
}

void ZDataList::_apply_all_mount_size_flags() {
	_prune_options();
	for (int i = 0; i < option_ids.size(); i++) {
		_apply_mount_size_flags(_mount_control_for(_option_at(i)));
	}
}

Node *ZDataList::_resolve_list_root() {
	if (content_host == nullptr) {
		// Bootstrap with defaults so option_add always has a parent.
		_ensure_structure(Math::max(1, sections), 0);
	}
	if (content_host != nullptr) {
		// For HBox-of-VBoxes, new mounts go into content_host temporarily;
		// _relayout redistributes into column VBoxes.
		return content_host;
	}
	return this;
}

ZPlayer *ZDataList::_resolve_player() const {
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return nullptr;
	}
	TypedArray<ZPlayer> players = z->get_players();
	if (player_index < 0 || player_index >= players.size()) {
		return nullptr;
	}
	return Object::cast_to<ZPlayer>(players[player_index]);
}

void ZDataList::_prune_options() {
	for (int i = option_ids.size() - 1; i >= 0; i--) {
		if (ObjectDB::get_instance(option_ids[i]) == nullptr) {
			option_ids.remove_at(i);
		}
	}
}

int ZDataList::_index_of(ZDataOption *p_option) const {
	if (p_option == nullptr) {
		return -1;
	}
	const ObjectID id(p_option->get_instance_id());
	for (int i = 0; i < option_ids.size(); i++) {
		if (option_ids[i] == id) {
			return i;
		}
	}
	return -1;
}

ZDataOption *ZDataList::_option_at(int p_index) const {
	if (p_index < 0 || p_index >= option_ids.size()) {
		return nullptr;
	}
	return Object::cast_to<ZDataOption>(ObjectDB::get_instance(option_ids[p_index]));
}

Control *ZDataList::_mount_control_for(ZDataOption *p_option) const {
	if (p_option == nullptr) {
		return nullptr;
	}
	// Walk up until we hit content_host, a column VBox under it, or lose parent.
	Node *walk = p_option;
	Control *best = Object::cast_to<Control>(p_option);
	while (walk->get_parent() != nullptr) {
		Node *parent = walk->get_parent();
		if (parent == content_host) {
			return Object::cast_to<Control>(walk);
		}
		// Column VBox under HBox content.
		if (parent->get_parent() == content_host && Object::cast_to<VBoxContainer>(parent) != nullptr) {
			return Object::cast_to<Control>(walk);
		}
		if (Object::cast_to<Control>(walk) != nullptr) {
			best = Object::cast_to<Control>(walk);
		}
		walk = parent;
	}
	return best;
}

bool ZDataList::_eval_selectable(ZDataOption *p_option, int p_index) const {
	if (p_option == nullptr) {
		return false;
	}
	if (!p_option->get_is_selectable()) {
		return false;
	}
	if (option_is_selectable.is_valid()) {
		Variant result = option_is_selectable.call(p_option, p_index);
		if (result.get_type() == Variant::BOOL && !(bool)result) {
			return false;
		}
	}
	return true;
}

bool ZDataList::_eval_visible(ZDataOption *p_option, int p_index) const {
	if (p_option == nullptr) {
		return false;
	}
	if (option_is_visible.is_valid()) {
		Variant result = option_is_visible.call(p_option, p_index);
		if (result.get_type() == Variant::BOOL) {
			return (bool)result;
		}
	}
	return true;
}

void ZDataList::_refresh_option_filters(ZDataOption *p_option, int p_index) {
	if (p_option == nullptr) {
		return;
	}
	const bool vis = _eval_visible(p_option, p_index);
	p_option->set_option_visible(vis);
	Control *mount = _mount_control_for(p_option);
	if (mount != nullptr && mount != p_option) {
		mount->set_visible(vis);
	}
}

void ZDataList::_refresh_all_filters() {
	_prune_options();
	for (int i = 0; i < option_ids.size(); i++) {
		_refresh_option_filters(_option_at(i), i);
	}
	_queue_relayout();
}

void ZDataList::_register_existing_children() {
	Node *root = _resolve_list_root();
	Vector<Node *> candidates;
	if (root != nullptr) {
		for (int i = 0; i < root->get_child_count(); i++) {
			candidates.push_back(root->get_child(i));
		}
	}
	for (int i = 0; i < get_child_count(); i++) {
		Node *c = get_child(i);
		if (c == scroll_host || c == content_host) {
			continue;
		}
		candidates.push_back(c);
	}

	for (int i = 0; i < candidates.size(); i++) {
		Node *child = candidates[i];
		if (child == nullptr) {
			continue;
		}
		// Skip internal column helpers.
		if (child->get_name().begins_with("_z_col_")) {
			for (int j = 0; j < child->get_child_count(); j++) {
				candidates.push_back(child->get_child(j));
			}
			continue;
		}
		ZDataOption *opt = Object::cast_to<ZDataOption>(child);
		Node *mount = child;
		if (opt == nullptr) {
			TypedArray<Node> found = child->find_children("*", "ZDataOption", true, false);
			if (found.size() > 0) {
				opt = Object::cast_to<ZDataOption>(found[0]);
				mount = child;
			}
		}
		if (opt == nullptr || _index_of(opt) >= 0) {
			continue;
		}
		if (mount->get_parent() != root && root != nullptr) {
			if (mount->get_parent() != nullptr) {
				mount->get_parent()->remove_child(mount);
			}
			root->add_child(mount);
		}
		_apply_mount_size_flags(Object::cast_to<Control>(mount));
		option_ids.push_back(ObjectID(opt->get_instance_id()));
		const int idx = option_ids.size() - 1;
		opt->_notify_added_to_list(this, idx, opt->get_source_object());
		emit_signal("on_option_added", opt, idx);
	}
}

void ZDataList::_unlink_option(ZDataOption *p_option, int p_index, bool p_free) {
	if (p_option == nullptr) {
		return;
	}

	ZPlayer *player = _resolve_player();
	if (player != nullptr && player->get_selected_data_option() == p_option) {
		player->set_selected_data_option(nullptr);
	}

	if (hovered_option_id == ObjectID(p_option->get_instance_id())) {
		hovered_option_id = ObjectID();
	}

	emit_signal("on_option_removed", p_option, p_index);
	p_option->_bind_to_list(nullptr);

	if (p_free) {
		Control *mount = _mount_control_for(p_option);
		Node *to_free = mount != nullptr ? static_cast<Node *>(mount) : p_option;
		if (to_free->get_parent() != nullptr) {
			to_free->get_parent()->remove_child(to_free);
		}
		to_free->queue_free();
	}

	_queue_relayout();
}

bool ZDataList::_is_preview_option(ZDataOption *p_option) {
	return p_option != nullptr && p_option->has_meta(PREVIEW_META) && (bool)p_option->get_meta(PREVIEW_META);
}

void ZDataList::_clear_preview_options() {
	_prune_options();
	for (int i = option_ids.size() - 1; i >= 0; i--) {
		ZDataOption *opt = _option_at(i);
		if (!_is_preview_option(opt)) {
			continue;
		}
		option_ids.remove_at(i);
		_unlink_option(opt, i, true);
	}
}

void ZDataList::_refresh_editor_preview() {
	if (!Engine::get_singleton()->is_editor_hint() || !is_inside_tree()) {
		return;
	}
	_clear_preview_options();
	if (default_options.is_empty()) {
		_queue_relayout();
		return;
	}
	for (int i = 0; i < default_options.size(); i++) {
		Object *src = Object::cast_to<Object>(default_options[i]);
		if (src == nullptr) {
			continue;
		}
		ZDataOption *opt = option_add(src);
		if (opt != nullptr) {
			opt->set_meta(PREVIEW_META, true);
			Control *mount = _mount_control_for(opt);
			if (mount != nullptr) {
				mount->set_owner(nullptr);
			}
			opt->set_owner(nullptr);
		}
	}
	_queue_relayout();
}

void ZDataList::_apply_default_options_if_empty() {
	_prune_options();
	if (!option_ids.is_empty() || default_options.is_empty()) {
		return;
	}
	option_add_list(default_options, false);
}

Vector2 ZDataList::_sample_option_size() const {
	for (int i = 0; i < option_ids.size(); i++) {
		Control *mount = const_cast<ZDataList *>(this)->_mount_control_for(_option_at(i));
		if (mount == nullptr || !mount->is_visible()) {
			continue;
		}
		Vector2 min_sz = mount->get_combined_minimum_size();
		if (min_sz.x <= 0.0f) {
			min_sz.x = 32.0f;
		}
		if (min_sz.y <= 0.0f) {
			min_sz.y = 32.0f;
		}
		return min_sz;
	}
	return Vector2(32, 32);
}

int ZDataList::_effective_sections(const Vector2 &p_avail, const Vector2 &p_sample) const {
	if (!scale_sections_to_size) {
		return sections > 0 ? sections : 1;
	}
	if (list_layout == LIST_LAYOUT_VERTICAL) {
		const real_t w = p_sample.x > 0.0f ? p_sample.x : 1.0f;
		return Math::max(1, (int)Math::floor(p_avail.x / w));
	}
	const real_t h = p_sample.y > 0.0f ? p_sample.y : 1.0f;
	return Math::max(1, (int)Math::floor(p_avail.y / h));
}

int ZDataList::_count_visible_mounts() const {
	int n = 0;
	for (int i = 0; i < option_ids.size(); i++) {
		Control *m = const_cast<ZDataList *>(this)->_mount_control_for(_option_at(i));
		if (m != nullptr && m->is_visible()) {
			n++;
		}
	}
	return n;
}

void ZDataList::_queue_relayout() {
	if (layout_queued) {
		return;
	}
	layout_queued = true;
	callable_mp(this, &ZDataList::_relayout).call_deferred();
}

void ZDataList::_relayout() {
	layout_queued = false;
	if (!is_inside_tree()) {
		return;
	}
	_prune_options();

	Vector2 avail = get_size();
	if (avail.x <= 0.0f) {
		avail.x = 1.0f;
	}
	if (avail.y <= 0.0f) {
		avail.y = 1.0f;
	}
	const int sec = _effective_sections(avail, _sample_option_size());
	const int vis = _count_visible_mounts();

	// Always rebuild distribution so order/parents stay correct after add/remove.
	cached_content_kind = -1;
	_ensure_structure(sec, vis);
	_apply_all_mount_size_flags();
}

void ZDataList::set_clear_list_on_ready(bool p_clear) {
	clear_list_on_ready = p_clear;
}

bool ZDataList::get_clear_list_on_ready() const {
	return clear_list_on_ready;
}

void ZDataList::set_list_owner(Object *p_owner) {
	if (p_owner == nullptr) {
		list_owner_id = ObjectID();
	} else {
		list_owner_id = ObjectID(p_owner->get_instance_id());
	}
	Object *owner = get_list_owner();
	_prune_options();
	for (int i = 0; i < option_ids.size(); i++) {
		ZDataOption *opt = _option_at(i);
		if (opt != nullptr) {
			opt->_notify_list_owner_changed(this, owner);
		}
	}
}

Object *ZDataList::get_list_owner() const {
	if (list_owner_id.is_null()) {
		return nullptr;
	}
	return ObjectDB::get_instance(list_owner_id);
}

void ZDataList::set_option_scene(const Ref<PackedScene> &p_scene) {
	if (option_scene == p_scene) {
		return;
	}
	option_scene = p_scene;
	if (Engine::get_singleton()->is_editor_hint() && is_inside_tree()) {
		_refresh_editor_preview();
	}
}

Ref<PackedScene> ZDataList::get_option_scene() const {
	return option_scene;
}

void ZDataList::set_player_index(int p_index) {
	player_index = p_index;
}

int ZDataList::get_player_index() const {
	return player_index;
}

void ZDataList::set_list_layout(EListLayout p_layout) {
	if (list_layout == p_layout) {
		return;
	}
	list_layout = p_layout;
	_queue_relayout();
}

ZDataList::EListLayout ZDataList::get_list_layout() const {
	return list_layout;
}

void ZDataList::set_sections(int p_sections) {
	const int s = p_sections > 0 ? p_sections : 1;
	if (sections == s) {
		return;
	}
	sections = s;
	_queue_relayout();
}

int ZDataList::get_sections() const {
	return sections;
}

void ZDataList::set_scale_sections_to_size(bool p_enabled) {
	if (scale_sections_to_size == p_enabled) {
		return;
	}
	scale_sections_to_size = p_enabled;
	_queue_relayout();
}

bool ZDataList::get_scale_sections_to_size() const {
	return scale_sections_to_size;
}

void ZDataList::set_is_scrollable(bool p_scrollable) {
	if (is_scrollable == p_scrollable) {
		return;
	}
	is_scrollable = p_scrollable;
	_queue_relayout();
}

bool ZDataList::get_is_scrollable() const {
	return is_scrollable;
}

void ZDataList::set_default_options(const Array &p_options) {
	default_options = p_options;
	if (Engine::get_singleton()->is_editor_hint() && is_inside_tree()) {
		_refresh_editor_preview();
	}
}

Array ZDataList::get_default_options() const {
	return default_options;
}

void ZDataList::set_horizontal_alignment(EAlign p_align) {
	if (horizontal_alignment == p_align) {
		return;
	}
	horizontal_alignment = p_align;
	_apply_box_alignment();
	_apply_all_mount_size_flags();
}

ZDataList::EAlign ZDataList::get_horizontal_alignment() const {
	return horizontal_alignment;
}

void ZDataList::set_vertical_alignment(EAlign p_align) {
	if (vertical_alignment == p_align) {
		return;
	}
	vertical_alignment = p_align;
	_apply_box_alignment();
	_apply_all_mount_size_flags();
}

ZDataList::EAlign ZDataList::get_vertical_alignment() const {
	return vertical_alignment;
}

void ZDataList::set_expand_horizontal(bool p_expand) {
	if (expand_horizontal == p_expand) {
		return;
	}
	expand_horizontal = p_expand;
	_apply_all_mount_size_flags();
}

bool ZDataList::get_expand_horizontal() const {
	return expand_horizontal;
}

void ZDataList::set_expand_vertical(bool p_expand) {
	if (expand_vertical == p_expand) {
		return;
	}
	expand_vertical = p_expand;
	_apply_all_mount_size_flags();
}

bool ZDataList::get_expand_vertical() const {
	return expand_vertical;
}

void ZDataList::set_option_is_selectable(const Callable &p_callable) {
	option_is_selectable = p_callable;
	_refresh_all_filters();
}

Callable ZDataList::get_option_is_selectable() const {
	return option_is_selectable;
}

void ZDataList::set_option_is_visible(const Callable &p_callable) {
	option_is_visible = p_callable;
	_refresh_all_filters();
}

Callable ZDataList::get_option_is_visible() const {
	return option_is_visible;
}

ZDataOption *ZDataList::option_add(Object *p_option) {
	if (p_option == nullptr) {
		UtilityFunctions::push_warning("ZDataList.option_add: option is null.");
		return nullptr;
	}

	// Ensure a real container exists before parenting.
	if (content_host == nullptr) {
		_ensure_structure(Math::max(1, sections), option_ids.size());
	}

	ZDataOption *opt = Object::cast_to<ZDataOption>(p_option);
	Object *source = nullptr;
	Node *mount = nullptr;

	if (opt == nullptr) {
		source = p_option;
		if (option_scene.is_valid()) {
			Node *inst = option_scene->instantiate();
			if (inst == nullptr) {
				UtilityFunctions::push_warning("ZDataList.option_add: failed to instantiate option_scene.");
				return nullptr;
			}
			opt = Object::cast_to<ZDataOption>(inst);
			if (opt == nullptr) {
				TypedArray<Node> found = inst->find_children("*", "ZDataOption", true, false);
				if (found.size() > 0) {
					opt = Object::cast_to<ZDataOption>(found[0]);
				}
			}
			if (opt == nullptr) {
				UtilityFunctions::push_warning("ZDataList.option_add: option_scene has no ZDataOption root or child.");
				inst->queue_free();
				return nullptr;
			}
			mount = inst;
		} else {
			opt = memnew(ZDataOption);
			mount = opt;
		}
		opt->set_source_object(source);
	} else {
		source = opt->get_source_object();
		mount = opt;
		Node *scene_root = opt;
		while (scene_root->get_parent() != nullptr && !scene_root->get_parent()->is_inside_tree()) {
			scene_root = scene_root->get_parent();
		}
		if (scene_root->get_parent() == nullptr) {
			mount = scene_root;
		}
	}

	if (_index_of(opt) >= 0) {
		return opt;
	}

	Node *root = _resolve_list_root();
	if (mount->get_parent() != root) {
		if (mount->get_parent() != nullptr) {
			mount->get_parent()->remove_child(mount);
		}
		root->add_child(mount);
	}

	_apply_mount_size_flags(Object::cast_to<Control>(mount));

	option_ids.push_back(ObjectID(opt->get_instance_id()));
	const int idx = option_ids.size() - 1;
	opt->_notify_added_to_list(this, idx, source);
	opt->_notify_list_owner_changed(this, get_list_owner());
	_refresh_option_filters(opt, idx);
	emit_signal("on_option_added", opt, idx);
	_queue_relayout();
	return opt;
}

void ZDataList::option_add_list(const Array &p_options, bool p_clear_first) {
	if (p_clear_first) {
		option_clear_all();
	}
	for (int i = 0; i < p_options.size(); i++) {
		option_add(Object::cast_to<Object>(p_options[i]));
	}
}

void ZDataList::option_clear_all() {
	_prune_options();
	Vector<ObjectID> copy = option_ids;
	option_ids.clear();
	for (int i = 0; i < copy.size(); i++) {
		ZDataOption *opt = Object::cast_to<ZDataOption>(ObjectDB::get_instance(copy[i]));
		_unlink_option(opt, i, true);
	}
	hovered_option_id = ObjectID();
	_queue_relayout();
}

void ZDataList::option_remove(int p_index) {
	_prune_options();
	ZDataOption *opt = _option_at(p_index);
	if (opt == nullptr) {
		return;
	}
	option_ids.remove_at(p_index);
	_unlink_option(opt, p_index, true);
}

void ZDataList::option_select(int p_index) {
	_prune_options();
	ZDataOption *opt = _option_at(p_index);
	if (opt == nullptr || !_eval_selectable(opt, p_index)) {
		return;
	}
	ZPlayer *player = _resolve_player();
	if (player != nullptr) {
		player->set_selected_data_option(opt);
	}
	emit_signal("on_option_selected", opt, p_index);
	opt->emit_signal("on_selected");
}

void ZDataList::select_option_node(ZDataOption *p_option) {
	const int idx = find_option_index(p_option);
	if (idx < 0) {
		return;
	}
	option_select(idx);
}

void ZDataList::option_hover(int p_index) {
	_prune_options();
	ZDataOption *opt = _option_at(p_index);
	if (opt == nullptr) {
		return;
	}
	const ObjectID opt_id(opt->get_instance_id());
	if (!hovered_option_id.is_null() && hovered_option_id != opt_id) {
		ZDataOption *prev = Object::cast_to<ZDataOption>(ObjectDB::get_instance(hovered_option_id));
		if (prev != nullptr) {
			prev->set_hovered(false);
		}
	}
	hovered_option_id = opt_id;
	opt->set_hovered(true);
}

void ZDataList::option_highlight(int p_index, bool p_is_highlighted) {
	_prune_options();
	ZDataOption *opt = _option_at(p_index);
	if (opt == nullptr) {
		return;
	}
	const bool was = opt->get_is_highlighted();
	opt->set_is_highlighted(p_is_highlighted);
	if (was != p_is_highlighted) {
		emit_signal("on_option_highlighted", opt, p_index, p_is_highlighted);
	}
}

int ZDataList::get_option_count() const {
	const_cast<ZDataList *>(this)->_prune_options();
	return option_ids.size();
}

ZDataOption *ZDataList::get_option(int p_index) const {
	const_cast<ZDataList *>(this)->_prune_options();
	return _option_at(p_index);
}

TypedArray<ZDataOption> ZDataList::get_options() const {
	const_cast<ZDataList *>(this)->_prune_options();
	TypedArray<ZDataOption> out;
	for (int i = 0; i < option_ids.size(); i++) {
		ZDataOption *opt = _option_at(i);
		if (opt != nullptr) {
			out.push_back(opt);
		}
	}
	return out;
}

int ZDataList::find_option_index(ZDataOption *p_option) const {
	const_cast<ZDataList *>(this)->_prune_options();
	return _index_of(p_option);
}
