#ifdef DEBUG_ENABLED

#include "flow_editor_plugin.h"

#include "res/flow_basic_node.h"
#include "res/flow_end_node.h"
#include "res/flow_start_node.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_resource_preview.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// ---------------------------------------------------------------------------
// FlowPaletteList
// ---------------------------------------------------------------------------

Variant FlowPaletteList::_get_drag_data(const Vector2 &p_at_position) {
	int idx = get_item_at_position(p_at_position, true);
	if (idx < 0 || !plugin) {
		return Variant();
	}
	Variant meta = get_item_metadata(idx);
	if (meta.get_type() != Variant::DICTIONARY) {
		return Variant();
	}
	Dictionary entry = meta;
	String class_name = entry.get("class", "");
	String script_path = entry.get("script", "");
	if (class_name.is_empty() && script_path.is_empty()) {
		return Variant(); // category header
	}

	Dictionary drag;
	drag["type"] = "flow_node_palette";
	drag["class"] = class_name;
	drag["script"] = script_path;

	// Drag preview
	HBoxContainer *preview = memnew(HBoxContainer);
	Panel *swatch = memnew(Panel);
	swatch->set_custom_minimum_size(Vector2(14, 14));
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	Ref<FlowNode> tmp = plugin->_instantiate_flow_node(class_name, script_path);
	Color col = tmp.is_valid() ? tmp->get_color_bind() : Color(0.5f, 0.5f, 0.5f);
	sb->set_bg_color(col);
	sb->set_corner_radius_all(2);
	swatch->add_theme_stylebox_override("panel", sb);
	preview->add_child(swatch);
	Label *lbl = memnew(Label);
	lbl->set_text(get_item_text(idx));
	preview->add_child(lbl);
	set_drag_preview(preview);

	return drag;
}

// ---------------------------------------------------------------------------
// FlowGraphCanvas
// ---------------------------------------------------------------------------

bool FlowGraphCanvas::_can_drop_data(const Vector2 &p_at_position, const Variant &p_data) const {
	(void)p_at_position;
	if (p_data.get_type() != Variant::DICTIONARY) {
		return false;
	}
	Dictionary d = p_data;
	if (String(d.get("type", "")) != "flow_node_palette") {
		return false;
	}
	return !String(d.get("class", "")).is_empty() || !String(d.get("script", "")).is_empty();
}

void FlowGraphCanvas::_drop_data(const Vector2 &p_at_position, const Variant &p_data) {
	if (!plugin || !_can_drop_data(p_at_position, p_data)) {
		return;
	}
	Dictionary d = p_data;
	String class_name = d.get("class", "");
	String script_path = d.get("script", "");
	Vector2 graph_pos = plugin->_graph_position_from_local(p_at_position);
	plugin->_add_node_of_class(class_name, graph_pos, script_path);
}

// ---------------------------------------------------------------------------
// FlowEditorPlugin
// ---------------------------------------------------------------------------

void FlowEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_new_pressed"), &FlowEditorPlugin::_on_new_pressed);
	ClassDB::bind_method(D_METHOD("_on_open_pressed"), &FlowEditorPlugin::_on_open_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &FlowEditorPlugin::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_as_pressed"), &FlowEditorPlugin::_on_save_as_pressed);
	ClassDB::bind_method(D_METHOD("_on_close_pressed"), &FlowEditorPlugin::_on_close_pressed);
	ClassDB::bind_method(D_METHOD("_on_open_file_selected", "path"), &FlowEditorPlugin::_on_open_file_selected);
	ClassDB::bind_method(D_METHOD("_on_save_file_selected", "path"), &FlowEditorPlugin::_on_save_file_selected);
	ClassDB::bind_method(D_METHOD("_on_open_list_selected", "index"), &FlowEditorPlugin::_on_open_list_selected);
	ClassDB::bind_method(D_METHOD("_on_open_list_activated", "index"), &FlowEditorPlugin::_on_open_list_activated);
	ClassDB::bind_method(D_METHOD("_on_palette_activated", "index"), &FlowEditorPlugin::_on_palette_activated);
	ClassDB::bind_method(D_METHOD("_on_palette_filter_changed", "text"), &FlowEditorPlugin::_on_palette_filter_changed);
	ClassDB::bind_method(D_METHOD("_on_connection_request", "from", "from_port", "to", "to_port"), &FlowEditorPlugin::_on_connection_request);
	ClassDB::bind_method(D_METHOD("_on_disconnection_request", "from", "from_port", "to", "to_port"), &FlowEditorPlugin::_on_disconnection_request);
	ClassDB::bind_method(D_METHOD("_on_connection_to_empty", "from", "from_port", "release_position"), &FlowEditorPlugin::_on_connection_to_empty);
	ClassDB::bind_method(D_METHOD("_on_connection_from_empty", "to", "to_port", "release_position"), &FlowEditorPlugin::_on_connection_from_empty);
	ClassDB::bind_method(D_METHOD("_on_duplicate_nodes_request"), &FlowEditorPlugin::_on_duplicate_nodes_request);
	ClassDB::bind_method(D_METHOD("_on_node_selected", "node"), &FlowEditorPlugin::_on_node_selected);
	ClassDB::bind_method(D_METHOD("_on_delete_nodes_request", "nodes"), &FlowEditorPlugin::_on_delete_nodes_request);
	ClassDB::bind_method(D_METHOD("_on_graph_changed"), &FlowEditorPlugin::_on_graph_changed);
	ClassDB::bind_method(D_METHOD("_on_node_moved", "node_id"), &FlowEditorPlugin::_on_node_moved);
	ClassDB::bind_method(D_METHOD("_on_graph_popup_request", "position"), &FlowEditorPlugin::_on_graph_popup_request);
	ClassDB::bind_method(D_METHOD("_on_add_node_menu_id", "id"), &FlowEditorPlugin::_on_add_node_menu_id);
	ClassDB::bind_method(D_METHOD("_on_add_node_menu_hide"), &FlowEditorPlugin::_on_add_node_menu_hide);
	ClassDB::bind_method(D_METHOD("_clear_pending_connect"), &FlowEditorPlugin::_clear_pending_connect);
	ClassDB::bind_method(D_METHOD("_on_node_inspector_property_edited", "property"), &FlowEditorPlugin::_on_node_inspector_property_edited);
	ClassDB::bind_method(D_METHOD("_on_graph_property_changed", "property", "value", "field", "changing", "node_id"), &FlowEditorPlugin::_on_graph_property_changed);
	ClassDB::bind_method(D_METHOD("_refresh_palette"), &FlowEditorPlugin::_refresh_palette);
}

FlowEditorPlugin::FlowEditorPlugin() {
}

FlowEditorPlugin::~FlowEditorPlugin() {
}

String FlowEditorPlugin::_get_plugin_name() const {
	return "Flow";
}

bool FlowEditorPlugin::_has_main_screen() const {
	return true;
}

void FlowEditorPlugin::_make_visible(bool p_visible) {
	if (main_screen) {
		main_screen->set_visible(p_visible);
	}
}

bool FlowEditorPlugin::_handles(Object *p_object) const {
	return Object::cast_to<FlowGraph>(p_object) != nullptr;
}

void FlowEditorPlugin::_edit(Object *p_object) {
	FlowGraph *graph = Object::cast_to<FlowGraph>(p_object);
	if (!graph) {
		return;
	}
	Ref<FlowGraph> ref = Ref<FlowGraph>(graph);
	String path = graph->get_path();
	_open_graph(ref, path);
	_make_visible(true);
	get_editor_interface()->set_main_screen_editor(_get_plugin_name());
}

void FlowEditorPlugin::_apply_changes() {
	_sync_positions_from_view();
	if (dirty && !current_path.is_empty()) {
		_save_to_path(current_path);
	}
}

void FlowEditorPlugin::_enter_tree() {
	_build_ui();
	// Defer palette build so global classes / @tool scripts finish loading first.
	call_deferred("_refresh_palette");

	// FileSystem thumbnails: use ZResource.icon when set.
	if (z_preview_generator.is_null()) {
		z_preview_generator.instantiate();
	}
	EditorInterface *ei = EditorInterface::get_singleton();
	if (ei) {
		EditorResourcePreview *previewer = ei->get_resource_previewer();
		if (previewer && z_preview_generator.is_valid()) {
			previewer->add_preview_generator(z_preview_generator);
		}
	}
}

void FlowEditorPlugin::_exit_tree() {
	EditorResourcePreview *previewer = EditorInterface::get_singleton()->get_resource_previewer();
	if (previewer && z_preview_generator.is_valid()) {
		previewer->remove_preview_generator(z_preview_generator);
	}
	z_preview_generator.unref();

	if (main_screen) {
		EditorInterface::get_singleton()->get_editor_main_screen()->remove_child(main_screen);
		main_screen->queue_free();
		main_screen = nullptr;
	}
	graph_nodes.clear();
}

void FlowEditorPlugin::_build_ui() {
	main_screen = memnew(Control);
	main_screen->set_name("FlowEditor");
	main_screen->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	main_screen->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_screen->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	EditorInterface::get_singleton()->get_editor_main_screen()->add_child(main_screen);
	main_screen->set_visible(false);

	MarginContainer *margin = memnew(MarginContainer);
	margin->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	margin->add_theme_constant_override("margin_left", 4);
	margin->add_theme_constant_override("margin_top", 4);
	margin->add_theme_constant_override("margin_right", 4);
	margin->add_theme_constant_override("margin_bottom", 4);
	main_screen->add_child(margin);

	root_split = memnew(HSplitContainer);
	root_split->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	root_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	root_split->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	margin->add_child(root_split);

	// ---- Left: open graphs + node palette ----
	left_split = memnew(VSplitContainer);
	left_split->set_custom_minimum_size(Vector2(220, 0));
	left_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	root_split->add_child(left_split);

	VBoxContainer *open_box = memnew(VBoxContainer);
	open_box->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	left_split->add_child(open_box);

	Label *open_label = memnew(Label);
	open_label->set_text("Open Graphs");
	open_box->add_child(open_label);

	open_list = memnew(ItemList);
	open_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	open_list->set_allow_reselect(true);
	open_list->set_auto_translate_mode(Node::AUTO_TRANSLATE_MODE_DISABLED);
	open_list->connect("item_selected", Callable(this, "_on_open_list_selected"));
	open_list->connect("item_activated", Callable(this, "_on_open_list_activated"));
	open_box->add_child(open_list);

	VBoxContainer *palette_box = memnew(VBoxContainer);
	palette_box->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	left_split->add_child(palette_box);

	Label *palette_label = memnew(Label);
	palette_label->set_text("Add Node");
	palette_box->add_child(palette_label);

	palette_filter = memnew(LineEdit);
	palette_filter->set_placeholder("Filter nodes...");
	palette_filter->set_clear_button_enabled(true);
	palette_filter->connect("text_changed", Callable(this, "_on_palette_filter_changed"));
	palette_box->add_child(palette_filter);

	palette_list = memnew(FlowPaletteList);
	palette_list->set_plugin(this);
	palette_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	palette_list->set_fixed_icon_size(Vector2i(16, 16));
	palette_list->set_icon_mode(ItemList::ICON_MODE_LEFT);
	palette_list->set_allow_reselect(true);
	palette_list->set_auto_translate_mode(Node::AUTO_TRANSLATE_MODE_DISABLED);
	palette_list->set_tooltip_text("Double-click or drag onto the graph to add a node.");
	palette_list->connect("item_activated", Callable(this, "_on_palette_activated"));
	palette_box->add_child(palette_list);

	// ---- Center ----
	center_box = memnew(VBoxContainer);
	center_box->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_box->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	root_split->add_child(center_box);

	toolbar = memnew(HBoxContainer);
	center_box->add_child(toolbar);

	Button *new_btn = memnew(Button);
	new_btn->set_text("New");
	new_btn->set_tooltip_text("Create a new FlowGraph");
	new_btn->connect("pressed", Callable(this, "_on_new_pressed"));
	toolbar->add_child(new_btn);

	Button *open_btn = memnew(Button);
	open_btn->set_text("Open...");
	open_btn->connect("pressed", Callable(this, "_on_open_pressed"));
	toolbar->add_child(open_btn);

	Button *save_btn = memnew(Button);
	save_btn->set_text("Save");
	save_btn->connect("pressed", Callable(this, "_on_save_pressed"));
	toolbar->add_child(save_btn);

	Button *save_as_btn = memnew(Button);
	save_as_btn->set_text("Save As...");
	save_as_btn->connect("pressed", Callable(this, "_on_save_as_pressed"));
	toolbar->add_child(save_as_btn);

	Button *close_btn = memnew(Button);
	close_btn->set_text("Close");
	close_btn->set_tooltip_text("Close the current graph tab");
	close_btn->connect("pressed", Callable(this, "_on_close_pressed"));
	toolbar->add_child(close_btn);

	path_label = memnew(Label);
	path_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	path_label->set_text("(no graph open)");
	path_label->set_clip_text(true);
	toolbar->add_child(path_label);

	graph_edit = memnew(FlowGraphCanvas);
	graph_edit->set_plugin(this);
	graph_edit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	graph_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	graph_edit->set_right_disconnects(true);
	graph_edit->set_show_grid(true);
	graph_edit->add_valid_connection_type(0, 0);
	graph_edit->connect("connection_request", Callable(this, "_on_connection_request"));
	graph_edit->connect("disconnection_request", Callable(this, "_on_disconnection_request"));
	graph_edit->connect("connection_to_empty", Callable(this, "_on_connection_to_empty"));
	graph_edit->connect("connection_from_empty", Callable(this, "_on_connection_from_empty"));
	graph_edit->connect("duplicate_nodes_request", Callable(this, "_on_duplicate_nodes_request"));
	graph_edit->connect("node_selected", Callable(this, "_on_node_selected"));
	graph_edit->connect("delete_nodes_request", Callable(this, "_on_delete_nodes_request"));
	graph_edit->connect("end_node_move", Callable(this, "_on_graph_changed"));
	graph_edit->connect("popup_request", Callable(this, "_on_graph_popup_request"));
	center_box->add_child(graph_edit);

	add_node_menu = memnew(PopupMenu);
	add_node_menu->connect("id_pressed", Callable(this, "_on_add_node_menu_id"));
	add_node_menu->connect("popup_hide", Callable(this, "_on_add_node_menu_hide"));
	main_screen->add_child(add_node_menu);

	// ---- Right ----
	right_tabs = memnew(TabContainer);
	right_tabs->set_custom_minimum_size(Vector2(280, 0));
	right_tabs->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	root_split->add_child(right_tabs);

	// Godot 4.x+ exposes EditorInspector as a usable Control for custom docks / main screens.
	// It shows the same property editors as the Inspector dock for whatever Object you pass to edit().
	node_inspector = memnew(EditorInspector);
	node_inspector->set_name("FlowNode");
	node_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	node_inspector->connect("property_edited", Callable(this, "_on_node_inspector_property_edited"));
	right_tabs->add_child(node_inspector);

	graph_inspector = memnew(EditorInspector);
	graph_inspector->set_name("FlowAsset");
	graph_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	graph_inspector->connect("property_edited", Callable(this, "_on_node_inspector_property_edited"));
	right_tabs->add_child(graph_inspector);

	open_dialog = memnew(EditorFileDialog);
	open_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
	open_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
	open_dialog->add_filter("*.tres", "Flow Graph");
	open_dialog->add_filter("*.res", "Flow Graph (binary)");
	open_dialog->add_filter("*.gdflow", "Flow Graph (legacy JSON)");
	open_dialog->connect("file_selected", Callable(this, "_on_open_file_selected"));
	main_screen->add_child(open_dialog);

	save_dialog = memnew(EditorFileDialog);
	save_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
	save_dialog->add_filter("*.tres", "Flow Graph");
	save_dialog->add_filter("*.res", "Flow Graph (binary)");
	save_dialog->connect("file_selected", Callable(this, "_on_save_file_selected"));
	main_screen->add_child(save_dialog);

	root_split->set_split_offset(220);
}

Ref<Texture2D> FlowEditorPlugin::_make_color_icon(const Color &p_color) const {
	Ref<Image> img = Image::create_empty(16, 16, false, Image::FORMAT_RGBA8);
	img->fill(p_color);
	// Subtle border
	Color border = p_color.darkened(0.35f);
	for (int x = 0; x < 16; x++) {
		img->set_pixel(x, 0, border);
		img->set_pixel(x, 15, border);
		img->set_pixel(0, x, border);
		img->set_pixel(15, x, border);
	}
	return ImageTexture::create_from_image(img);
}

String FlowEditorPlugin::_resolve_script_path_for_class(const String &p_class_name) const {
	if (p_class_name.is_empty()) {
		return String();
	}
	TypedArray<Dictionary> globals = ProjectSettings::get_singleton()->get_global_class_list();
	for (int i = 0; i < globals.size(); i++) {
		Dictionary info = globals[i];
		if (String(info.get("class", "")) == p_class_name) {
			return info.get("path", "");
		}
	}
	return String();
}

bool FlowEditorPlugin::_script_extends_flow_node(const Ref<Script> &p_script) const {
	if (p_script.is_null()) {
		return false;
	}
	StringName base = p_script->get_instance_base_type();
	if (base == StringName("FlowNode")) {
		return true;
	}
	if (base != StringName() && ClassDB::class_exists(base) && ClassDB::is_parent_class(base, "FlowNode")) {
		return true;
	}
	Ref<Script> cur = p_script;
	HashSet<String> guard;
	while (cur.is_valid()) {
		String path = cur->get_path();
		if (!path.is_empty()) {
			if (guard.has(path)) {
				break;
			}
			guard.insert(path);
		}
		StringName gn = cur->get_global_name();
		if (gn == StringName("FlowNode")) {
			return true;
		}
		StringName ib = cur->get_instance_base_type();
		if (ib == StringName("FlowNode") || (ib != StringName() && ClassDB::class_exists(ib) && ClassDB::is_parent_class(ib, "FlowNode"))) {
			return true;
		}
		cur = cur->get_base_script();
	}
	return false;
}

Ref<FlowNode> FlowEditorPlugin::_instantiate_flow_node(const String &p_class_name, const String &p_script_path) {
	String script_path = p_script_path;
	if (script_path.is_empty() && !p_class_name.is_empty()) {
		script_path = _resolve_script_path_for_class(p_class_name);
	}

	// Scripted types: construct native FlowNode, then attach script.
	// ClassDB::instantiate(script_class) is unreliable for GDExtension bases from GDScript.
	if (!script_path.is_empty()) {
		if (!ResourceLoader::get_singleton()->exists(script_path)) {
			UtilityFunctions::push_warning("FlowEditor: script not found: ", script_path);
			return Ref<FlowNode>();
		}
		Ref<Resource> res = ResourceLoader::get_singleton()->load(script_path, "Script");
		Ref<Script> script = res;
		if (script.is_null()) {
			UtilityFunctions::push_warning("FlowEditor: failed to load script: ", script_path);
			return Ref<FlowNode>();
		}

		Ref<FlowNode> node;
		node.instantiate();
		node->set_script(script);
		node->rebuild_ports();
		if (node->get_inputs().is_empty() && node->get_outputs().is_empty()) {
			UtilityFunctions::push_warning("FlowEditor: '", script_path, "' has no ports after _define (check add_input_port / add_output_port, and @tool).");
		}
		return node;
	}

	// Native / ClassDB-only types (FlowStartNode, FlowEndNode, FlowBasicNode, C++ subclasses).
	if (p_class_name.is_empty() || p_class_name == "FlowNode") {
		return Ref<FlowNode>();
	}
	if (!ClassDB::can_instantiate(p_class_name)) {
		return Ref<FlowNode>();
	}

	Variant inst = ClassDB::instantiate(p_class_name);
	Object *obj = inst;
	FlowNode *raw = Object::cast_to<FlowNode>(obj);
	if (!raw) {
		return Ref<FlowNode>();
	}
	Ref<FlowNode> node = Ref<FlowNode>(raw);
	node->rebuild_ports();
	return node;
}

void FlowEditorPlugin::_rebuild_type_catalog() {
	type_catalog.clear();
	HashSet<String> seen_keys;

	bool has_start = current_graph.is_valid() && current_graph->get_start_node().is_valid();

	auto add_type = [&](const String &cname, const String &script_path, const String &fallback_label, const String &fallback_category) {
		if (cname == "FlowNode" && script_path.is_empty()) {
			return;
		}
		if (cname == "FlowStartNode" && has_start) {
			return;
		}
		String key = !script_path.is_empty() ? String("s:") + script_path : String("c:") + cname;
		if (key == "s:" || key == "c:" || seen_keys.has(key)) {
			return;
		}

		PaletteType t;
		t.class_name = cname;
		t.script_path = script_path;
		t.label = fallback_label;
		t.category = fallback_category;
		t.color = Color(0.45f, 0.55f, 0.75f, 1.0f);

		// Best-effort preview (listing does not require success).
		// Non-@tool scripts are placeholders in-editor; FlowNode recovers meta from source.
		Ref<FlowNode> tmp = _instantiate_flow_node(cname, script_path);
		if (tmp.is_valid()) {
			String cap = tmp->get_caption_bind();
			if (!cap.is_empty() && !cap.contains("(") && !cap.to_lower().contains("placeholder") && !cap.to_lower().contains("attempt to call")) {
				t.label = cap;
			}
			String cat = tmp->get_category_bind();
			if (!cat.is_empty() && !cat.to_lower().contains("placeholder") && !cat.to_lower().contains("attempt to call")) {
				t.category = cat;
			}
			t.color = tmp->get_color_bind();
			Ref<Texture2D> thumb = tmp->get_thumbnail_bind();
			if (thumb.is_valid()) {
				t.thumbnail = thumb;
			}
		}

		if (cname == "FlowStartNode") {
			t.label = "Start";
			t.category = "Built-in";
		} else if (cname == "FlowEndNode") {
			t.label = "End";
			t.category = "Built-in";
		} else if (cname == "FlowBasicNode") {
			t.label = "Basic";
			t.category = "Built-in";
		}

		if (t.label.is_empty()) {
			t.label = !cname.is_empty() ? cname : script_path.get_file().get_basename();
		}
		if (t.category.is_empty()) {
			t.category = script_path.is_empty() ? String("Built-in") : String("Custom");
		}

		type_catalog.push_back(t);
		seen_keys.insert(key);
	};

	add_type("FlowStartNode", String(), "Start", "Built-in");
	add_type("FlowEndNode", String(), "End", "Built-in");
	add_type("FlowBasicNode", String(), "Basic", "Built-in");

	PackedStringArray inheriters = ClassDB::get_inheriters_from_class("FlowNode");
	for (int i = 0; i < inheriters.size(); i++) {
		String cname = inheriters[i];
		if (cname == "FlowNode" || cname == "FlowStartNode" || cname == "FlowEndNode" || cname == "FlowBasicNode") {
			continue;
		}
		if (!_resolve_script_path_for_class(cname).is_empty()) {
			continue;
		}
		if (ClassDB::can_instantiate(cname)) {
			add_type(cname, String(), cname, "Custom");
		}
	}

	TypedArray<Dictionary> globals = ProjectSettings::get_singleton()->get_global_class_list();
	for (int i = 0; i < globals.size(); i++) {
		Dictionary info = globals[i];
		String cname = info.get("class", "");
		String base = info.get("base", "");
		String path = info.get("path", "");
		bool is_abstract = info.get("is_abstract", false);
		if (path.is_empty() || is_abstract) {
			continue;
		}

		bool is_flow = (base == "FlowNode");
		if (!is_flow && !cname.is_empty() && ClassDB::class_exists(cname) && ClassDB::is_parent_class(cname, "FlowNode")) {
			is_flow = true;
		}
		if (!is_flow && ResourceLoader::get_singleton()->exists(path)) {
			Ref<Resource> res = ResourceLoader::get_singleton()->load(path, "Script");
			Ref<Script> script = res;
			if (script.is_valid() && _script_extends_flow_node(script)) {
				is_flow = true;
			}
		}
		if (!is_flow && FileAccess::file_exists(path)) {
			String src = FileAccess::get_file_as_string(path);
			PackedStringArray lines = src.split("\n");
			for (int li = 0; li < lines.size(); li++) {
				String line = lines[li].strip_edges();
				if (line.begins_with("#") || line.begins_with("//")) {
					continue;
				}
				if (line.begins_with("extends")) {
					if (line.contains("FlowNode")) {
						is_flow = true;
					}
					break;
				}
			}
		}
		if (!is_flow) {
			continue;
		}

		String label = !cname.is_empty() ? cname : path.get_file().get_basename();
		add_type(cname, path, label, "Custom");
	}

	// Folder scan for scripts without class_name.
	if (DirAccess::dir_exists_absolute("res://scripts/flownode")) {
		Ref<DirAccess> dir = DirAccess::open("res://scripts/flownode");
		if (dir.is_valid()) {
			dir->list_dir_begin();
			String fname = dir->get_next();
			while (!fname.is_empty()) {
				if (!dir->current_is_dir() && fname.ends_with(".gd")) {
					String path = String("res://scripts/flownode/").path_join(fname);
					String src = FileAccess::get_file_as_string(path);
					bool is_flow = false;
					String found_class;
					PackedStringArray lines = src.split("\n");
					for (int li = 0; li < lines.size(); li++) {
						String line = lines[li].strip_edges();
						if (line.begins_with("extends") && line.contains("FlowNode")) {
							is_flow = true;
						}
						if (line.begins_with("class_name")) {
							PackedStringArray parts = line.split(" ");
							if (parts.size() >= 2) {
								found_class = parts[1].strip_edges();
							}
						}
					}
					if (is_flow) {
						add_type(found_class, path, !found_class.is_empty() ? found_class : fname.get_basename(), "Custom");
					}
				}
				fname = dir->get_next();
			}
			dir->list_dir_end();
		}
	}

	struct CatalogComparator {
		bool operator()(const PaletteType &a, const PaletteType &b) const {
			if (a.category == b.category) {
				return a.label < b.label;
			}
			if (a.category == "Built-in" && b.category != "Built-in") {
				return true;
			}
			if (b.category == "Built-in" && a.category != "Built-in") {
				return false;
			}
			return a.category < b.category;
		}
	};
	type_catalog.sort_custom<CatalogComparator>();
}

void FlowEditorPlugin::_refresh_palette() {
	if (!palette_list) {
		return;
	}
	_rebuild_type_catalog();

	palette_list->clear();
	palette_entries.clear();

	String filter = palette_filter_text.strip_edges().to_lower();
	String last_category;

	for (int i = 0; i < type_catalog.size(); i++) {
		const PaletteType &e = type_catalog[i];
		if (!filter.is_empty()) {
			if (!e.label.to_lower().contains(filter) && !e.class_name.to_lower().contains(filter) && !e.category.to_lower().contains(filter) && !e.script_path.to_lower().contains(filter)) {
				continue;
			}
		}

		if (e.category != last_category) {
			last_category = e.category;
			int header_idx = palette_list->add_item(String("� ") + e.category + String(" �"));
			palette_list->set_item_selectable(header_idx, false);
			palette_list->set_item_disabled(header_idx, true);
			palette_list->set_item_custom_fg_color(header_idx, Color(0.65f, 0.7f, 0.8f));
			Dictionary empty_meta;
			palette_list->set_item_metadata(header_idx, empty_meta);
			palette_entries.push_back(empty_meta);
		}

		Dictionary meta;
		meta["class"] = e.class_name;
		meta["script"] = e.script_path;

		Ref<Texture2D> icon = e.thumbnail.is_valid() ? e.thumbnail : _make_color_icon(e.color);
		int idx = palette_list->add_item(e.label, icon);
		palette_list->set_item_metadata(idx, meta);
		String tip = e.class_name;
		if (!e.script_path.is_empty()) {
			if (!tip.is_empty()) {
				tip += String("\n");
			}
			tip += e.script_path;
		}
		tip += String("\nCategory: ") + e.category + String("\nDouble-click or drag to add.");
		palette_list->set_item_tooltip(idx, tip);
		palette_entries.push_back(meta);
	}
}

void FlowEditorPlugin::_populate_add_node_menu() {
	if (!add_node_menu) {
		return;
	}
	_rebuild_type_catalog();
	add_node_menu->clear();
	menu_id_to_catalog.clear();

	String last_category;
	int id = 0;
	for (int i = 0; i < type_catalog.size(); i++) {
		const PaletteType &e = type_catalog[i];
		if (e.category != last_category) {
			if (!last_category.is_empty()) {
				add_node_menu->add_separator();
			}
			add_node_menu->add_item(e.category, 100000 + id);
			int section_idx = add_node_menu->get_item_count() - 1;
			add_node_menu->set_item_disabled(section_idx, true);
			last_category = e.category;
		}
		add_node_menu->add_item(e.label, id);
		menu_id_to_catalog.push_back(i);
		id++;
	}
}

void FlowEditorPlugin::_refresh_open_list() {
	if (!open_list) {
		return;
	}
	open_list->clear();
	for (int i = 0; i < open_graphs.size(); i++) {
		String path = open_paths[i];
		String label = path.is_empty() ? String("(unsaved)") : path.get_file();
		bool is_current = current_graph.is_valid() && open_graphs[i] == current_graph;
		if (is_current && dirty) {
			label += " *";
		}
		int idx = open_list->add_item(label);
		open_list->set_item_tooltip(idx, path.is_empty() ? String("(unsaved graph)") : path);
		open_list->set_item_metadata(idx, path);
		if (is_current) {
			open_list->select(i);
		}
	}
}

void FlowEditorPlugin::_clear_graph_view() {
	if (!graph_edit) {
		return;
	}
	graph_edit->clear_connections();
	TypedArray<Node> children = graph_edit->get_children();
	for (int i = 0; i < children.size(); i++) {
		Node *child = Object::cast_to<Node>(children[i]);
		if (Object::cast_to<GraphNode>(child)) {
			graph_edit->remove_child(child);
			child->queue_free();
		}
	}
	graph_nodes.clear();
}

Dictionary FlowEditorPlugin::_find_property_dict(Object *p_object, const String &p_name) const {
	Dictionary empty;
	if (!p_object || p_name.is_empty()) {
		return empty;
	}
	TypedArray<Dictionary> plist = p_object->get_property_list();
	for (int i = 0; i < plist.size(); i++) {
		Dictionary d = plist[i];
		if (String(d.get("name", "")) == p_name) {
			return d;
		}
	}
	return empty;
}

GraphNode *FlowEditorPlugin::_make_graph_node(const Ref<FlowNode> &p_node) {
	GraphNode *gn = memnew(GraphNode);
	String id = p_node->get_node_id();
	gn->set_name(id);
	// Prefer script _get_caption ("Message"), then display_name / type name — never "FlowNode (id)".
	gn->set_title(p_node->get_caption_bind());
	gn->set_position_offset(p_node->get_position());

	Color node_color = p_node->get_color_bind();
	Ref<StyleBoxFlat> panel_sb;
	panel_sb.instantiate();
	panel_sb->set_bg_color(Color(node_color.r * 0.22f, node_color.g * 0.22f, node_color.b * 0.22f, 0.95f));
	panel_sb->set_border_color(node_color);
	panel_sb->set_border_width_all(2);
	panel_sb->set_corner_radius_all(4);
	panel_sb->set_content_margin_all(6);
	gn->add_theme_stylebox_override("panel", panel_sb);
	gn->add_theme_stylebox_override("panel_selected", panel_sb);

	Ref<StyleBoxFlat> title_sb;
	title_sb.instantiate();
	title_sb->set_bg_color(node_color.darkened(0.15f));
	title_sb->set_corner_radius_all(4);
	title_sb->set_content_margin_all(4);
	gn->add_theme_stylebox_override("titlebar", title_sb);
	gn->add_theme_stylebox_override("titlebar_selected", title_sb);

	// Optional type thumbnail on the title bar (only when get_thumbnail returns a valid Texture2D).
	// Applied after the GraphNode is in the tree by callers when needed; here we stash for titlebar
	// once the internal title HBox exists (safe before enter tree in Godot 4.x).
	Ref<Texture2D> thumb = p_node->get_thumbnail_bind();
	if (thumb.is_valid()) {
		HBoxContainer *title_box = gn->get_titlebar_hbox();
		if (title_box) {
			// Avoid duplicating if rebuild is called twice.
			if (!title_box->has_node(NodePath("FlowThumbnail"))) {
				TextureRect *icon = memnew(TextureRect);
				icon->set_name("FlowThumbnail");
				icon->set_texture(thumb);
				icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
				icon->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
				icon->set_custom_minimum_size(Vector2(18, 18));
				icon->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
				title_box->add_child(icon);
				if (title_box->get_child_count() > 1) {
					title_box->move_child(icon, 0);
				}
			}
		}
	}

	// Ensure script _define ports exist (also re-run if attach/load left them empty).
	if (p_node->get_inputs().is_empty() && p_node->get_outputs().is_empty()) {
		p_node->rebuild_ports();
	}

	PackedStringArray inputs = p_node->get_inputs();
	PackedStringArray outputs = p_node->get_outputs();
	PackedStringArray exposed = p_node->get_graph_exposed_params_bind();
	int slot_count = inputs.size() > outputs.size() ? inputs.size() : outputs.size();

	if (slot_count == 0 && exposed.is_empty()) {
		Label *spacer = memnew(Label);
		spacer->set_text(" ");
		gn->add_child(spacer);
	}

	const Color port_color(0.95f, 0.9f, 0.35f, 1.0f);
	for (int i = 0; i < slot_count; i++) {
		HBoxContainer *row = memnew(HBoxContainer);
		Label *left = memnew(Label);
		left->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		left->set_text(i < inputs.size() ? String(inputs[i]) : String());
		row->add_child(left);

		Label *right = memnew(Label);
		right->set_horizontal_alignment(HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT);
		right->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		right->set_text(i < outputs.size() ? String(outputs[i]) : String());
		row->add_child(right);
		gn->add_child(row);

		bool enable_left = i < inputs.size();
		bool enable_right = i < outputs.size();
		// Slot index == child index of this row.
		gn->set_slot(i, enable_left, 0, port_color, enable_right, 0, port_color);
	}

	// Embed real editor property widgets for params listed by _get_graph_exposed_params().
	// Values are the same Object properties shown in the side EditorInspector (stay in sync).
	for (int i = 0; i < exposed.size(); i++) {
		String prop_name = exposed[i];
		if (prop_name.is_empty()) {
			continue;
		}
		Dictionary info = _find_property_dict(p_node.ptr(), prop_name);
		if (info.is_empty()) {
			UtilityFunctions::push_warning("FlowEditor: exposed param '", prop_name, "' not found on ", p_node->get_type_name());
			continue;
		}

		Variant::Type ptype = (Variant::Type)(int)info.get("type", (int)Variant::NIL);
		PropertyHint phint = (PropertyHint)(int)info.get("hint", (int)PROPERTY_HINT_NONE);
		String hint_text = info.get("hint_string", "");
		uint32_t usage = (uint32_t)(int64_t)info.get("usage", (int64_t)PROPERTY_USAGE_DEFAULT);

		EditorProperty *ep = EditorInspector::instantiate_property_editor(
				p_node.ptr(), ptype, prop_name, phint, hint_text, usage, true);
		if (!ep) {
			UtilityFunctions::push_warning("FlowEditor: no EditorProperty for exposed param '", prop_name, "'");
			continue;
		}

		ep->set_object_and_property(p_node.ptr(), StringName(prop_name));
		ep->set_label(prop_name.capitalize());
		ep->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		ep->update_property();
		ep->set_meta("flow_prop", prop_name);
		ep->set_meta("flow_node_id", id);
		ep->connect("property_changed", Callable(this, "_on_graph_property_changed").bind(id));
		gn->add_child(ep);
	}

	gn->set_meta("flow_node_id", id);
	gn->connect("position_offset_changed", Callable(this, "_on_node_moved").bind(id));
	return gn;
}

void FlowEditorPlugin::_refresh_graph_node_ui(const String &p_node_id) {
	if (current_graph.is_null() || !graph_nodes.has(p_node_id)) {
		return;
	}
	GraphNode *gn = graph_nodes[p_node_id];
	Ref<FlowNode> node = current_graph->get_node(p_node_id);
	if (!gn || node.is_null()) {
		return;
	}
	gn->set_title(node->get_caption_bind());

	TypedArray<Node> children = gn->get_children();
	for (int i = 0; i < children.size(); i++) {
		Node *child = Object::cast_to<Node>(children[i]);
		EditorProperty *ep = Object::cast_to<EditorProperty>(child);
		if (ep) {
			ep->update_property();
		}
	}
}

void FlowEditorPlugin::_rebuild_graph_view() {
	_clear_graph_view();
	if (current_graph.is_null()) {
		if (node_inspector) {
			node_inspector->edit(nullptr);
		}
		if (graph_inspector) {
			graph_inspector->edit(nullptr);
		}
		return;
	}

	TypedArray<FlowNode> nodes = current_graph->get_nodes();
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> node = nodes[i];
		if (node.is_null()) {
			continue;
		}
		GraphNode *gn = _make_graph_node(node);
		graph_edit->add_child(gn);
		// Ensure offset applied after enter tree (some GraphEdit versions reset early).
		gn->set_position_offset(node->get_position());
		graph_nodes.insert(node->get_node_id(), gn);
	}

	_apply_connections_to_view();
	graph_inspector->edit(current_graph.ptr());
	node_inspector->edit(nullptr);
	_refresh_palette();
}

void FlowEditorPlugin::_apply_connections_to_view() {
	if (current_graph.is_null() || !graph_edit) {
		return;
	}
	TypedArray<Dictionary> conns = current_graph->get_connections();
	for (int i = 0; i < conns.size(); i++) {
		Dictionary c = conns[i];
		String from_node = c.get("from_node", "");
		String to_node = c.get("to_node", "");
		int32_t from_port = (int32_t)(int64_t)c.get("from_port", 0);
		int32_t to_port = (int32_t)(int64_t)c.get("to_port", 0);
		if (!graph_nodes.has(from_node) || !graph_nodes.has(to_node)) {
			continue;
		}
		graph_edit->connect_node(from_node, from_port, to_node, to_port);
	}
}

void FlowEditorPlugin::_mark_dirty() {
	dirty = true;
	_update_path_label();
	_refresh_open_list();
}

void FlowEditorPlugin::_update_path_label() {
	if (!path_label) {
		return;
	}
	if (current_graph.is_null()) {
		path_label->set_text("(no graph open)");
		return;
	}
	String text = current_path.is_empty() ? String("(unsaved)") : current_path;
	if (dirty) {
		text += " *";
	}
	path_label->set_text(text);
}

int FlowEditorPlugin::_find_open_path(const String &p_path) const {
	if (p_path.is_empty()) {
		return -1;
	}
	for (int i = 0; i < open_paths.size(); i++) {
		if (open_paths[i] == p_path) {
			return i;
		}
	}
	return -1;
}

void FlowEditorPlugin::_open_graph(const Ref<FlowGraph> &p_graph, const String &p_path, bool p_reload_from_disk) {
	if (p_graph.is_null() && p_path.is_empty()) {
		return;
	}

	Ref<FlowGraph> graph = p_graph;
	String path = p_path;

	if (p_reload_from_disk && !path.is_empty()) {
		Ref<Resource> res = ResourceLoader::get_singleton()->load(path, "FlowGraph", ResourceLoader::CACHE_MODE_IGNORE);
		graph = res;
		if (graph.is_null()) {
			UtilityFunctions::push_error("Failed to reload FlowGraph: ", path);
			return;
		}
	}

	if (graph.is_null()) {
		return;
	}

	if (!path.is_empty()) {
		int existing = _find_open_path(path);
		if (existing >= 0) {
			if (p_reload_from_disk) {
				open_graphs[existing] = graph;
			}
			_select_open_index(existing);
			return;
		}
	}

	for (int i = 0; i < open_graphs.size(); i++) {
		if (open_graphs[i] == graph) {
			_select_open_index(i);
			return;
		}
	}

	open_graphs.push_back(graph);
	open_paths.push_back(path);
	_select_open_index(open_graphs.size() - 1);
}

void FlowEditorPlugin::_select_open_index(int p_index) {
	if (p_index < 0 || p_index >= open_graphs.size()) {
		return;
	}
	_sync_positions_from_view();

	current_graph = open_graphs[p_index];
	current_path = open_paths[p_index];
	dirty = false;
	_rebuild_graph_view();
	_update_path_label();
	_refresh_open_list();
}

void FlowEditorPlugin::_sync_positions_from_view() {
	if (current_graph.is_null() || !graph_edit) {
		return;
	}
	for (const KeyValue<String, GraphNode *> &E : graph_nodes) {
		Ref<FlowNode> node = current_graph->get_node(E.key);
		if (node.is_valid() && E.value) {
			node->set_position(E.value->get_position_offset());
		}
	}
}

Vector2 FlowEditorPlugin::_graph_position_from_local(const Vector2 &p_local) const {
	if (!graph_edit) {
		return p_local;
	}
	float zoom = graph_edit->get_zoom();
	if (zoom <= 0.0001f) {
		zoom = 1.0f;
	}
	return (p_local + graph_edit->get_scroll_offset()) / zoom;
}

void FlowEditorPlugin::_clear_pending_connect() {
	pending_connect_active = false;
	pending_connect_from_output = true;
	pending_connect_node_id = String();
	pending_connect_port = 0;
}

void FlowEditorPlugin::_show_add_node_menu_at_local(const Vector2 &p_local_pos) {
	if (!add_node_menu || !graph_edit) {
		return;
	}
	context_add_position = _graph_position_from_local(p_local_pos);
	_populate_add_node_menu();
	if (add_node_menu->get_item_count() == 0) {
		_clear_pending_connect();
		return;
	}
	Vector2 screen = graph_edit->get_screen_position() + p_local_pos;
	add_node_menu->set_position(Vector2i((int32_t)screen.x, (int32_t)screen.y));
	add_node_menu->popup();
}

String FlowEditorPlugin::_add_node_of_class(const String &p_class_name, const Vector2 &p_position, const String &p_script_path) {
	if (current_graph.is_null()) {
		_on_new_pressed();
	}
	if (current_graph.is_null()) {
		return String();
	}
	if (p_class_name.is_empty() && p_script_path.is_empty()) {
		return String();
	}
	if (p_class_name == "FlowStartNode" && current_graph->get_start_node().is_valid()) {
		UtilityFunctions::push_warning("Flow graph already has a Start node.");
		return String();
	}

	Ref<FlowNode> node = _instantiate_flow_node(p_class_name, p_script_path);
	if (node.is_null()) {
		UtilityFunctions::push_error("Cannot instantiate flow node: class=", p_class_name, " script=", p_script_path);
		return String();
	}

	String prefix = !p_class_name.is_empty() ? p_class_name.to_snake_case() : p_script_path.get_file().get_basename().to_snake_case();
	node->set_node_id(current_graph->generate_node_id(prefix));
	node->set_position(p_position);
	current_graph->add_node(node);

	GraphNode *gn = _make_graph_node(node);
	graph_edit->add_child(gn);
	gn->set_position_offset(p_position);
	graph_nodes.insert(node->get_node_id(), gn);
	_mark_dirty();
	_refresh_palette();
	node_inspector->edit(node.ptr());
	right_tabs->set_current_tab(0);
	return node->get_node_id();
}

void FlowEditorPlugin::_duplicate_selected_nodes() {
	if (current_graph.is_null() || !graph_edit) {
		return;
	}
	_sync_positions_from_view();

	// Collect selected node ids (skip Start — only one allowed).
	Vector<String> selected_ids;
	for (const KeyValue<String, GraphNode *> &E : graph_nodes) {
		GraphNode *gn = E.value;
		if (!gn || !gn->is_selected()) {
			continue;
		}
		Ref<FlowNode> src = current_graph->get_node(E.key);
		if (src.is_null()) {
			continue;
		}
		if (src->is_start_node()) {
			UtilityFunctions::push_warning("Flow graph already has a Start node; Start was not duplicated.");
			continue;
		}
		selected_ids.push_back(E.key);
	}
	if (selected_ids.is_empty()) {
		return;
	}

	const Vector2 offset(40, 40);
	HashMap<String, String> id_remap; // old -> new

	// Deselect originals, select the duplicates.
	for (const KeyValue<String, GraphNode *> &E : graph_nodes) {
		if (E.value) {
			E.value->set_selected(false);
		}
	}

	for (int i = 0; i < selected_ids.size(); i++) {
		const String &old_id = selected_ids[i];
		Ref<FlowNode> src = current_graph->get_node(old_id);
		if (src.is_null()) {
			continue;
		}

		Ref<Resource> dup_res = src->duplicate(true);
		Ref<FlowNode> copy = dup_res;
		if (copy.is_null()) {
			UtilityFunctions::push_error("Failed to duplicate flow node: ", old_id);
			continue;
		}

		String prefix = copy->get_type_name().to_snake_case();
		if (prefix.is_empty()) {
			prefix = "node";
		}
		String new_id = current_graph->generate_node_id(prefix);
		copy->set_node_id(new_id);
		copy->set_position(src->get_position() + offset);
		// Ensure ports match define() after duplicate (scripts / define may need rebuild).
		if (copy->get_inputs().is_empty() && copy->get_outputs().is_empty()) {
			copy->rebuild_ports();
		}
		current_graph->add_node(copy);

		GraphNode *gn = _make_graph_node(copy);
		graph_edit->add_child(gn);
		gn->set_position_offset(copy->get_position());
		gn->set_selected(true);
		graph_nodes.insert(new_id, gn);
		id_remap.insert(old_id, new_id);
	}

	// Replicate connections that were entirely within the selection.
	TypedArray<Dictionary> conns = current_graph->get_connections();
	for (int i = 0; i < conns.size(); i++) {
		Dictionary c = conns[i];
		String from_node = c.get("from_node", "");
		String to_node = c.get("to_node", "");
		if (!id_remap.has(from_node) || !id_remap.has(to_node)) {
			continue;
		}
		int32_t from_port = (int32_t)(int64_t)c.get("from_port", 0);
		int32_t to_port = (int32_t)(int64_t)c.get("to_port", 0);
		String new_from = id_remap[from_node];
		String new_to = id_remap[to_node];
		current_graph->add_connection(new_from, from_port, new_to, to_port);
		graph_edit->connect_node(new_from, from_port, new_to, to_port);
	}

	if (!id_remap.is_empty()) {
		// Focus inspector on the last duplicated node.
		String last_new;
		for (const KeyValue<String, String> &E : id_remap) {
			last_new = E.value;
		}
		if (!last_new.is_empty()) {
			Ref<FlowNode> n = current_graph->get_node(last_new);
			if (n.is_valid()) {
				node_inspector->edit(n.ptr());
				right_tabs->set_current_tab(0);
			}
		}
		_mark_dirty();
		_refresh_palette();
	}
}

void FlowEditorPlugin::_on_new_pressed() {
	Ref<FlowGraph> graph;
	graph.instantiate();
	graph->ensure_defaults();
	_open_graph(graph, String());
	dirty = true;
	_update_path_label();
	_refresh_open_list();
}

void FlowEditorPlugin::_on_open_pressed() {
	open_dialog->popup_file_dialog();
}

void FlowEditorPlugin::_on_save_pressed() {
	_sync_positions_from_view();
	if (current_graph.is_null()) {
		return;
	}
	if (current_path.is_empty()) {
		_on_save_as_pressed();
		return;
	}
	_save_to_path(current_path);
}

void FlowEditorPlugin::_on_save_as_pressed() {
	if (current_graph.is_null()) {
		return;
	}
	save_dialog->popup_file_dialog();
}

void FlowEditorPlugin::_on_close_pressed() {
	if (current_graph.is_null()) {
		return;
	}
	_sync_positions_from_view();
	int idx = -1;
	for (int i = 0; i < open_graphs.size(); i++) {
		if (open_graphs[i] == current_graph) {
			idx = i;
			break;
		}
	}
	if (idx < 0) {
		return;
	}
	open_graphs.remove_at(idx);
	// PackedStringArray has no remove_at in older APIs — rebuild.
	PackedStringArray new_paths;
	for (int i = 0; i < open_paths.size(); i++) {
		if (i != idx) {
			new_paths.push_back(open_paths[i]);
		}
	}
	open_paths = new_paths;

	current_graph.unref();
	current_path = "";
	dirty = false;
	_clear_graph_view();
	if (open_graphs.size() > 0) {
		int next = idx;
		if (next >= open_graphs.size()) {
			next = open_graphs.size() - 1;
		}
		_select_open_index(next);
	} else {
		node_inspector->edit(nullptr);
		graph_inspector->edit(nullptr);
		_update_path_label();
		_refresh_open_list();
		_refresh_palette();
	}
}

void FlowEditorPlugin::_on_open_file_selected(const String &p_path) {
	// Always load fresh so positions/ports match disk.
	Ref<Resource> res = ResourceLoader::get_singleton()->load(p_path, "FlowGraph", ResourceLoader::CACHE_MODE_REPLACE);
	Ref<FlowGraph> graph = res;
	if (graph.is_null()) {
		UtilityFunctions::push_error("Failed to load FlowGraph: ", p_path);
		return;
	}
	_open_graph(graph, p_path, false);
	_make_visible(true);
	get_editor_interface()->set_main_screen_editor(_get_plugin_name());
}

void FlowEditorPlugin::_on_save_file_selected(const String &p_path) {
	String path = p_path;
	String ext = path.get_extension().to_lower();
	if (ext != "tres" && ext != "res") {
		path += ".tres";
	}
	_save_to_path(path);
}

Error FlowEditorPlugin::_save_to_path(const String &p_path) {
	if (current_graph.is_null()) {
		return ERR_INVALID_DATA;
	}
	_sync_positions_from_view();

	// Native Godot resource text/binary only. Migrate legacy .gdflow paths to .tres on save.
	String path = p_path;
	String ext = path.get_extension().to_lower();
	if (ext == "gdflow" || ext.is_empty()) {
		path = path.get_basename() + ".tres";
		UtilityFunctions::print("FlowEditor: saving as native resource ", path, " (legacy .gdflow is read-only)");
	}

	// Nested FlowNodes must be path-less so the text saver embeds them as sub-resources
	// (with their script + @export properties). External refs like Texture2D stay ExtResource.
	TypedArray<FlowNode> nodes = current_graph->get_nodes();
	for (int i = 0; i < nodes.size(); i++) {
		Ref<FlowNode> node = nodes[i];
		if (node.is_valid() && !node->get_path().is_empty()) {
			node->set_path(String());
		}
	}

	// Built-in text/binary saver serializes FlowGraph + nested FlowNode properties
	// (including GDScript @export fields on scripted nodes).
	Error err = ResourceSaver::get_singleton()->save(current_graph, path, ResourceSaver::FLAG_CHANGE_PATH | ResourceSaver::FLAG_REPLACE_SUBRESOURCE_PATHS);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to save FlowGraph to ", path, " error=", (int)err);
		return err;
	}
	current_path = path;
	current_graph->set_path(path);
	for (int i = 0; i < open_graphs.size(); i++) {
		if (open_graphs[i] == current_graph) {
			open_paths.set(i, path);
			break;
		}
	}
	dirty = false;
	_update_path_label();
	_refresh_open_list();
	EditorInterface::get_singleton()->get_resource_filesystem()->update_file(path);
	EditorInterface::get_singleton()->get_resource_filesystem()->scan();
	return OK;
}

void FlowEditorPlugin::_on_open_list_selected(int p_index) {
	_select_open_index(p_index);
}

void FlowEditorPlugin::_on_open_list_activated(int p_index) {
	// Double-click an open tab: re-focus (and reload from disk if it has a path).
	if (p_index < 0 || p_index >= open_paths.size()) {
		return;
	}
	String path = open_paths[p_index];
	if (!path.is_empty()) {
		_open_graph(Ref<FlowGraph>(), path, true);
	} else {
		_select_open_index(p_index);
	}
	_make_visible(true);
	get_editor_interface()->set_main_screen_editor(_get_plugin_name());
}

void FlowEditorPlugin::_on_palette_activated(int p_index) {
	if (p_index < 0 || p_index >= palette_entries.size()) {
		return;
	}
	Dictionary entry = palette_entries[p_index];
	String class_name = entry.get("class", "");
	String script_path = entry.get("script", "");
	if (class_name.is_empty() && script_path.is_empty()) {
		return;
	}
	Vector2 pos = _graph_position_from_local(graph_edit->get_size() * 0.35f);
	_add_node_of_class(class_name, pos, script_path);
}

void FlowEditorPlugin::_on_palette_filter_changed(const String &p_text) {
	palette_filter_text = p_text;
	_refresh_palette();
}

void FlowEditorPlugin::_on_connection_request(const StringName &p_from, int32_t p_from_port, const StringName &p_to, int32_t p_to_port) {
	if (current_graph.is_null()) {
		return;
	}
	String from_s = p_from;
	String to_s = p_to;
	if (!current_graph->has_node(from_s) || !current_graph->has_node(to_s)) {
		return;
	}
	current_graph->add_connection(from_s, p_from_port, to_s, p_to_port);
	graph_edit->connect_node(p_from, p_from_port, p_to, p_to_port);
	_mark_dirty();
}

void FlowEditorPlugin::_on_disconnection_request(const StringName &p_from, int32_t p_from_port, const StringName &p_to, int32_t p_to_port) {
	if (current_graph.is_null()) {
		return;
	}
	current_graph->remove_connection(p_from, p_from_port, p_to, p_to_port);
	graph_edit->disconnect_node(p_from, p_from_port, p_to, p_to_port);
	_mark_dirty();
}

void FlowEditorPlugin::_on_connection_to_empty(const StringName &p_from, int32_t p_from_port, const Vector2 &p_release_position) {
	if (current_graph.is_null() || !graph_edit) {
		return;
	}
	String from_s = p_from;
	if (!current_graph->has_node(from_s)) {
		return;
	}
	// Dragging from an output port into empty space → pick a node and auto-connect.
	pending_connect_active = true;
	pending_connect_from_output = true;
	pending_connect_node_id = from_s;
	pending_connect_port = p_from_port;
	_show_add_node_menu_at_local(p_release_position);
}

void FlowEditorPlugin::_on_connection_from_empty(const StringName &p_to, int32_t p_to_port, const Vector2 &p_release_position) {
	if (current_graph.is_null() || !graph_edit) {
		return;
	}
	String to_s = p_to;
	if (!current_graph->has_node(to_s)) {
		return;
	}
	// Dragging from an input port into empty space → pick a node and auto-connect into this port.
	pending_connect_active = true;
	pending_connect_from_output = false;
	pending_connect_node_id = to_s;
	pending_connect_port = p_to_port;
	_show_add_node_menu_at_local(p_release_position);
}

void FlowEditorPlugin::_on_duplicate_nodes_request() {
	_duplicate_selected_nodes();
}

void FlowEditorPlugin::_on_node_selected(Node *p_node) {
	GraphNode *gn = Object::cast_to<GraphNode>(p_node);
	if (!gn || current_graph.is_null()) {
		return;
	}
	String id = gn->get_meta("flow_node_id", gn->get_name());
	Ref<FlowNode> node = current_graph->get_node(id);
	if (node.is_valid()) {
		node_inspector->edit(node.ptr());
		right_tabs->set_current_tab(0);
	}
}

void FlowEditorPlugin::_on_delete_nodes_request(const TypedArray<StringName> &p_nodes) {
	if (current_graph.is_null()) {
		return;
	}
	for (int i = 0; i < p_nodes.size(); i++) {
		String id = p_nodes[i];
		current_graph->remove_node(id);
		if (graph_nodes.has(id)) {
			GraphNode *gn = graph_nodes[id];
			graph_nodes.erase(id);
			if (gn) {
				graph_edit->remove_child(gn);
				gn->queue_free();
			}
		}
	}
	graph_edit->clear_connections();
	_apply_connections_to_view();
	node_inspector->edit(nullptr);
	_mark_dirty();
	_refresh_palette();
}

void FlowEditorPlugin::_on_graph_changed() {
	_sync_positions_from_view();
	_mark_dirty();
}

void FlowEditorPlugin::_on_node_moved(const String &p_node_id) {
	if (current_graph.is_null() || !graph_nodes.has(p_node_id)) {
		return;
	}
	GraphNode *gn = graph_nodes[p_node_id];
	Ref<FlowNode> node = current_graph->get_node(p_node_id);
	if (node.is_valid() && gn) {
		node->set_position(gn->get_position_offset());
		_mark_dirty();
	}
}

void FlowEditorPlugin::_on_graph_popup_request(const Vector2 &p_position) {
	// RMB on empty canvas — plain add (no auto-connect).
	_clear_pending_connect();
	_show_add_node_menu_at_local(p_position);
}

void FlowEditorPlugin::_on_add_node_menu_id(int32_t p_id) {
	if (p_id < 0 || p_id >= menu_id_to_catalog.size()) {
		_clear_pending_connect();
		return; // section header / invalid
	}
	int catalog_idx = menu_id_to_catalog[p_id];
	if (catalog_idx < 0 || catalog_idx >= type_catalog.size()) {
		_clear_pending_connect();
		return;
	}
	const PaletteType &t = type_catalog[catalog_idx];

	// Snapshot pending connect before add (menu hide can race).
	const bool do_connect = pending_connect_active;
	const bool from_output = pending_connect_from_output;
	const String connect_node = pending_connect_node_id;
	const int32_t connect_port = pending_connect_port;
	_clear_pending_connect();

	String new_id = _add_node_of_class(t.class_name, context_add_position, t.script_path);
	if (new_id.is_empty() || !do_connect || current_graph.is_null()) {
		return;
	}
	if (!current_graph->has_node(connect_node) || !current_graph->has_node(new_id)) {
		return;
	}

	Ref<FlowNode> created = current_graph->get_node(new_id);
	if (created.is_null()) {
		return;
	}

	if (from_output) {
		// Output of existing node → input of new node (prefer first input if any).
		if (created->get_inputs().is_empty()) {
			return;
		}
		current_graph->add_connection(connect_node, connect_port, new_id, 0);
		graph_edit->connect_node(connect_node, connect_port, new_id, 0);
		_mark_dirty();
	} else {
		// Output of new node → input of existing node (prefer first output if any).
		if (created->get_outputs().is_empty()) {
			return;
		}
		current_graph->add_connection(new_id, 0, connect_node, connect_port);
		graph_edit->connect_node(new_id, 0, connect_node, connect_port);
		_mark_dirty();
	}
}

void FlowEditorPlugin::_on_add_node_menu_hide() {
	// Deferred so id_pressed (same frame, before hide) can still consume pending state.
	// Cancel path: nothing selected → pending cleared next idle.
	call_deferred("_clear_pending_connect");
}

void FlowEditorPlugin::_on_node_inspector_property_edited(const String &p_property) {
	_mark_dirty();
	// Keep graph-embedded EditorProperty widgets + titles in sync with the side inspector.
	Object *edited = nullptr;
	if (node_inspector) {
		edited = node_inspector->get_edited_object();
	}
	if (!edited && graph_inspector) {
		edited = graph_inspector->get_edited_object();
	}
	FlowNode *flow = Object::cast_to<FlowNode>(edited);
	if (flow) {
		_refresh_graph_node_ui(flow->get_node_id());
		// Caption may depend on display_name / script fields.
		if (graph_nodes.has(flow->get_node_id())) {
			graph_nodes[flow->get_node_id()]->set_title(flow->get_caption_bind());
		}
	}
	(void)p_property;
}

void FlowEditorPlugin::_on_graph_property_changed(const StringName &p_property, const Variant &p_value, const StringName &p_field, bool p_changing, const String &p_node_id) {
	(void)p_field;
	if (current_graph.is_null() || p_node_id.is_empty()) {
		return;
	}
	Ref<FlowNode> node = current_graph->get_node(p_node_id);
	if (node.is_null()) {
		return;
	}

	// EditorProperty usually already applied the value; re-set to be safe and emit_changed.
	if (p_property != StringName()) {
		node->set(p_property, p_value);
	}

	if (!p_changing) {
		_mark_dirty();
		// Refresh side inspector if this node is selected.
		if (node_inspector && node_inspector->get_edited_object() == node.ptr()) {
			node_inspector->edit(node.ptr());
		}
		_refresh_graph_node_ui(p_node_id);
	}
}

#endif // DEBUG_ENABLED
