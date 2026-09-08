#include "z_parse.h"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

enum ScalarHint {
	SCALAR_CSV,
	SCALAR_YAML,
	SCALAR_INI,
	SCALAR_TOML,
};

static char32_t _at(const String &p_s, int p_i) {
	if (p_i < 0 || p_i >= p_s.length()) {
		return 0;
	}
	return (char32_t)p_s.unicode_at(p_i);
}

static bool _is_ws(char32_t p_c) {
	return p_c == ' ' || p_c == '\t';
}

static bool _is_nl(char32_t p_c) {
	return p_c == '\n' || p_c == '\r';
}

static String _key_str(const Variant &p_key) {
	if (p_key.get_type() == Variant::STRING) {
		return p_key;
	}
	return String(p_key);
}

static String _format_number(const Variant &p_v) {
	if (p_v.get_type() == Variant::INT) {
		return String::num_int64((int64_t)p_v);
	}
	return String::num((double)p_v);
}

static String _json_cell(const Variant &p_v) {
	return JSON::stringify(p_v, String(), false);
}

static bool _eq_ci(const String &p_a, const char *p_b) {
	return p_a.to_lower() == String(p_b);
}

static Variant _parse_typed_scalar(const String &p_raw, ScalarHint p_hint) {
	const String s = p_raw.strip_edges();
	if (s.is_empty()) {
		return p_hint == SCALAR_CSV ? Variant(String()) : Variant();
	}

	const String low = s.to_lower();
	if (low == "null" || low == "~") {
		return Variant();
	}
	if (low == "true") {
		return true;
	}
	if (low == "false") {
		return false;
	}
	if (p_hint == SCALAR_YAML || p_hint == SCALAR_INI) {
		if (low == "yes" || low == "on") {
			return true;
		}
		if (low == "no" || low == "off") {
			return false;
		}
	}

	if (s.is_valid_int()) {
		return s.to_int();
	}
	if (s.is_valid_float()) {
		return s.to_float();
	}

	if ((s.begins_with("{") && s.ends_with("}")) || (s.begins_with("[") && s.ends_with("]"))) {
		const Variant parsed = JSON::parse_string(s);
		if (parsed.get_type() != Variant::NIL) {
			return parsed;
		}
		if (s == "null") {
			return Variant();
		}
	}

	return s;
}

static int _find_unquoted(const String &p_s, char32_t p_needle) {
	bool in_s = false;
	bool in_d = false;
	for (int i = 0; i < p_s.length(); i++) {
		const char32_t c = _at(p_s, i);
		if (c == '\\' && in_d) {
			i++;
			continue;
		}
		if (c == '\'' && !in_d) {
			in_s = !in_s;
		} else if (c == '"' && !in_s) {
			in_d = !in_d;
		} else if (c == p_needle && !in_s && !in_d) {
			return i;
		}
	}
	return -1;
}

static String _strip_unquoted_comment(const String &p_s) {
	const int hash = _find_unquoted(p_s, '#');
	if (hash < 0) {
		return p_s;
	}
	return p_s.substr(0, hash);
}

static String _rtrim(const String &p_s) {
	int end = p_s.length();
	while (end > 0 && _is_ws(_at(p_s, end - 1))) {
		end--;
	}
	return p_s.substr(0, end);
}

static String _unescape_basic(const String &p_s) {
	String out;
	for (int i = 0; i < p_s.length(); i++) {
		const char32_t c = _at(p_s, i);
		if (c != '\\' || i + 1 >= p_s.length()) {
			out += String::chr(c);
			continue;
		}
		i++;
		const char32_t e = _at(p_s, i);
		switch (e) {
			case 'n':
				out += "\n";
				break;
			case 't':
				out += "\t";
				break;
			case 'r':
				out += "\r";
				break;
			case 'b':
				out += String::chr(8);
				break;
			case 'f':
				out += String::chr(12);
				break;
			case '\\':
				out += "\\";
				break;
			case '"':
				out += "\"";
				break;
			case '\'':
				out += "'";
				break;
			default:
				out += String::chr(e);
				break;
		}
	}
	return out;
}

static String _parse_quoted_at(const String &p_s, int &p_i) {
	if (p_i >= p_s.length()) {
		return String();
	}
	const char32_t q = _at(p_s, p_i);
	if (q != '"' && q != '\'') {
		return String();
	}
	p_i++;
	String raw;
	while (p_i < p_s.length()) {
		const char32_t c = _at(p_s, p_i);
		p_i++;
		if (c == q) {
			break;
		}
		if (c == '\\' && q == '"' && p_i < p_s.length()) {
			raw += String::chr(c);
			raw += String::chr(_at(p_s, p_i));
			p_i++;
			continue;
		}
		raw += String::chr(c);
	}
	return q == '"' ? _unescape_basic(raw) : raw;
}

static String _parse_key_token(const String &p_s) {
	const String t = p_s.strip_edges();
	if (t.is_empty()) {
		return t;
	}
	const char32_t q = _at(t, 0);
	if (q == '"' || q == '\'') {
		int i = 0;
		return _parse_quoted_at(t, i);
	}
	return t;
}

static bool _looks_special_plain(const String &p_s) {
	if (p_s.is_empty()) {
		return true;
	}
	const String low = p_s.to_lower();
	if (low == "true" || low == "false" || low == "null" || low == "~" || low == "yes" || low == "no" || low == "on" || low == "off") {
		return true;
	}
	if (p_s.is_valid_int() || p_s.is_valid_float()) {
		return true;
	}
	const char32_t first = _at(p_s, 0);
	if (first == '-' || first == '[' || first == '{' || first == '*' || first == '&' || first == '!' || first == '|' || first == '>' || first == '%' || first == '@' || first == '`') {
		return true;
	}
	if (_is_ws(first) || _is_ws(_at(p_s, p_s.length() - 1))) {
		return true;
	}
	return p_s.find(":") >= 0 || p_s.find("#") >= 0 || p_s.find("\n") >= 0 || p_s.find("\r") >= 0;
}

static String _yaml_escape(const String &p_s) {
	String out = "\"";
	for (int i = 0; i < p_s.length(); i++) {
		const char32_t c = _at(p_s, i);
		switch (c) {
			case '\\':
				out += "\\\\";
				break;
			case '"':
				out += "\\\"";
				break;
			case '\n':
				out += "\\n";
				break;
			case '\t':
				out += "\\t";
				break;
			case '\r':
				out += "\\r";
				break;
			default:
				out += String::chr(c);
				break;
		}
	}
	out += "\"";
	return out;
}

static String _spaces(int p_n) {
	String s;
	for (int i = 0; i < p_n; i++) {
		s += " ";
	}
	return s;
}

// ==========================================================================================
// CSV
// ==========================================================================================

static String _csv_escape(const String &p_field) {
	const bool need = p_field.find(",") >= 0 || p_field.find("\"") >= 0 || p_field.find("\n") >= 0 || p_field.find("\r") >= 0;
	if (!need) {
		return p_field;
	}
	return String("\"") + p_field.replace("\"", "\"\"") + "\"";
}

static String _csv_cell(const Variant &p_v) {
	switch (p_v.get_type()) {
		case Variant::NIL:
			return String();
		case Variant::BOOL:
			return (bool)p_v ? "true" : "false";
		case Variant::INT:
		case Variant::FLOAT:
			return _format_number(p_v);
		case Variant::STRING:
			return p_v;
		case Variant::ARRAY:
		case Variant::DICTIONARY:
			return _json_cell(p_v);
		default:
			return String(p_v);
	}
}

static Variant _csv_parse_cell(const String &p_field) {
	return _parse_typed_scalar(p_field, SCALAR_CSV);
}

static void _csv_parse_records(const String &p_str, Vector<PackedStringArray> &r_rows) {
	PackedStringArray row;
	String field;
	bool in_quotes = false;
	const int n = p_str.length();
	for (int i = 0; i < n; i++) {
		const char32_t c = _at(p_str, i);
		if (in_quotes) {
			if (c == '"') {
				if (i + 1 < n && _at(p_str, i + 1) == '"') {
					field += "\"";
					i++;
				} else {
					in_quotes = false;
				}
			} else {
				field += String::chr(c);
			}
			continue;
		}
		if (c == '"') {
			in_quotes = true;
			continue;
		}
		if (c == ',') {
			row.push_back(field);
			field = String();
			continue;
		}
		if (c == '\r') {
			continue;
		}
		if (c == '\n') {
			row.push_back(field);
			field = String();
			r_rows.push_back(row);
			row = PackedStringArray();
			continue;
		}
		field += String::chr(c);
	}
	if (in_quotes || !field.is_empty() || row.size() > 0) {
		row.push_back(field);
		r_rows.push_back(row);
	}
}

static void _collect_csv_headers(const TypedArray<Dictionary> &p_rows, PackedStringArray &r_headers) {
	for (int i = 0; i < p_rows.size(); i++) {
		const Dictionary row = p_rows[i];
		const Array keys = row.keys();
		for (int k = 0; k < keys.size(); k++) {
			const String name = _key_str(keys[k]);
			bool found = false;
			for (int h = 0; h < r_headers.size(); h++) {
				if (r_headers[h] == name) {
					found = true;
					break;
				}
			}
			if (!found) {
				r_headers.push_back(name);
			}
		}
	}
}

// ==========================================================================================
// YAML flow + block
// ==========================================================================================

static void _skip_ws(const String &p_s, int &p_i) {
	while (p_i < p_s.length() && _is_ws(_at(p_s, p_i))) {
		p_i++;
	}
}

static Variant _yaml_parse_flow(const String &p_s, int &p_i);

static Variant _yaml_parse_flow(const String &p_s, int &p_i) {
	_skip_ws(p_s, p_i);
	if (p_i >= p_s.length()) {
		return Variant();
	}
	const char32_t c = _at(p_s, p_i);
	if (c == '"' || c == '\'') {
		return _parse_quoted_at(p_s, p_i);
	}
	if (c == '[') {
		p_i++;
		Array a;
		_skip_ws(p_s, p_i);
		if (p_i < p_s.length() && _at(p_s, p_i) == ']') {
			p_i++;
			return a;
		}
		while (p_i < p_s.length()) {
			a.push_back(_yaml_parse_flow(p_s, p_i));
			_skip_ws(p_s, p_i);
			if (p_i < p_s.length() && _at(p_s, p_i) == ',') {
				p_i++;
				continue;
			}
			if (p_i < p_s.length() && _at(p_s, p_i) == ']') {
				p_i++;
			}
			break;
		}
		return a;
	}
	if (c == '{') {
		p_i++;
		Dictionary d;
		_skip_ws(p_s, p_i);
		if (p_i < p_s.length() && _at(p_s, p_i) == '}') {
			p_i++;
			return d;
		}
		while (p_i < p_s.length()) {
			_skip_ws(p_s, p_i);
			String key;
			if (p_i < p_s.length() && (_at(p_s, p_i) == '"' || _at(p_s, p_i) == '\'')) {
				key = _parse_quoted_at(p_s, p_i);
			} else {
				const int start = p_i;
				while (p_i < p_s.length()) {
					const char32_t ch = _at(p_s, p_i);
					if (ch == ':' || ch == ',' || ch == '}') {
						break;
					}
					p_i++;
				}
				key = p_s.substr(start, p_i - start).strip_edges();
			}
			_skip_ws(p_s, p_i);
			if (p_i < p_s.length() && _at(p_s, p_i) == ':') {
				p_i++;
			}
			d[key] = _yaml_parse_flow(p_s, p_i);
			_skip_ws(p_s, p_i);
			if (p_i < p_s.length() && _at(p_s, p_i) == ',') {
				p_i++;
				continue;
			}
			if (p_i < p_s.length() && _at(p_s, p_i) == '}') {
				p_i++;
			}
			break;
		}
		return d;
	}

	const int start = p_i;
	while (p_i < p_s.length()) {
		const char32_t ch = _at(p_s, p_i);
		if (ch == ',' || ch == ']' || ch == '}' || ch == '#' || _is_nl(ch)) {
			break;
		}
		p_i++;
	}
	return _parse_typed_scalar(p_s.substr(start, p_i - start), SCALAR_YAML);
}

static Variant _yaml_parse_value_token(const String &p_s) {
	int i = 0;
	return _yaml_parse_flow(p_s, i);
}

struct YLine {
	int indent = 0;
	String content;
};

static bool _yaml_is_seq(const String &p_content) {
	return p_content == "-" || p_content.begins_with("- ");
}

static Vector<YLine> _yaml_prep(const String &p_src) {
	const String norm = p_src.replace("\r\n", "\n").replace("\r", "\n");
	const PackedStringArray raw = norm.split("\n");
	Vector<YLine> lines;
	for (int li = 0; li < raw.size(); li++) {
		const String line = raw[li];
		const String trimmed = line.strip_edges();
		if (trimmed.is_empty() || trimmed.begins_with("#") || trimmed == "---" || trimmed == "...") {
			continue;
		}
		int indent = 0;
		int i = 0;
		while (i < line.length()) {
			const char32_t c = _at(line, i);
			if (c == ' ') {
				indent++;
				i++;
			} else if (c == '\t') {
				indent += 2;
				i++;
			} else {
				break;
			}
		}
		String content = _rtrim(_strip_unquoted_comment(line.substr(i)));
		if (content.is_empty()) {
			continue;
		}
		YLine yl;
		yl.indent = indent;
		yl.content = content;
		lines.push_back(yl);
	}
	return lines;
}

static Variant _yaml_parse_node(const Vector<YLine> &p_lines, int &p_idx, int p_min_indent);
static Dictionary _yaml_parse_map(const Vector<YLine> &p_lines, int &p_idx, int p_indent);

static String _yaml_parse_block_scalar(const Vector<YLine> &p_lines, int &p_idx, int p_parent_indent, const String &p_style) {
	if (p_idx >= p_lines.size() || p_lines[p_idx].indent <= p_parent_indent) {
		return String();
	}
	const int base = p_lines[p_idx].indent;
	PackedStringArray parts;
	while (p_idx < p_lines.size() && p_lines[p_idx].indent >= base) {
		const int extra = p_lines[p_idx].indent - base;
		parts.push_back(_spaces(extra) + p_lines[p_idx].content);
		p_idx++;
	}
	if (p_style.begins_with(">")) {
		String folded;
		for (int i = 0; i < parts.size(); i++) {
			if (i > 0) {
				folded += parts[i].is_empty() ? "\n" : " ";
			}
			folded += parts[i];
		}
		return folded;
	}
	String joined;
	for (int i = 0; i < parts.size(); i++) {
		if (i > 0) {
			joined += "\n";
		}
		joined += parts[i];
	}
	return joined;
}

static void _dict_merge(Dictionary &p_into, const Dictionary &p_from) {
	const Array keys = p_from.keys();
	for (int i = 0; i < keys.size(); i++) {
		p_into[keys[i]] = p_from[keys[i]];
	}
}

static Dictionary _yaml_parse_map(const Vector<YLine> &p_lines, int &p_idx, int p_indent) {
	Dictionary d;
	while (p_idx < p_lines.size() && p_lines[p_idx].indent == p_indent && !_yaml_is_seq(p_lines[p_idx].content)) {
		const String content = p_lines[p_idx].content;
		const int colon = _find_unquoted(content, ':');
		if (colon < 0) {
			p_idx++;
			continue;
		}
		const String key = _parse_key_token(content.substr(0, colon));
		const String rest = content.substr(colon + 1).strip_edges();
		p_idx++;
		if (rest.is_empty()) {
			if (p_idx < p_lines.size() && p_lines[p_idx].indent > p_indent) {
				d[key] = _yaml_parse_node(p_lines, p_idx, p_indent + 1);
			} else {
				d[key] = Variant();
			}
		} else if (rest == "|" || rest == "|-" || rest == "|+" || rest == ">" || rest == ">-" || rest == ">+") {
			d[key] = _yaml_parse_block_scalar(p_lines, p_idx, p_indent, rest);
		} else {
			d[key] = _yaml_parse_value_token(rest);
		}
	}
	return d;
}

static Array _yaml_parse_seq(const Vector<YLine> &p_lines, int &p_idx, int p_indent) {
	Array a;
	while (p_idx < p_lines.size() && p_lines[p_idx].indent == p_indent && _yaml_is_seq(p_lines[p_idx].content)) {
		const String content = p_lines[p_idx].content;
		const String rest = content == "-" ? String() : content.substr(2).strip_edges();
		p_idx++;
		if (rest.is_empty()) {
			if (p_idx < p_lines.size() && p_lines[p_idx].indent > p_indent) {
				a.push_back(_yaml_parse_node(p_lines, p_idx, p_indent + 1));
			} else {
				a.push_back(Variant());
			}
			continue;
		}
		if (rest == "|" || rest == "|-" || rest == "|+" || rest == ">" || rest == ">-" || rest == ">+") {
			a.push_back(_yaml_parse_block_scalar(p_lines, p_idx, p_indent, rest));
			continue;
		}
		const int colon = _find_unquoted(rest, ':');
		const bool flow = rest.begins_with("[") || rest.begins_with("{") || rest.begins_with("\"") || rest.begins_with("'");
		if (colon >= 0 && !flow) {
			Dictionary item;
			const String key = _parse_key_token(rest.substr(0, colon));
			const String val = rest.substr(colon + 1).strip_edges();
			if (val.is_empty()) {
				if (p_idx < p_lines.size() && p_lines[p_idx].indent > p_indent) {
					item[key] = _yaml_parse_node(p_lines, p_idx, p_indent + 1);
				} else {
					item[key] = Variant();
				}
			} else {
				item[key] = _yaml_parse_value_token(val);
			}
			if (p_idx < p_lines.size() && p_lines[p_idx].indent > p_indent && !_yaml_is_seq(p_lines[p_idx].content)) {
				const Dictionary more = _yaml_parse_map(p_lines, p_idx, p_lines[p_idx].indent);
				_dict_merge(item, more);
			}
			a.push_back(item);
		} else {
			a.push_back(_yaml_parse_value_token(rest));
		}
	}
	return a;
}

static Variant _yaml_parse_node(const Vector<YLine> &p_lines, int &p_idx, int p_min_indent) {
	if (p_idx >= p_lines.size() || p_lines[p_idx].indent < p_min_indent) {
		return Dictionary();
	}
	const int indent = p_lines[p_idx].indent;
	if (_yaml_is_seq(p_lines[p_idx].content)) {
		return _yaml_parse_seq(p_lines, p_idx, indent);
	}
	return _yaml_parse_map(p_lines, p_idx, indent);
}

static bool _yaml_quote_key(const String &p_key) {
	if (p_key.is_empty()) {
		return true;
	}
	for (int i = 0; i < p_key.length(); i++) {
		const char32_t c = _at(p_key, i);
		const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
		if (!ok) {
			return true;
		}
	}
	return false;
}

static String _yaml_fmt_scalar(const Variant &p_v) {
	switch (p_v.get_type()) {
		case Variant::NIL:
			return "null";
		case Variant::BOOL:
			return (bool)p_v ? "true" : "false";
		case Variant::INT:
		case Variant::FLOAT:
			return _format_number(p_v);
		case Variant::STRING: {
			const String s = p_v;
			if (s.find("\n") >= 0) {
				return String(); // caller uses block
			}
			return _looks_special_plain(s) ? _yaml_escape(s) : s;
		}
		default:
			return _yaml_escape(String(p_v));
	}
}

static void _yaml_write(const Variant &p_v, int p_indent, bool p_seq_item, String &r_out);

static void _yaml_write_key(const String &p_key, String &r_out) {
	r_out += _yaml_quote_key(p_key) ? _yaml_escape(p_key) : p_key;
}

static void _yaml_write_dict(const Dictionary &p_d, int p_indent, bool p_seq_item, String &r_out) {
	const Array keys = p_d.keys();
	if (keys.is_empty()) {
		if (p_seq_item) {
			r_out += _spaces(p_indent) + "- {}\n";
		} else if (p_indent == 0) {
			// empty document
		} else {
			r_out += "{}\n";
		}
		return;
	}
	for (int i = 0; i < keys.size(); i++) {
		const String key = _key_str(keys[i]);
		const Variant val = p_d[keys[i]];
		if (p_seq_item && i == 0) {
			r_out += _spaces(p_indent) + "- ";
			_yaml_write_key(key, r_out);
			r_out += ":";
		} else {
			r_out += _spaces(p_seq_item ? p_indent + 2 : p_indent);
			_yaml_write_key(key, r_out);
			r_out += ":";
		}

		const int child_indent = p_seq_item ? p_indent + 4 : p_indent + 2;
		if (val.get_type() == Variant::DICTIONARY) {
			const Dictionary child = val;
			if (child.is_empty()) {
				r_out += " {}\n";
			} else {
				r_out += "\n";
				_yaml_write(val, child_indent, false, r_out);
			}
		} else if (val.get_type() == Variant::ARRAY) {
			const Array child = val;
			if (child.is_empty()) {
				r_out += " []\n";
			} else {
				r_out += "\n";
				_yaml_write(val, child_indent, false, r_out);
			}
		} else if (val.get_type() == Variant::STRING && String(val).find("\n") >= 0) {
			r_out += " |\n";
			const PackedStringArray parts = String(val).replace("\r\n", "\n").split("\n");
			for (int p = 0; p < parts.size(); p++) {
				r_out += _spaces(child_indent) + parts[p] + "\n";
			}
		} else {
			r_out += " " + _yaml_fmt_scalar(val) + "\n";
		}
	}
}

static void _yaml_write(const Variant &p_v, int p_indent, bool p_seq_item, String &r_out) {
	if (p_v.get_type() == Variant::DICTIONARY) {
		_yaml_write_dict(p_v, p_indent, p_seq_item, r_out);
		return;
	}
	if (p_v.get_type() == Variant::ARRAY) {
		const Array a = p_v;
		if (a.is_empty()) {
			r_out += _spaces(p_indent) + (p_seq_item ? "- " : "") + "[]\n";
			return;
		}
		for (int i = 0; i < a.size(); i++) {
			const Variant item = a[i];
			if (item.get_type() == Variant::DICTIONARY) {
				_yaml_write_dict(item, p_indent, true, r_out);
			} else if (item.get_type() == Variant::ARRAY) {
				r_out += _spaces(p_indent) + "-\n";
				_yaml_write(item, p_indent + 2, false, r_out);
			} else {
				r_out += _spaces(p_indent) + "- " + _yaml_fmt_scalar(item) + "\n";
			}
		}
		return;
	}
	r_out += _spaces(p_indent) + (p_seq_item ? "- " : "") + _yaml_fmt_scalar(p_v) + "\n";
}

// ==========================================================================================
// INI
// ==========================================================================================

static String _ini_format_value(const Variant &p_v) {
	switch (p_v.get_type()) {
		case Variant::NIL:
			return String();
		case Variant::BOOL:
			return (bool)p_v ? "true" : "false";
		case Variant::INT:
		case Variant::FLOAT:
			return _format_number(p_v);
		case Variant::STRING: {
			const String s = p_v;
			if (s.find("\n") >= 0 || s.find(";") >= 0 || s.find("#") >= 0 || (s.find("=") >= 0) || _is_ws(_at(s, 0)) || _is_ws(_at(s, s.length() - 1))) {
				return _yaml_escape(s);
			}
			return s;
		}
		case Variant::ARRAY:
		case Variant::DICTIONARY:
			return _json_cell(p_v);
		default:
			return String(p_v);
	}
}

static Dictionary _ini_ensure_path(Dictionary p_root, const PackedStringArray &p_path) {
	Dictionary cur = p_root;
	for (int i = 0; i < p_path.size(); i++) {
		const String k = p_path[i];
		if (k.is_empty()) {
			continue;
		}
		if (!cur.has(k) || cur[k].get_type() != Variant::DICTIONARY) {
			Dictionary nxt;
			cur[k] = nxt;
			cur = nxt;
		} else {
			cur = cur[k];
		}
	}
	return cur;
}

static PackedStringArray _split_dotted(const String &p_path) {
	PackedStringArray out;
	if (p_path.is_empty()) {
		return out;
	}
	const PackedStringArray parts = p_path.split(".");
	for (int i = 0; i < parts.size(); i++) {
		const String p = parts[i].strip_edges();
		if (!p.is_empty()) {
			out.push_back(p);
		}
	}
	return out;
}

static void _ini_write_section(const String &p_path, const Dictionary &p_d, String &r_out) {
	String body;
	Dictionary nested;
	const Array keys = p_d.keys();
	for (int i = 0; i < keys.size(); i++) {
		const String key = _key_str(keys[i]);
		const Variant val = p_d[keys[i]];
		if (val.get_type() == Variant::DICTIONARY) {
			nested[key] = val;
			continue;
		}
		body += key + String("=") + _ini_format_value(val) + "\n";
	}
	if (!body.is_empty()) {
		if (!r_out.is_empty() && !r_out.ends_with("\n\n")) {
			if (!r_out.ends_with("\n")) {
				r_out += "\n";
			}
			if (!p_path.is_empty()) {
				r_out += "\n";
			}
		}
		if (!p_path.is_empty()) {
			r_out += "[" + p_path + "]\n";
		}
		r_out += body;
	}
	const Array nkeys = nested.keys();
	for (int i = 0; i < nkeys.size(); i++) {
		const String key = _key_str(nkeys[i]);
		const String child_path = p_path.is_empty() ? key : p_path + String(".") + key;
		_ini_write_section(child_path, nested[nkeys[i]], r_out);
	}
}

static String _ini_strip_inline_comment(const String &p_s) {
	int cut = _find_unquoted(p_s, '#');
	const int semi = _find_unquoted(p_s, ';');
	if (semi >= 0 && (cut < 0 || semi < cut)) {
		cut = semi;
	}
	if (cut < 0) {
		return p_s.strip_edges();
	}
	return p_s.substr(0, cut).strip_edges();
}

// ==========================================================================================
// TOML
// ==========================================================================================

struct Scan {
	String s;
	int i = 0;
	int n = 0;

	explicit Scan(const String &p_s) :
			s(p_s), n(p_s.length()) {}

	bool eof() const { return i >= n; }

	char32_t peek(int p_off = 0) const {
		const int j = i + p_off;
		if (j < 0 || j >= n) {
			return 0;
		}
		return (char32_t)s.unicode_at(j);
	}

	char32_t take() {
		return eof() ? 0 : (char32_t)s.unicode_at(i++);
	}

	void skip_ws() {
		while (!eof() && _is_ws(peek())) {
			take();
		}
	}

	void skip_ws_nl_comments() {
		while (!eof()) {
			const char32_t c = peek();
			if (_is_ws(c) || _is_nl(c)) {
				take();
			} else if (c == '#') {
				while (!eof() && !_is_nl(peek())) {
					take();
				}
			} else {
				break;
			}
		}
	}
};

static bool _toml_bare_ok(const String &p_s) {
	if (p_s.is_empty()) {
		return false;
	}
	for (int i = 0; i < p_s.length(); i++) {
		const char32_t c = _at(p_s, i);
		const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
		if (!ok) {
			return false;
		}
	}
	return true;
}

static String _toml_quote_key(const String &p_key) {
	return _toml_bare_ok(p_key) ? p_key : _yaml_escape(p_key);
}

static String _toml_fmt_string(const String &p_s) {
	String out = "\"";
	for (int i = 0; i < p_s.length(); i++) {
		const char32_t c = _at(p_s, i);
		switch (c) {
			case '\\':
				out += "\\\\";
				break;
			case '"':
				out += "\\\"";
				break;
			case '\n':
				out += "\\n";
				break;
			case '\t':
				out += "\\t";
				break;
			case '\r':
				out += "\\r";
				break;
			default:
				out += String::chr(c);
				break;
		}
	}
	out += "\"";
	return out;
}

static bool _is_array_of_dicts(const Array &p_a) {
	if (p_a.is_empty()) {
		return false;
	}
	for (int i = 0; i < p_a.size(); i++) {
		if (p_a[i].get_type() != Variant::DICTIONARY) {
			return false;
		}
	}
	return true;
}

static String _toml_fmt_value(const Variant &p_v);

static String _toml_fmt_array(const Array &p_a) {
	String out = "[";
	for (int i = 0; i < p_a.size(); i++) {
		if (i > 0) {
			out += ", ";
		}
		out += _toml_fmt_value(p_a[i]);
	}
	out += "]";
	return out;
}

static String _toml_fmt_inline_table(const Dictionary &p_d) {
	String out = "{ ";
	const Array keys = p_d.keys();
	for (int i = 0; i < keys.size(); i++) {
		if (i > 0) {
			out += ", ";
		}
		out += _toml_quote_key(_key_str(keys[i])) + " = " + _toml_fmt_value(p_d[keys[i]]);
	}
	out += " }";
	return out;
}

static String _toml_fmt_value(const Variant &p_v) {
	switch (p_v.get_type()) {
		case Variant::NIL:
			return "\"\"";
		case Variant::BOOL:
			return (bool)p_v ? "true" : "false";
		case Variant::INT:
		case Variant::FLOAT:
			return _format_number(p_v);
		case Variant::STRING:
			return _toml_fmt_string(p_v);
		case Variant::ARRAY:
			return _toml_fmt_array(p_v);
		case Variant::DICTIONARY:
			return _toml_fmt_inline_table(p_v);
		default:
			return _toml_fmt_string(String(p_v));
	}
}

static void _toml_write_table(const String &p_path, const Dictionary &p_d, bool p_array_item, String &r_out) {
	String kvs;
	Dictionary nested;
	Dictionary arrays;
	const Array keys = p_d.keys();
	for (int i = 0; i < keys.size(); i++) {
		const String key = _key_str(keys[i]);
		const Variant val = p_d[keys[i]];
		if (val.get_type() == Variant::DICTIONARY) {
			nested[key] = val;
			continue;
		}
		if (val.get_type() == Variant::ARRAY && _is_array_of_dicts(val)) {
			arrays[key] = val;
			continue;
		}
		kvs += _toml_quote_key(key) + " = " + _toml_fmt_value(val) + "\n";
	}

	if (!kvs.is_empty() || p_array_item) {
		if (!r_out.is_empty() && !r_out.ends_with("\n\n")) {
			if (!r_out.ends_with("\n")) {
				r_out += "\n";
			}
			r_out += "\n";
		}
		if (!p_path.is_empty()) {
			if (p_array_item) {
				r_out += "[[" + p_path + "]]\n";
			} else {
				r_out += "[" + p_path + "]\n";
			}
		}
		r_out += kvs;
	}

	const Array nkeys = nested.keys();
	for (int i = 0; i < nkeys.size(); i++) {
		const String key = _key_str(nkeys[i]);
		const String child = p_path.is_empty() ? _toml_quote_key(key) : p_path + String(".") + _toml_quote_key(key);
		_toml_write_table(child, nested[nkeys[i]], false, r_out);
	}
	const Array akeys = arrays.keys();
	for (int i = 0; i < akeys.size(); i++) {
		const String key = _key_str(akeys[i]);
		const String child = p_path.is_empty() ? _toml_quote_key(key) : p_path + String(".") + _toml_quote_key(key);
		const Array arr = arrays[akeys[i]];
		for (int r = 0; r < arr.size(); r++) {
			_toml_write_table(child, arr[r], true, r_out);
		}
	}
}

static String _toml_parse_literal_or_basic(Scan &p_sc, bool p_literal, bool p_multiline) {
	String out;
	if (p_multiline) {
		// Skip the first newline after the opening quotes.
		if (p_sc.peek() == '\r') {
			p_sc.take();
		}
		if (p_sc.peek() == '\n') {
			p_sc.take();
		}
	}
	while (!p_sc.eof()) {
		const char32_t c = p_sc.peek();
		if (p_literal) {
			if (c == '\'') {
				if (p_multiline && p_sc.peek(1) == '\'' && p_sc.peek(2) == '\'') {
					p_sc.take();
					p_sc.take();
					p_sc.take();
					break;
				}
				if (!p_multiline) {
					p_sc.take();
					break;
				}
			}
			out += String::chr(p_sc.take());
			continue;
		}
		if (c == '"') {
			if (p_multiline && p_sc.peek(1) == '"' && p_sc.peek(2) == '"') {
				p_sc.take();
				p_sc.take();
				p_sc.take();
				break;
			}
			if (!p_multiline) {
				p_sc.take();
				break;
			}
		}
		if (c == '\\') {
			p_sc.take();
			if (p_sc.eof()) {
				break;
			}
			const char32_t e = p_sc.take();
			if (p_multiline && (e == '\n' || e == '\r')) {
				if (e == '\r' && p_sc.peek() == '\n') {
					p_sc.take();
				}
				while (_is_ws(p_sc.peek())) {
					p_sc.take();
				}
				continue;
			}
			switch (e) {
				case 'n':
					out += "\n";
					break;
				case 't':
					out += "\t";
					break;
				case 'r':
					out += "\r";
					break;
				case 'b':
					out += String::chr(8);
					break;
				case 'f':
					out += String::chr(12);
					break;
				case '\\':
					out += "\\";
					break;
				case '"':
					out += "\"";
					break;
				default:
					out += String::chr(e);
					break;
			}
			continue;
		}
		out += String::chr(p_sc.take());
	}
	return out;
}

static Variant _toml_parse_value(Scan &p_sc);

static PackedStringArray _toml_parse_key(Scan &p_sc) {
	PackedStringArray parts;
	while (!p_sc.eof()) {
		p_sc.skip_ws();
		String part;
		const char32_t c = p_sc.peek();
		if (c == '"' || c == '\'') {
			const bool lit = c == '\'';
			p_sc.take();
			part = _toml_parse_literal_or_basic(p_sc, lit, false);
		} else {
			while (!p_sc.eof()) {
				const char32_t ch = p_sc.peek();
				const bool ok = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-';
				if (!ok) {
					break;
				}
				part += String::chr(p_sc.take());
			}
		}
		if (!part.is_empty()) {
			parts.push_back(part);
		}
		p_sc.skip_ws();
		if (p_sc.peek() == '.') {
			p_sc.take();
			continue;
		}
		break;
	}
	return parts;
}

static Variant _toml_parse_array(Scan &p_sc) {
	Array a;
	p_sc.take(); // [
	while (!p_sc.eof()) {
		p_sc.skip_ws_nl_comments();
		if (p_sc.peek() == ']') {
			p_sc.take();
			break;
		}
		a.push_back(_toml_parse_value(p_sc));
		p_sc.skip_ws_nl_comments();
		if (p_sc.peek() == ',') {
			p_sc.take();
			continue;
		}
		if (p_sc.peek() == ']') {
			p_sc.take();
		}
		break;
	}
	return a;
}

static Variant _toml_parse_inline_table(Scan &p_sc) {
	Dictionary d;
	p_sc.take(); // {
	while (!p_sc.eof()) {
		p_sc.skip_ws();
		if (p_sc.peek() == '}') {
			p_sc.take();
			break;
		}
		const PackedStringArray path = _toml_parse_key(p_sc);
		p_sc.skip_ws();
		if (p_sc.peek() == '=') {
			p_sc.take();
		}
		p_sc.skip_ws();
		const Variant val = _toml_parse_value(p_sc);
		if (path.size() == 1) {
			d[path[0]] = val;
		} else if (path.size() > 1) {
			PackedStringArray parent = path;
			parent.remove_at(parent.size() - 1);
			Dictionary nest = _ini_ensure_path(d, parent);
			nest[path[path.size() - 1]] = val;
		}
		p_sc.skip_ws();
		if (p_sc.peek() == ',') {
			p_sc.take();
			continue;
		}
		if (p_sc.peek() == '}') {
			p_sc.take();
		}
		break;
	}
	return d;
}

static Variant _toml_parse_value(Scan &p_sc) {
	p_sc.skip_ws();
	const char32_t c = p_sc.peek();
	if (c == '"') {
		const bool ml = p_sc.peek(1) == '"' && p_sc.peek(2) == '"';
		if (ml) {
			p_sc.take();
			p_sc.take();
			p_sc.take();
			return _toml_parse_literal_or_basic(p_sc, false, true);
		}
		p_sc.take();
		return _toml_parse_literal_or_basic(p_sc, false, false);
	}
	if (c == '\'') {
		const bool ml = p_sc.peek(1) == '\'' && p_sc.peek(2) == '\'';
		if (ml) {
			p_sc.take();
			p_sc.take();
			p_sc.take();
			return _toml_parse_literal_or_basic(p_sc, true, true);
		}
		p_sc.take();
		return _toml_parse_literal_or_basic(p_sc, true, false);
	}
	if (c == '[') {
		return _toml_parse_array(p_sc);
	}
	if (c == '{') {
		return _toml_parse_inline_table(p_sc);
	}

	String raw;
	while (!p_sc.eof()) {
		const char32_t ch = p_sc.peek();
		if (_is_ws(ch) || _is_nl(ch) || ch == '#' || ch == ',' || ch == ']' || ch == '}' || ch == '=') {
			break;
		}
		raw += String::chr(p_sc.take());
	}
	raw = raw.replace("_", "");
	if (_eq_ci(raw, "true")) {
		return true;
	}
	if (_eq_ci(raw, "false")) {
		return false;
	}
	if (raw.is_valid_int()) {
		return raw.to_int();
	}
	if (raw.is_valid_float()) {
		return raw.to_float();
	}
	return raw;
}

static Dictionary _toml_ensure_array_table(Dictionary p_root, const PackedStringArray &p_path) {
	if (p_path.is_empty()) {
		return p_root;
	}
	PackedStringArray parent_path = p_path;
	parent_path.remove_at(parent_path.size() - 1);
	Dictionary parent = _ini_ensure_path(p_root, parent_path);
	const String last = p_path[p_path.size() - 1];
	Array arr;
	if (parent.has(last) && parent[last].get_type() == Variant::ARRAY) {
		arr = parent[last];
	} else {
		arr = Array();
		parent[last] = arr;
	}
	Dictionary item;
	arr.push_back(item);
	return item;
}

static Dictionary _toml_current_from_path(Dictionary p_root, const PackedStringArray &p_path) {
	if (p_path.is_empty()) {
		return p_root;
	}
	return _ini_ensure_path(p_root, p_path);
}

static void _toml_assign_dotted(Dictionary p_table, const PackedStringArray &p_path, const Variant &p_val) {
	if (p_path.is_empty()) {
		return;
	}
	if (p_path.size() == 1) {
		p_table[p_path[0]] = p_val;
		return;
	}
	PackedStringArray parent = p_path;
	parent.remove_at(parent.size() - 1);
	Dictionary nest = _ini_ensure_path(p_table, parent);
	nest[p_path[p_path.size() - 1]] = p_val;
}

} // namespace

void Z_Parse::_bind_methods() {
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_CSV", "data"), &Z_Parse::to_csv);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_CSV", "str"), &Z_Parse::from_csv);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_YAML", "data"), &Z_Parse::to_yaml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_YAML", "str"), &Z_Parse::from_yaml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_INI", "data"), &Z_Parse::to_ini);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_INI", "str"), &Z_Parse::from_ini);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_TOML", "data"), &Z_Parse::to_toml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_TOML", "str"), &Z_Parse::from_toml);

	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_csv", "data"), &Z_Parse::to_csv);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_csv", "str"), &Z_Parse::from_csv);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_yaml", "data"), &Z_Parse::to_yaml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_yaml", "str"), &Z_Parse::from_yaml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_ini", "data"), &Z_Parse::to_ini);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_ini", "str"), &Z_Parse::from_ini);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("to_toml", "data"), &Z_Parse::to_toml);
	ClassDB::bind_static_method("Z_Parse", D_METHOD("from_toml", "str"), &Z_Parse::from_toml);
}

String Z_Parse::to_csv(const Variant &p_data) {
	TypedArray<Dictionary> rows;
	if (p_data.get_type() == Variant::DICTIONARY) {
		rows.push_back(Dictionary(p_data));
	} else if (p_data.get_type() == Variant::ARRAY) {
		const Array a = p_data;
		for (int i = 0; i < a.size(); i++) {
			if (a[i].get_type() != Variant::DICTIONARY) {
				UtilityFunctions::push_warning("Z_Parse.to_CSV: skipping non-Dictionary row at index " + String::num_int64(i) + ".");
				continue;
			}
			rows.push_back(Dictionary(a[i]));
		}
	} else if (p_data.get_type() != Variant::NIL) {
		UtilityFunctions::push_warning("Z_Parse.to_CSV: expected Dictionary or Array[Dictionary].");
		return String();
	}
	if (rows.is_empty()) {
		return String();
	}

	PackedStringArray headers;
	_collect_csv_headers(rows, headers);
	if (headers.is_empty()) {
		return String();
	}

	String out;
	for (int h = 0; h < headers.size(); h++) {
		if (h > 0) {
			out += ",";
		}
		out += _csv_escape(headers[h]);
	}
	for (int r = 0; r < rows.size(); r++) {
		out += "\n";
		const Dictionary row = rows[r];
		for (int h = 0; h < headers.size(); h++) {
			if (h > 0) {
				out += ",";
			}
			if (row.has(headers[h])) {
				out += _csv_escape(_csv_cell(row[headers[h]]));
			}
		}
	}
	return out;
}

TypedArray<Dictionary> Z_Parse::from_csv(const String &p_str) {
	TypedArray<Dictionary> out;
	if (p_str.strip_edges().is_empty()) {
		return out;
	}
	Vector<PackedStringArray> records;
	_csv_parse_records(p_str, records);
	if (records.is_empty()) {
		return out;
	}
	const PackedStringArray headers = records[0];
	for (int r = 1; r < records.size(); r++) {
		const PackedStringArray rec = records[r];
		bool any = false;
		for (int c = 0; c < rec.size(); c++) {
			if (!rec[c].is_empty()) {
				any = true;
				break;
			}
		}
		if (!any) {
			continue;
		}
		Dictionary row;
		const int cols = headers.size() < rec.size() ? headers.size() : rec.size();
		for (int c = 0; c < cols; c++) {
			if (headers[c].is_empty()) {
				continue;
			}
			row[headers[c]] = _csv_parse_cell(rec[c]);
		}
		out.push_back(row);
	}
	return out;
}

String Z_Parse::to_yaml(const Dictionary &p_data) {
	if (p_data.is_empty()) {
		return String();
	}
	String out;
	_yaml_write(p_data, 0, false, out);
	if (out.ends_with("\n")) {
		out = out.substr(0, out.length() - 1);
	}
	return out;
}

Dictionary Z_Parse::from_yaml(const String &p_str) {
	if (p_str.strip_edges().is_empty()) {
		return Dictionary();
	}
	const Vector<YLine> lines = _yaml_prep(p_str);
	if (lines.is_empty()) {
		return Dictionary();
	}
	int idx = 0;
	const Variant root = _yaml_parse_node(lines, idx, 0);
	if (root.get_type() == Variant::DICTIONARY) {
		return root;
	}
	Dictionary wrap;
	wrap["_"] = root;
	return wrap;
}

String Z_Parse::to_ini(const Dictionary &p_data) {
	if (p_data.is_empty()) {
		return String();
	}
	String out;
	_ini_write_section(String(), p_data, out);
	if (out.ends_with("\n")) {
		out = out.substr(0, out.length() - 1);
	}
	return out;
}

Dictionary Z_Parse::from_ini(const String &p_str) {
	Dictionary root;
	if (p_str.strip_edges().is_empty()) {
		return root;
	}
	const String norm = p_str.replace("\r\n", "\n").replace("\r", "\n");
	const PackedStringArray lines = norm.split("\n");
	PackedStringArray section_path;
	for (int li = 0; li < lines.size(); li++) {
		const String raw = _ini_strip_inline_comment(lines[li]);
		if (raw.is_empty()) {
			continue;
		}
		if (raw.begins_with("[") && raw.ends_with("]")) {
			section_path = _split_dotted(raw.substr(1, raw.length() - 2).strip_edges());
			_ini_ensure_path(root, section_path);
			continue;
		}
		const int eq = _find_unquoted(raw, '=');
		if (eq < 0) {
			continue;
		}
		const String key = raw.substr(0, eq).strip_edges();
		String val = raw.substr(eq + 1).strip_edges();
		if (key.is_empty()) {
			continue;
		}
		if ((val.begins_with("\"") && val.ends_with("\"")) || (val.begins_with("'") && val.ends_with("'"))) {
			int i = 0;
			val = _parse_quoted_at(val, i);
			_ini_ensure_path(root, section_path)[key] = val;
		} else {
			_ini_ensure_path(root, section_path)[key] = _parse_typed_scalar(val, SCALAR_INI);
		}
	}
	return root;
}

String Z_Parse::to_toml(const Dictionary &p_data) {
	if (p_data.is_empty()) {
		return String();
	}
	String out;
	_toml_write_table(String(), p_data, false, out);
	if (out.ends_with("\n")) {
		out = out.substr(0, out.length() - 1);
	}
	return out;
}

Dictionary Z_Parse::from_toml(const String &p_str) {
	Dictionary root;
	if (p_str.strip_edges().is_empty()) {
		return root;
	}
	Scan sc(p_str);
	PackedStringArray table_path;
	bool table_is_array = false;
	Dictionary current = root;
	sc.skip_ws_nl_comments();
	while (!sc.eof()) {
		if (sc.peek() == '[') {
			const bool arr = sc.peek(1) == '[';
			sc.take();
			if (arr) {
				sc.take();
			}
			table_path = _toml_parse_key(sc);
			sc.skip_ws();
			if (sc.peek() == ']') {
				sc.take();
			}
			if (arr && sc.peek() == ']') {
				sc.take();
			}
			table_is_array = arr;
			current = arr ? _toml_ensure_array_table(root, table_path) : _toml_current_from_path(root, table_path);
			sc.skip_ws_nl_comments();
			continue;
		}
		const PackedStringArray key_path = _toml_parse_key(sc);
		sc.skip_ws();
		if (sc.peek() == '=') {
			sc.take();
		}
		sc.skip_ws();
		const Variant val = _toml_parse_value(sc);
		_toml_assign_dotted(current, key_path, val);
		sc.skip_ws_nl_comments();
	}
	(void)table_is_array;
	return root;
}
