#pragma once

#ifdef DEBUG_ENABLED

#include "ed/z_resource_preview_generator.h"
#include "res/flow_graph.h"
#include "res/flow_node.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/graph_node.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_split_container.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/v_split_container.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class Button;
class FlowEditorPlugin;

// Palette ItemList with drag support (class name as drag payload).
class FlowPaletteList : public ItemList {
	GDCLASS(FlowPaletteList, ItemList)

	FlowEditorPlugin *plugin = nullptr;

protected:
	static void _bind_methods() {}

public:
	void set_plugin(FlowEditorPlugin *p_plugin) { plugin = p_plugin; }
	virtual Variant _get_drag_data(const Vector2 &p_at_position) override;

	FlowPaletteList() = default;
};

// GraphEdit that accepts palette drops.
class FlowGraphCanvas : public GraphEdit {
	GDCLASS(FlowGraphCanvas, GraphEdit)

	FlowEditorPlugin *plugin = nullptr;

protected:
	static void _bind_methods() {}

public:
	void set_plugin(FlowEditorPlugin *p_plugin) { plugin = p_plugin; }
	virtual bool _can_drop_data(const Vector2 &p_at_position, const Variant &p_data) const override;
	virtual void _drop_data(const Vector2 &p_at_position, const Variant &p_data) override;

	FlowGraphCanvas() = default;
};

// Main-screen Flow editor (tab next to Scene / Script).
class FlowEditorPlugin : public EditorPlugin {
	GDCLASS(FlowEditorPlugin, EditorPlugin)

	friend class FlowPaletteList;
	friend class FlowGraphCanvas;

	Control *main_screen = nullptr;
	HSplitContainer *root_split = nullptr;

	VSplitContainer *left_split = nullptr;
	ItemList *open_list = nullptr;
	LineEdit *palette_filter = nullptr;
	FlowPaletteList *palette_list = nullptr;

	VBoxContainer *center_box = nullptr;
	HBoxContainer *toolbar = nullptr;
	Label *path_label = nullptr;
	FlowGraphCanvas *graph_edit = nullptr;

	TabContainer *right_tabs = nullptr;
	EditorInspector *node_inspector = nullptr;
	EditorInspector *graph_inspector = nullptr;

	EditorFileDialog *open_dialog = nullptr;
	EditorFileDialog *save_dialog = nullptr;
	PopupMenu *add_node_menu = nullptr;

	// ZResource FileSystem thumbnails from the resource `icon` property.
	Ref<ZResourcePreviewGenerator> z_preview_generator;

	Ref<FlowGraph> current_graph;
	String current_path;
	bool dirty = false;

	PackedStringArray open_paths;
	TypedArray<FlowGraph> open_graphs;

	HashMap<String, GraphNode *> graph_nodes;

	struct PaletteType {
		String class_name;
		String script_path;
		String label;
		String category;
		Color color = Color(0.5f, 0.55f, 0.65f, 1.0f);
		Ref<Texture2D> thumbnail;
	};

	// Catalog of addable node types (built-ins + script classes). Rebuilt by _rebuild_type_catalog().
	Vector<PaletteType> type_catalog;
	// Parallel to palette ItemList rows: Dictionary { "class", "script" } or empty = header.
	TypedArray<Dictionary> palette_entries;
	// Context-menu id -> catalog index.
	Vector<int> menu_id_to_catalog;
	String palette_filter_text;
	Vector2 context_add_position;

	// Pending port connection when dropping a drag on empty space (node-create popup).
	bool pending_connect_active = false;
	bool pending_connect_from_output = true; // true: output -> new; false: new -> input
	String pending_connect_node_id;
	int32_t pending_connect_port = 0;

	void _build_ui();
	void _rebuild_type_catalog();
	void _refresh_palette();
	void _populate_add_node_menu();
	void _refresh_open_list();
	void _rebuild_graph_view();
	void _clear_graph_view();
	GraphNode *_make_graph_node(const Ref<FlowNode> &p_node);
	void _mark_dirty();
	void _update_path_label();
	void _select_open_index(int p_index);
	int _find_open_path(const String &p_path) const;
	void _open_graph(const Ref<FlowGraph> &p_graph, const String &p_path, bool p_reload_from_disk = false);
	void _sync_positions_from_view();
	void _apply_connections_to_view();
	void _refresh_graph_node_ui(const String &p_node_id);
	Dictionary _find_property_dict(Object *p_object, const String &p_name) const;
	void _clear_pending_connect();
	void _show_add_node_menu_at_local(const Vector2 &p_local_pos);
	// Returns new node id, or empty on failure.
	String _add_node_of_class(const String &p_class_name, const Vector2 &p_position, const String &p_script_path = String());
	void _duplicate_selected_nodes();

	// Instantiate by global class name and/or script path (for GDScript FlowNode subclasses).
	Ref<FlowNode> _instantiate_flow_node(const String &p_class_name, const String &p_script_path = String());
	String _resolve_script_path_for_class(const String &p_class_name) const;
	bool _script_extends_flow_node(const Ref<Script> &p_script) const;
	Ref<Texture2D> _make_color_icon(const Color &p_color) const;
	Vector2 _graph_position_from_local(const Vector2 &p_local) const;

	void _on_new_pressed();
	void _on_open_pressed();
	void _on_save_pressed();
	void _on_save_as_pressed();
	void _on_close_pressed();
	void _on_open_file_selected(const String &p_path);
	void _on_save_file_selected(const String &p_path);

	void _on_open_list_selected(int p_index);
	void _on_open_list_activated(int p_index);
	void _on_palette_activated(int p_index);
	void _on_palette_filter_changed(const String &p_text);

	void _on_connection_request(const StringName &p_from, int32_t p_from_port, const StringName &p_to, int32_t p_to_port);
	void _on_disconnection_request(const StringName &p_from, int32_t p_from_port, const StringName &p_to, int32_t p_to_port);
	void _on_connection_to_empty(const StringName &p_from, int32_t p_from_port, const Vector2 &p_release_position);
	void _on_connection_from_empty(const StringName &p_to, int32_t p_to_port, const Vector2 &p_release_position);
	void _on_duplicate_nodes_request();
	void _on_node_selected(Node *p_node);
	void _on_delete_nodes_request(const TypedArray<StringName> &p_nodes);
	void _on_graph_changed();
	void _on_node_moved(const String &p_node_id);
	void _on_graph_popup_request(const Vector2 &p_position);
	void _on_add_node_menu_id(int32_t p_id);
	void _on_add_node_menu_hide();
	void _on_node_inspector_property_edited(const String &p_property);
	void _on_graph_property_changed(const StringName &p_property, const Variant &p_value, const StringName &p_field, bool p_changing, const String &p_node_id);

	Error _save_to_path(const String &p_path);

protected:
	static void _bind_methods();

public:
	virtual String _get_plugin_name() const override;
	virtual bool _has_main_screen() const override;
	virtual void _make_visible(bool p_visible) override;
	virtual void _edit(Object *p_object) override;
	virtual bool _handles(Object *p_object) const override;
	virtual void _apply_changes() override;

	virtual void _enter_tree() override;
	virtual void _exit_tree() override;

	FlowEditorPlugin();
	~FlowEditorPlugin();
};

} // namespace godot

#endif // DEBUG_ENABLED
