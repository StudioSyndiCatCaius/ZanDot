#include "flow_node.h"

#include "nodes/flow_player.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {

// Extract first quoted string after `return` on lines following a `func name` definition.
String extract_return_string_after_func(const String &p_src, const String &p_func) {
	PackedStringArray lines = p_src.split("\n");
	bool in_func = false;
	int base_indent = -1;
	for (int i = 0; i < lines.size(); i++) {
		String raw = lines[i];
		String line = raw.strip_edges();
		if (!in_func) {
			if (line.begins_with("func " + p_func) || line.begins_with("func " + p_func + "(")) {
				in_func = true;
				// Indent of next non-empty body line determines function body.
				continue;
			}
			continue;
		}

		if (line.is_empty() || line.begins_with("#")) {
			continue;
		}

		// Count leading whitespace of original line.
		int indent = 0;
		while (indent < raw.length() && (raw[indent] == ' ' || raw[indent] == '\t')) {
			indent++;
		}
		if (base_indent < 0) {
			base_indent = indent;
		} else if (indent <= 0 || (base_indent > 0 && indent < base_indent && !line.begins_with("func "))) {
			// Left the function body.
			if (line.begins_with("func ") || indent == 0) {
				break;
			}
		}

		if (line.begins_with("func ")) {
			break;
		}

		// return "..." or return '...'
		if (line.begins_with("return")) {
			int q1 = line.find("\"");
			int q2 = -1;
			if (q1 >= 0) {
				q2 = line.find("\"", q1 + 1);
			} else {
				q1 = line.find("'");
				if (q1 >= 0) {
					q2 = line.find("'", q1 + 1);
				}
			}
			if (q1 >= 0 && q2 > q1) {
				return line.substr(q1 + 1, q2 - q1 - 1);
			}
		}
	}
	return String();
}

Color extract_return_color_after_func(const String &p_src, const String &p_func) {
	PackedStringArray lines = p_src.split("\n");
	bool in_func = false;
	for (int i = 0; i < lines.size(); i++) {
		String line = lines[i].strip_edges();
		if (!in_func) {
			if (line.begins_with("func " + p_func)) {
				in_func = true;
			}
			continue;
		}
		if (line.begins_with("func ")) {
			break;
		}
		if (!line.begins_with("return")) {
			continue;
		}
		// Color(r, g, b) or Color(r, g, b, a)
		int lp = line.find("Color(");
		if (lp < 0) {
			continue;
		}
		int rp = line.find(")", lp);
		if (rp < 0) {
			continue;
		}
		String inside = line.substr(lp + 6, rp - lp - 6);
		PackedStringArray parts = inside.split(",");
		if (parts.size() >= 3) {
			float r = parts[0].strip_edges().to_float();
			float g = parts[1].strip_edges().to_float();
			float b = parts[2].strip_edges().to_float();
			float a = parts.size() >= 4 ? parts[3].strip_edges().to_float() : 1.0f;
			return Color(r, g, b, a);
		}
	}
	return Color();
}

PackedStringArray extract_string_array_after_func(const String &p_src, const String &p_func) {
	PackedStringArray out;
	PackedStringArray lines = p_src.split("\n");
	bool in_func = false;
	String blob;
	for (int i = 0; i < lines.size(); i++) {
		String line = lines[i].strip_edges();
		if (!in_func) {
			if (line.begins_with("func " + p_func)) {
				in_func = true;
			}
			continue;
		}
		if (line.begins_with("func ")) {
			break;
		}
		blob += line + " ";
		// Stop after a return statement that likely closed.
		if (line.begins_with("return") && (line.contains("]") || line.contains(")"))) {
			break;
		}
	}
	// Collect "..." and '...' literals inside the function blob.
	for (int i = 0; i < blob.length(); i++) {
		char32_t c = blob[i];
		if (c == '"' || c == '\'') {
			char32_t quote = c;
			int start = i + 1;
			int end = start;
			while (end < blob.length() && blob[end] != quote) {
				end++;
			}
			if (end < blob.length()) {
				String s = blob.substr(start, end - start);
				// Skip the function name itself if it appeared oddly; keep short identifiers.
				if (!s.is_empty() && s != p_func) {
					out.push_back(s);
				}
				i = end;
			}
		}
	}
	return out;
}

void extract_ports_from_source(const String &p_src, PackedStringArray &r_inputs, PackedStringArray &r_outputs) {
	// Match add_input_port("x") / add_output_port('y') anywhere (usually in _define).
	auto scan = [&](const String &fn, PackedStringArray &dest) {
		int from = 0;
		while (true) {
			int idx = p_src.find(fn, from);
			if (idx < 0) {
				break;
			}
			int lp = p_src.find("(", idx);
			if (lp < 0) {
				break;
			}
			int rp = p_src.find(")", lp);
			if (rp < 0) {
				break;
			}
			String inside = p_src.substr(lp + 1, rp - lp - 1).strip_edges();
			if (inside.length() >= 2) {
				char32_t q0 = inside[0];
				char32_t q1 = inside[inside.length() - 1];
				if ((q0 == '"' && q1 == '"') || (q0 == '\'' && q1 == '\'')) {
					dest.push_back(inside.substr(1, inside.length() - 2));
				}
			}
			from = rp + 1;
		}
	};
	scan("add_input_port", r_inputs);
	scan("add_output_port", r_outputs);
}

} // namespace

void FlowNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_node_id", "id"), &FlowNode::set_node_id);
	ClassDB::bind_method(D_METHOD("get_node_id"), &FlowNode::get_node_id);

	ClassDB::bind_method(D_METHOD("set_display_name", "name"), &FlowNode::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &FlowNode::get_display_name);

	ClassDB::bind_method(D_METHOD("set_position", "position"), &FlowNode::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &FlowNode::get_position);

	ClassDB::bind_method(D_METHOD("set_inputs", "inputs"), &FlowNode::set_inputs);
	ClassDB::bind_method(D_METHOD("get_inputs"), &FlowNode::get_inputs);

	ClassDB::bind_method(D_METHOD("set_outputs", "outputs"), &FlowNode::set_outputs);
	ClassDB::bind_method(D_METHOD("get_outputs"), &FlowNode::get_outputs);

	ClassDB::bind_method(D_METHOD("set_params", "params"), &FlowNode::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &FlowNode::get_params);

	ClassDB::bind_method(D_METHOD("clear_ports"), &FlowNode::clear_ports);
	ClassDB::bind_method(D_METHOD("add_input_port", "name"), &FlowNode::add_input_port);
	ClassDB::bind_method(D_METHOD("add_output_port", "name"), &FlowNode::add_output_port);
	ClassDB::bind_method(D_METHOD("rebuild_ports"), &FlowNode::rebuild_ports);

	ClassDB::bind_method(D_METHOD("get_caption"), &FlowNode::get_caption_bind);
	ClassDB::bind_method(D_METHOD("get_category"), &FlowNode::get_category_bind);
	ClassDB::bind_method(D_METHOD("get_color"), &FlowNode::get_color_bind);
	ClassDB::bind_method(D_METHOD("get_graph_exposed_params"), &FlowNode::get_graph_exposed_params_bind);
	ClassDB::bind_method(D_METHOD("get_thumbnail"), &FlowNode::get_thumbnail_bind);
	ClassDB::bind_method(D_METHOD("get_type_name"), &FlowNode::get_type_name);
	ClassDB::bind_method(D_METHOD("is_start_node"), &FlowNode::is_start_node);
	ClassDB::bind_method(D_METHOD("is_end_node"), &FlowNode::is_end_node);

	ClassDB::bind_method(D_METHOD("on_enter", "player"), &FlowNode::on_enter);
	ClassDB::bind_method(D_METHOD("on_exit", "player"), &FlowNode::on_exit);
	ClassDB::bind_method(D_METHOD("choose_output", "player"), &FlowNode::choose_output);

	// Inspector: only user-facing fields. Graph topology / layout edited in the graph view.
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "node_id"), "set_node_id", "get_node_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "params"), "set_params", "get_params");

	// Layout / ports: stored in .tres, edited in the graph view (not inspector).
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "inputs", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_inputs", "get_inputs");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "outputs", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_outputs", "get_outputs");

	GDVIRTUAL_BIND(_define);
	GDVIRTUAL_BIND(_get_color);
	GDVIRTUAL_BIND(_get_caption);
	GDVIRTUAL_BIND(_get_category);
	GDVIRTUAL_BIND(_get_graph_exposed_params);
	GDVIRTUAL_BIND(_on_enter, "player");
	GDVIRTUAL_BIND(_on_exit, "player");
	GDVIRTUAL_BIND(_choose_output, "player");
}

FlowNode::FlowNode() {
	node_id = "";
	display_name = "";
	position = Vector2(0, 0);
}

FlowNode::~FlowNode() {
}

bool FlowNode::_is_bad_meta_string(const String &p_s) {
	if (p_s.is_empty()) {
		return true;
	}
	String lower = p_s.to_lower();
	if (lower.contains("placeholder")) {
		return true;
	}
	if (lower.contains("attempt to call")) {
		return true;
	}
	if (lower.contains("tool mode")) {
		return true;
	}
	return false;
}

Ref<Script> FlowNode::_get_attached_script() const {
	Variant script_var = get_script();
	if (script_var.get_type() != Variant::OBJECT) {
		return Ref<Script>();
	}
	return script_var;
}

bool FlowNode::_script_methods_safe() const {
	Ref<Script> script = _get_attached_script();
	if (script.is_null()) {
		return false;
	}
	// Non-@tool scripts become placeholder instances in the editor — calling methods errors
	// with: "Attempt to call a method on a placeholder instance. Check if the script is in tool mode."
	Engine *engine = Engine::get_singleton();
	if (engine && engine->is_editor_hint() && !script->is_tool()) {
		return false;
	}
	return true;
}

String FlowNode::_get_script_source() const {
	Ref<Script> script = _get_attached_script();
	if (script.is_null()) {
		return String();
	}
	if (script->has_source_code()) {
		return script->get_source_code();
	}
	String path = script->get_path();
	if (!path.is_empty() && FileAccess::file_exists(path)) {
		return FileAccess::get_file_as_string(path);
	}
	return String();
}

bool FlowNode::_apply_ports_from_script_source() {
	String src = _get_script_source();
	if (src.is_empty()) {
		return false;
	}
	PackedStringArray in_ports;
	PackedStringArray out_ports;
	extract_ports_from_source(src, in_ports, out_ports);
	if (in_ports.is_empty() && out_ports.is_empty()) {
		return false;
	}
	inputs = in_ports;
	outputs = out_ports;
	return true;
}

String FlowNode::_parse_script_string_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return String();
	}
	return extract_return_string_after_func(src, p_func_name);
}

Color FlowNode::_parse_script_color_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return Color();
	}
	return extract_return_color_after_func(src, p_func_name);
}

PackedStringArray FlowNode::_parse_script_string_array_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return PackedStringArray();
	}
	return extract_string_array_after_func(src, p_func_name);
}

bool FlowNode::_script_has_method(const StringName &p_method) const {
	// IMPORTANT: Do not use Object::has_method() for dispatch — that is true for our own
	// ClassDB-bound wrappers (e.g. get_thumbnail -> get_thumbnail_bind) and causes recursion.
	Ref<Script> script = _get_attached_script();
	if (script.is_null()) {
		return false;
	}
	TypedArray<Dictionary> methods = script->get_script_method_list();
	for (int i = 0; i < methods.size(); i++) {
		Dictionary m = methods[i];
		if (StringName(m.get("name", StringName())) == p_method) {
			return true;
		}
	}
	return false;
}

bool FlowNode::_call_script_void(const StringName &p_method) {
	if (!_script_methods_safe() || !_script_has_method(p_method)) {
		return false;
	}
	call(p_method);
	return true;
}

bool FlowNode::_call_script_ret(const StringName &p_method, Variant &r_ret) const {
	if (!_script_methods_safe() || !_script_has_method(p_method)) {
		return false;
	}
	FlowNode *self = const_cast<FlowNode *>(this);
	r_ret = self->call(p_method);
	return true;
}

void FlowNode::clear_ports() {
	inputs.clear();
	outputs.clear();
}

void FlowNode::add_input_port(const String &p_name) {
	inputs.push_back(p_name);
}

void FlowNode::add_output_port(const String &p_name) {
	outputs.push_back(p_name);
}

void FlowNode::define() {
	// Abstract default: no ports. Concrete subclasses override.
}

void FlowNode::rebuild_ports() {
	clear_ports();
	bool ran = false;

	if (_script_methods_safe()) {
		if (GDVIRTUAL_CALL(_define)) {
			ran = true;
		} else if (_script_has_method(StringName("_define"))) {
			call("_define");
			ran = true;
		}
	}

	// Non-@tool in editor (or failed call): recover ports from source text.
	if ((!ran || (inputs.is_empty() && outputs.is_empty())) && _get_attached_script().is_valid()) {
		if (_apply_ports_from_script_source()) {
			ran = true;
		}
	}

	if (!ran) {
		define();
	}
	emit_changed();
}

void FlowNode::set_node_id(const String &p_id) {
	if (node_id == p_id) {
		return;
	}
	node_id = p_id;
	emit_changed();
}

String FlowNode::get_node_id() const {
	return node_id;
}

void FlowNode::set_display_name(const String &p_name) {
	if (display_name == p_name) {
		return;
	}
	display_name = p_name;
	emit_changed();
}

String FlowNode::get_display_name() const {
	return display_name;
}

void FlowNode::set_position(const Vector2 &p_position) {
	if (position == p_position) {
		return;
	}
	position = p_position;
	emit_changed();
}

Vector2 FlowNode::get_position() const {
	return position;
}

void FlowNode::set_inputs(const PackedStringArray &p_inputs) {
	inputs = p_inputs;
	emit_changed();
}

PackedStringArray FlowNode::get_inputs() const {
	return inputs;
}

void FlowNode::set_outputs(const PackedStringArray &p_outputs) {
	outputs = p_outputs;
	emit_changed();
}

PackedStringArray FlowNode::get_outputs() const {
	return outputs;
}

void FlowNode::set_params(const Dictionary &p_params) {
	params = p_params;
	emit_changed();
}

Dictionary FlowNode::get_params() const {
	return params;
}

String FlowNode::get_type_name() const {
	Ref<Script> script = _get_attached_script();
	if (script.is_valid()) {
		StringName gn = script->get_global_name();
		if (gn != StringName()) {
			return String(gn);
		}
		String path = script->get_path();
		if (!path.is_empty()) {
			return path.get_file().get_basename();
		}
	}
	return String(get_class());
}

String FlowNode::get_caption() const {
	if (!display_name.is_empty()) {
		return display_name;
	}
	return get_type_name();
}

String FlowNode::get_category() const {
	return "Custom";
}

Color FlowNode::get_color() const {
	return Color(0.45f, 0.50f, 0.58f, 1.0f);
}

PackedStringArray FlowNode::get_graph_exposed_params() const {
	return PackedStringArray();
}

Ref<Texture2D> FlowNode::get_thumbnail() const {
	return Ref<Texture2D>();
}

String FlowNode::get_caption_bind() const {
	if (_script_methods_safe()) {
		String result;
		if (GDVIRTUAL_CALL(_get_caption, result) && !_is_bad_meta_string(result)) {
			return result;
		}
		Variant v;
		if (_call_script_ret(StringName("_get_caption"), v) && v.get_type() == Variant::STRING) {
			String s = v;
			if (!_is_bad_meta_string(s)) {
				return s;
			}
		}
	}

	String parsed = _parse_script_string_return("_get_caption");
	if (!_is_bad_meta_string(parsed)) {
		return parsed;
	}
	return get_caption();
}

String FlowNode::get_category_bind() const {
	if (_script_methods_safe()) {
		String result;
		if (GDVIRTUAL_CALL(_get_category, result) && !_is_bad_meta_string(result)) {
			return result;
		}
		Variant v;
		if (_call_script_ret(StringName("_get_category"), v) && v.get_type() == Variant::STRING) {
			String s = v;
			if (!_is_bad_meta_string(s)) {
				return s;
			}
		}
	}

	String parsed = _parse_script_string_return("_get_category");
	if (!_is_bad_meta_string(parsed)) {
		return parsed;
	}
	return get_category();
}

Color FlowNode::get_color_bind() const {
	if (_script_methods_safe()) {
		Color result;
		if (GDVIRTUAL_CALL(_get_color, result)) {
			return result;
		}
		Variant v;
		if (_call_script_ret(StringName("_get_color"), v) && v.get_type() == Variant::COLOR) {
			return v;
		}
	}

	// Source parse: only if the script defines _get_color with a Color(...) return.
	String src = _get_script_source();
	if (!src.is_empty()) {
		int fi = src.find("func _get_color");
		if (fi >= 0) {
			int color_i = src.find("Color(", fi);
			int next_func = src.find("\nfunc ", fi + 1);
			if (color_i >= 0 && (next_func < 0 || color_i < next_func)) {
				return extract_return_color_after_func(src, "_get_color");
			}
		}
	}
	return get_color();
}

PackedStringArray FlowNode::get_graph_exposed_params_bind() const {
	if (_script_methods_safe()) {
		PackedStringArray result;
		if (GDVIRTUAL_CALL(_get_graph_exposed_params, result)) {
			return result;
		}
		Variant v;
		if (_call_script_ret(StringName("_get_graph_exposed_params"), v)) {
			if (v.get_type() == Variant::PACKED_STRING_ARRAY) {
				return v;
			}
			if (v.get_type() == Variant::ARRAY) {
				PackedStringArray out;
				Array arr = v;
				for (int i = 0; i < arr.size(); i++) {
					out.push_back(String(arr[i]));
				}
				return out;
			}
		}
	}

	// Parse string literals from _get_graph_exposed_params body.
	// Also strip known non-param strings if the parser is greedy — prefer PackedStringArray([...]) form.
	PackedStringArray parsed = _parse_script_string_array_return("_get_graph_exposed_params");
	// Filter out method name noise; keep plausible property identifiers.
	PackedStringArray cleaned;
	for (int i = 0; i < parsed.size(); i++) {
		String s = parsed[i];
		if (s.is_empty() || s.begins_with("_") || s.contains(" ")) {
			continue;
		}
		if (s == "PackedStringArray" || s == "Array") {
			continue;
		}
		cleaned.push_back(s);
	}
	if (!cleaned.is_empty()) {
		return cleaned;
	}
	return get_graph_exposed_params();
}

Ref<Texture2D> FlowNode::get_thumbnail_bind() const {
	// C++ override first.
	Ref<Texture2D> cpp_tex = get_thumbnail();
	if (cpp_tex.is_valid()) {
		return cpp_tex;
	}

	if (!_script_methods_safe()) {
		return Ref<Texture2D>();
	}

	// Script: only `_get_thumbnail` (never call `get_thumbnail` — that is this bound method).
	auto as_tex = [](const Variant &v) -> Ref<Texture2D> {
		if (v.get_type() != Variant::OBJECT || !v) {
			return Ref<Texture2D>();
		}
		Object *obj = v;
		if (!obj) {
			return Ref<Texture2D>();
		}
		return Object::cast_to<Texture2D>(obj);
	};

	Variant v;
	if (_call_script_ret(StringName("_get_thumbnail"), v)) {
		Ref<Texture2D> tex = as_tex(v);
		if (tex.is_valid()) {
			return tex;
		}
	}
	return Ref<Texture2D>();
}

void FlowNode::on_enter(FlowPlayer *p_player) {
	Object *player_obj = p_player;
	if (!_script_methods_safe()) {
		return;
	}
	if (GDVIRTUAL_CALL(_on_enter, player_obj)) {
		return;
	}
	if (_script_has_method(StringName("_on_enter"))) {
		call("_on_enter", player_obj);
	}
}

void FlowNode::on_exit(FlowPlayer *p_player) {
	Object *player_obj = p_player;
	if (!_script_methods_safe()) {
		return;
	}
	if (GDVIRTUAL_CALL(_on_exit, player_obj)) {
		return;
	}
	if (_script_has_method(StringName("_on_exit"))) {
		call("_on_exit", player_obj);
	}
}

int64_t FlowNode::choose_output(FlowPlayer *p_player) {
	int64_t result = 0;
	Object *player_obj = p_player;
	if (!_script_methods_safe()) {
		return 0;
	}
	if (GDVIRTUAL_CALL(_choose_output, player_obj, result)) {
		return result;
	}
	if (_script_has_method(StringName("_choose_output"))) {
		Variant v = call("_choose_output", player_obj);
		if (v.get_type() == Variant::INT) {
			return (int64_t)v;
		}
	}
	return 0;
}
