#include "flow_graph_loader.h"

#include "flow_graph.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void FlowGraphLoader::_bind_methods() {
}

FlowGraphLoader::FlowGraphLoader() {
}

FlowGraphLoader::~FlowGraphLoader() {
}

PackedStringArray FlowGraphLoader::_get_recognized_extensions() const {
	PackedStringArray exts;
	exts.push_back("gdflow");
	return exts;
}

bool FlowGraphLoader::_recognize_path(const String &p_path, const StringName &p_type) const {
	if (p_path.get_extension().to_lower() != "gdflow") {
		return false;
	}
	return p_type == StringName() || p_type == StringName("FlowGraph") || p_type == StringName("Resource");
}

bool FlowGraphLoader::_handles_type(const StringName &p_type) const {
	return p_type == StringName("FlowGraph") || p_type == StringName("Resource");
}

String FlowGraphLoader::_get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == "gdflow") {
		return "FlowGraph";
	}
	return "";
}

Variant FlowGraphLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
	(void)p_use_sub_threads;
	(void)p_cache_mode;

	String path = p_original_path.is_empty() ? p_path : p_original_path;
	if (!FileAccess::file_exists(path)) {
		UtilityFunctions::push_error("FlowGraphLoader: file not found: ", path);
		return Variant();
	}

	// Legacy JSON format only. Prefer re-saving as .tres so script @export fields persist.
	String text = FileAccess::get_file_as_string(path);
	Variant parsed = JSON::parse_string(text);
	if (parsed.get_type() != Variant::DICTIONARY) {
		UtilityFunctions::push_error("FlowGraphLoader: invalid JSON in ", path, " (legacy .gdflow). Prefer FlowGraph .tres.");
		return Variant();
	}

	UtilityFunctions::push_warning("FlowGraphLoader: loaded legacy .gdflow '", path, "'. Save as .tres to use Godot resource serialization (script properties were not stored in JSON).");

	Ref<FlowGraph> graph = FlowGraph::from_dict(parsed);
	if (graph.is_valid()) {
		// Don't bind path to .gdflow — next save should write .tres via the editor.
		graph->set_path("");
	}
	return graph;
}
