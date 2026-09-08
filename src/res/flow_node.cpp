#include "flow_node.h"

#include "flow_graph.h"
#include "nodes/flow_player.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/vector2i.hpp>

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

void ZFlowNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_node_id", "id"), &ZFlowNode::set_node_id);
	ClassDB::bind_method(D_METHOD("get_node_id"), &ZFlowNode::get_node_id);

	ClassDB::bind_method(D_METHOD("set_display_name", "name"), &ZFlowNode::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &ZFlowNode::get_display_name);

	ClassDB::bind_method(D_METHOD("set_position", "position"), &ZFlowNode::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &ZFlowNode::get_position);

	ClassDB::bind_method(D_METHOD("set_inputs", "inputs"), &ZFlowNode::set_inputs);
	ClassDB::bind_method(D_METHOD("get_inputs"), &ZFlowNode::get_inputs);

	ClassDB::bind_method(D_METHOD("set_outputs", "outputs"), &ZFlowNode::set_outputs);
	ClassDB::bind_method(D_METHOD("get_outputs"), &ZFlowNode::get_outputs);

	ClassDB::bind_method(D_METHOD("set_params", "params"), &ZFlowNode::set_params);
	ClassDB::bind_method(D_METHOD("get_params"), &ZFlowNode::get_params);

	ClassDB::bind_method(D_METHOD("clear_ports"), &ZFlowNode::clear_ports);
	ClassDB::bind_method(D_METHOD("add_input_port", "name"), &ZFlowNode::add_input_port);
	ClassDB::bind_method(D_METHOD("add_output_port", "name"), &ZFlowNode::add_output_port);
	ClassDB::bind_method(D_METHOD("rebuild_ports"), &ZFlowNode::rebuild_ports);

	ClassDB::bind_method(D_METHOD("get_caption"), &ZFlowNode::get_caption_bind);
	ClassDB::bind_method(D_METHOD("get_category"), &ZFlowNode::get_category_bind);
	ClassDB::bind_method(D_METHOD("get_color"), &ZFlowNode::get_color_bind);
	ClassDB::bind_method(D_METHOD("get_graph_exposed_params"), &ZFlowNode::get_graph_exposed_params_bind);
	ClassDB::bind_method(D_METHOD("get_thumbnail"), &ZFlowNode::get_thumbnail_bind);
	ClassDB::bind_method(D_METHOD("get_node_icon"), &ZFlowNode::get_node_icon_bind);
	ClassDB::bind_method(D_METHOD("get_node_size"), &ZFlowNode::get_node_size_bind);
	ClassDB::bind_method(D_METHOD("get_type_name"), &ZFlowNode::get_type_name);
	ClassDB::bind_method(D_METHOD("is_start_node"), &ZFlowNode::is_start_node);
	ClassDB::bind_method(D_METHOD("is_end_node"), &ZFlowNode::is_end_node);
	ClassDB::bind_method(D_METHOD("is_active"), &ZFlowNode::is_active);
	ClassDB::bind_method(D_METHOD("get_graph"), &ZFlowNode::get_graph_bind);
	ClassDB::bind_method(D_METHOD("get_player"), &ZFlowNode::get_runtime_player);

	ClassDB::bind_method(D_METHOD("get_connected_nodes", "input", "output", "input_pin", "output_pin"), &ZFlowNode::get_connected_nodes, DEFVAL(-1), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("trigger_output", "output", "connection", "stop_node"), &ZFlowNode::trigger_output, DEFVAL(-1), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("trigger_output_to_node", "node", "input", "only_if_connected", "stop_node"), &ZFlowNode::trigger_output_to_node, DEFVAL(0), DEFVAL(true), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("stop_node"), &ZFlowNode::stop_node);

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
	GDVIRTUAL_BIND(_get_node_size);
	GDVIRTUAL_BIND(_on_enter, "pin", "player", "previous_node", "previous_out_pin");
	GDVIRTUAL_BIND(_on_exit, "pin", "player");
}

ZFlowNode::ZFlowNode() {
	node_id = "";
	display_name = "";
	position = Vector2(0, 0);
	// Native instances (and GDScript bases before _define) start with 1 in / 1 out.
	rebuild_ports();
}

ZFlowNode::~ZFlowNode() {
}

bool ZFlowNode::_is_bad_meta_string(const String &p_s) {
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

Ref<Script> ZFlowNode::_get_attached_script() const {
	Variant script_var = get_script();
	if (script_var.get_type() != Variant::OBJECT) {
		return Ref<Script>();
	}
	return script_var;
}

bool ZFlowNode::_script_methods_safe() const {
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

String ZFlowNode::_get_script_source() const {
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

bool ZFlowNode::_apply_ports_from_script_source() {
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

String ZFlowNode::_parse_script_string_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return String();
	}
	return extract_return_string_after_func(src, p_func_name);
}

Color ZFlowNode::_parse_script_color_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return Color();
	}
	return extract_return_color_after_func(src, p_func_name);
}

PackedStringArray ZFlowNode::_parse_script_string_array_return(const String &p_func_name) const {
	String src = _get_script_source();
	if (src.is_empty()) {
		return PackedStringArray();
	}
	return extract_string_array_after_func(src, p_func_name);
}

bool ZFlowNode::_script_has_method(const StringName &p_method) const {
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

bool ZFlowNode::_call_script_void(const StringName &p_method) {
	if (!_script_methods_safe() || !_script_has_method(p_method)) {
		return false;
	}
	call(p_method);
	return true;
}

bool ZFlowNode::_call_script_ret(const StringName &p_method, Variant &r_ret) const {
	if (!_script_methods_safe() || !_script_has_method(p_method)) {
		return false;
	}
	ZFlowNode *self = const_cast<ZFlowNode *>(this);
	r_ret = self->call(p_method);
	return true;
}

void ZFlowNode::clear_ports() {
	inputs.clear();
	outputs.clear();
}

void ZFlowNode::add_input_port(const String &p_name) {
	inputs.push_back(p_name);
}

void ZFlowNode::add_output_port(const String &p_name) {
	outputs.push_back(p_name);
}

void ZFlowNode::define() {
	// Default flow topology: one input, one output. Override to customize
	// (Start/End/Hub/ToHub and scripted nodes that need different pins).
	add_input_port("in");
	add_output_port("out");
}

void ZFlowNode::rebuild_ports() {
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

	// If nothing declared pins (e.g. empty _define), still get the default 1 in / 1 out.
	// Asymmetric nodes (Start/End/Hub/ToHub) leave one side empty on purpose — only
	// apply defaults when both sides are empty.
	if (inputs.is_empty() && outputs.is_empty()) {
		add_input_port("in");
		add_output_port("out");
	}
	emit_changed();
}

void ZFlowNode::set_node_id(const String &p_id) {
	if (node_id == p_id) {
		return;
	}
	node_id = p_id;
	emit_changed();
}

String ZFlowNode::get_node_id() const {
	return node_id;
}

void ZFlowNode::set_display_name(const String &p_name) {
	if (display_name == p_name) {
		return;
	}
	display_name = p_name;
	emit_changed();
}

String ZFlowNode::get_display_name() const {
	return display_name;
}

void ZFlowNode::set_position(const Vector2 &p_position) {
	if (position == p_position) {
		return;
	}
	position = p_position;
	emit_changed();
}

Vector2 ZFlowNode::get_position() const {
	return position;
}

void ZFlowNode::set_inputs(const PackedStringArray &p_inputs) {
	inputs = p_inputs;
	emit_changed();
}

PackedStringArray ZFlowNode::get_inputs() const {
	return inputs;
}

void ZFlowNode::set_outputs(const PackedStringArray &p_outputs) {
	outputs = p_outputs;
	emit_changed();
}

PackedStringArray ZFlowNode::get_outputs() const {
	return outputs;
}

void ZFlowNode::set_params(const Dictionary &p_params) {
	params = p_params;
	emit_changed();
}

Dictionary ZFlowNode::get_params() const {
	return params;
}

String ZFlowNode::get_type_name() const {
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

String ZFlowNode::get_caption() const {
	if (!display_name.is_empty()) {
		return display_name;
	}
	return get_type_name();
}

String ZFlowNode::get_category() const {
	return "Custom";
}

Color ZFlowNode::get_color() const {
	return Color(0.45f, 0.50f, 0.58f, 1.0f);
}

PackedStringArray ZFlowNode::get_graph_exposed_params() const {
	return PackedStringArray();
}

Ref<Texture2D> ZFlowNode::get_thumbnail() const {
	return Ref<Texture2D>();
}

Ref<Texture2D> ZFlowNode::get_node_icon() const {
	return Ref<Texture2D>();
}

Vector2 ZFlowNode::get_node_size() const {
	return Vector2();
}

String ZFlowNode::get_caption_bind() const {
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

String ZFlowNode::get_category_bind() const {
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

Color ZFlowNode::get_color_bind() const {
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

PackedStringArray ZFlowNode::get_graph_exposed_params_bind() const {
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

// Script texture getters: call `_get_*` only (never the `get_*` bound wrapper — that is the caller).
Ref<Texture2D> ZFlowNode::_script_texture(const StringName &p_method) const {
	if (!_script_methods_safe()) {
		return Ref<Texture2D>();
	}
	Variant v;
	if (!_call_script_ret(p_method, v) || v.get_type() != Variant::OBJECT || !v) {
		return Ref<Texture2D>();
	}
	Object *obj = v;
	if (!obj) {
		return Ref<Texture2D>();
	}
	return Ref<Texture2D>(Object::cast_to<Texture2D>(obj));
}

Ref<Texture2D> ZFlowNode::get_thumbnail_bind() const {
	// C++ override first.
	Ref<Texture2D> cpp_tex = get_thumbnail();
	if (cpp_tex.is_valid()) {
		return cpp_tex;
	}
	return _script_texture(StringName("_get_thumbnail"));
}

Ref<Texture2D> ZFlowNode::get_node_icon_bind() const {
	Ref<Texture2D> cpp_tex = get_node_icon();
	if (cpp_tex.is_valid()) {
		return cpp_tex;
	}
	return _script_texture(StringName("_get_node_icon"));
}

Vector2 ZFlowNode::get_node_size_bind() const {
	if (_script_methods_safe()) {
		Vector2 result;
		if (GDVIRTUAL_CALL(_get_node_size, result)) {
			return result.maxf(0.0f);
		}
		Variant v;
		if (_call_script_ret(StringName("_get_node_size"), v)) {
			if (v.get_type() == Variant::VECTOR2) {
				return Vector2(v).maxf(0.0f);
			}
			if (v.get_type() == Variant::VECTOR2I) {
				Vector2i vi = v;
				return Vector2((real_t)vi.x, (real_t)vi.y).maxf(0.0f);
			}
		}
	}
	return get_node_size().maxf(0.0f);
}

void ZFlowNode::set_owning_graph(ZFlowGraph *p_graph) {
	owning_graph = p_graph;
}

ZFlowGraph *ZFlowNode::get_owning_graph() const {
	return owning_graph;
}

ZFlowPlayer *ZFlowNode::get_runtime_player() const {
	return runtime_player;
}

Ref<ZFlowGraph> ZFlowNode::get_graph_bind() const {
	if (owning_graph != nullptr) {
		return Ref<ZFlowGraph>(owning_graph);
	}
	if (runtime_player != nullptr) {
		return runtime_player->get_flow_graph();
	}
	return Ref<ZFlowGraph>();
}

void ZFlowNode::_set_runtime_player(ZFlowPlayer *p_player) {
	runtime_player = p_player;
}

void ZFlowNode::_set_active_flag(bool p_active) {
	active = p_active;
}

bool ZFlowNode::is_active() const {
	return active;
}

void ZFlowNode::on_enter(int32_t p_pin, ZFlowPlayer *p_player, ZFlowNode *p_previous_node, int32_t p_previous_out_pin) {
	if (!_script_methods_safe()) {
		return;
	}
	const Ref<ZFlowNode> previous(p_previous_node);
	if (GDVIRTUAL_CALL(_on_enter, (int64_t)p_pin, p_player, previous, (int64_t)p_previous_out_pin)) {
		return;
	}
	if (_script_has_method(StringName("_on_enter"))) {
		call("_on_enter", p_pin, p_player, previous, p_previous_out_pin);
	}
}

void ZFlowNode::on_exit(int32_t p_pin, ZFlowPlayer *p_player) {
	if (!_script_methods_safe()) {
		return;
	}
	if (GDVIRTUAL_CALL(_on_exit, (int64_t)p_pin, p_player)) {
		return;
	}
	if (_script_has_method(StringName("_on_exit"))) {
		call("_on_exit", p_pin, p_player);
	}
}

TypedArray<ZFlowNode> ZFlowNode::get_connected_nodes(bool p_input, bool p_output, int32_t p_input_pin, int32_t p_output_pin) const {
	TypedArray<ZFlowNode> result;
	ZFlowGraph *graph = owning_graph;
	if (!graph && runtime_player) {
		Ref<ZFlowGraph> g = runtime_player->get_flow_graph();
		graph = g.ptr();
	}
	if (!graph || node_id.is_empty()) {
		return result;
	}

	// Deduplicate by instance id.
	HashMap<uint64_t, bool> seen;

	auto push_unique = [&](const Ref<ZFlowNode> &n) {
		if (n.is_null()) {
			return;
		}
		uint64_t id = n->get_instance_id();
		if (seen.has(id)) {
			return;
		}
		seen[id] = true;
		result.push_back(n);
	};

	if (p_output) {
		if (p_output_pin < 0) {
			for (int port = 0; port < outputs.size(); port++) {
				TypedArray<Dictionary> list = graph->get_connections_from(node_id, port);
				for (int i = 0; i < list.size(); i++) {
					Dictionary c = list[i];
					push_unique(graph->get_node(String(c.get("to_node", ""))));
				}
			}
			// Also connections from ports beyond declared outputs (defensive).
			TypedArray<Dictionary> all = graph->get_connections();
			for (int i = 0; i < all.size(); i++) {
				Dictionary c = all[i];
				if (String(c.get("from_node", "")) != node_id) {
					continue;
				}
				push_unique(graph->get_node(String(c.get("to_node", ""))));
			}
		} else {
			TypedArray<Dictionary> list = graph->get_connections_from(node_id, p_output_pin);
			for (int i = 0; i < list.size(); i++) {
				Dictionary c = list[i];
				push_unique(graph->get_node(String(c.get("to_node", ""))));
			}
		}
	}

	if (p_input) {
		TypedArray<Dictionary> list = graph->get_connections_to(node_id, p_input_pin);
		for (int i = 0; i < list.size(); i++) {
			Dictionary c = list[i];
			push_unique(graph->get_node(String(c.get("from_node", ""))));
		}
	}

	return result;
}

bool ZFlowNode::trigger_output(int32_t p_output, int32_t p_connection, bool p_stop_node) {
	if (!active || !runtime_player) {
		return false;
	}
	ZFlowGraph *graph = owning_graph;
	if (!graph) {
		Ref<ZFlowGraph> g = runtime_player->get_flow_graph();
		graph = g.ptr();
	}
	if (!graph) {
		return false;
	}

	TypedArray<Dictionary> list = graph->get_connections_from(node_id, p_output);
	if (list.is_empty()) {
		if (p_stop_node) {
			stop_node();
		}
		return false;
	}

	// Snapshot targets before we may deactivate.
	struct Target {
		Ref<ZFlowNode> node;
		int32_t pin = 0;
	};
	LocalVector<Target> targets;
	if (p_connection < 0) {
		for (int i = 0; i < list.size(); i++) {
			Dictionary c = list[i];
			Target t;
			t.node = graph->get_node(String(c.get("to_node", "")));
			t.pin = int32_t(c.get("to_port", 0));
			if (t.node.is_valid()) {
				targets.push_back(t);
			}
		}
	} else {
		if (p_connection >= list.size()) {
			if (p_stop_node) {
				stop_node();
			}
			return false;
		}
		Dictionary c = list[p_connection];
		Target t;
		t.node = graph->get_node(String(c.get("to_node", "")));
		t.pin = int32_t(c.get("to_port", 0));
		if (t.node.is_valid()) {
			targets.push_back(t);
		}
	}

	Ref<ZFlowNode> self = this;
	ZFlowPlayer *player = runtime_player;
	player->_begin_transition();
	if (p_stop_node) {
		player->deactivate_node(self, p_output);
	}

	bool any = false;
	for (uint32_t i = 0; i < targets.size(); i++) {
		if (!player->is_playing()) {
			break;
		}
		player->activate_node(targets[i].node, targets[i].pin, self, p_output);
		any = true;
	}
	player->_end_transition();
	return any;
}

bool ZFlowNode::trigger_output_to_node(const Ref<ZFlowNode> &p_node, int32_t p_input, bool p_only_if_connected, bool p_stop_node) {
	if (!active || !runtime_player || p_node.is_null()) {
		return false;
	}
	ZFlowGraph *graph = owning_graph;
	if (!graph) {
		Ref<ZFlowGraph> g = runtime_player->get_flow_graph();
		graph = g.ptr();
	}
	if (!graph) {
		return false;
	}

	int32_t used_output = -1;
	if (p_only_if_connected) {
		bool found = false;
		TypedArray<Dictionary> all = graph->get_connections();
		for (int i = 0; i < all.size(); i++) {
			Dictionary c = all[i];
			if (String(c.get("from_node", "")) != node_id) {
				continue;
			}
			if (String(c.get("to_node", "")) != p_node->get_node_id()) {
				continue;
			}
			if (int32_t(c.get("to_port", -1)) != p_input) {
				continue;
			}
			used_output = int32_t(c.get("from_port", 0));
			found = true;
			break;
		}
		if (!found) {
			return false;
		}
	}

	Ref<ZFlowNode> self = this;
	ZFlowPlayer *player = runtime_player;
	player->_begin_transition();
	if (p_stop_node) {
		player->deactivate_node(self, used_output);
	}
	if (player->is_playing()) {
		player->activate_node(p_node, p_input, self, used_output);
	}
	player->_end_transition();
	return true;
}

void ZFlowNode::stop_node() {
	if (!active || !runtime_player) {
		active = false;
		runtime_player = nullptr;
		return;
	}
	Ref<ZFlowNode> self = this;
	runtime_player->deactivate_node(self, -1);
}
