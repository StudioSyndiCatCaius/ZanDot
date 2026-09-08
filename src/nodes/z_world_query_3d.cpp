#include "z_world_query_3d.h"

#include "res/world_query_common.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void ZWorldQuery3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shapes", "shapes"), &ZWorldQuery3D::set_shapes);
	ClassDB::bind_method(D_METHOD("get_shapes"), &ZWorldQuery3D::get_shapes);

	ClassDB::bind_method(D_METHOD("set_utilities", "utilities"), &ZWorldQuery3D::set_utilities);
	ClassDB::bind_method(D_METHOD("get_utilities"), &ZWorldQuery3D::get_utilities);

	ClassDB::bind_method(D_METHOD("set_debug_draw", "enabled"), &ZWorldQuery3D::set_debug_draw);
	ClassDB::bind_method(D_METHOD("get_debug_draw"), &ZWorldQuery3D::get_debug_draw);

	ClassDB::bind_method(D_METHOD("set_debug_follow_node", "enabled"), &ZWorldQuery3D::set_debug_follow_node);
	ClassDB::bind_method(D_METHOD("get_debug_follow_node"), &ZWorldQuery3D::get_debug_follow_node);

	ClassDB::bind_method(D_METHOD("set_debug_sphere_radius", "radius"), &ZWorldQuery3D::set_debug_sphere_radius);
	ClassDB::bind_method(D_METHOD("get_debug_sphere_radius"), &ZWorldQuery3D::get_debug_sphere_radius);

	ClassDB::bind_method(D_METHOD("set_debug_low_color", "color"), &ZWorldQuery3D::set_debug_low_color);
	ClassDB::bind_method(D_METHOD("get_debug_low_color"), &ZWorldQuery3D::get_debug_low_color);

	ClassDB::bind_method(D_METHOD("set_debug_high_color", "color"), &ZWorldQuery3D::set_debug_high_color);
	ClassDB::bind_method(D_METHOD("get_debug_high_color"), &ZWorldQuery3D::get_debug_high_color);

	ClassDB::bind_method(D_METHOD("run_query", "querier"), &ZWorldQuery3D::run_query);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "shapes", PROPERTY_HINT_ARRAY_TYPE, "WorldQueryShape"), "set_shapes", "get_shapes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "utilities", PROPERTY_HINT_ARRAY_TYPE, "WorldQueryUtility"), "set_utilities", "get_utilities");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_draw"), "set_debug_draw", "get_debug_draw");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_follow_node"), "set_debug_follow_node", "get_debug_follow_node");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "debug_sphere_radius", PROPERTY_HINT_RANGE, "0.01,5,0.01,or_greater"), "set_debug_sphere_radius", "get_debug_sphere_radius");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "debug_low_color"), "set_debug_low_color", "get_debug_low_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "debug_high_color"), "set_debug_high_color", "get_debug_high_color");

	ADD_SIGNAL(MethodInfo("on_query_finish", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "WorldQueryResult")));
}

ZWorldQuery3D::ZWorldQuery3D() {
	set_notify_transform(true);
}

ZWorldQuery3D::~ZWorldQuery3D() {
}

void ZWorldQuery3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
		case NOTIFICATION_TRANSFORM_CHANGED: {
			_editor_preview_update();
		} break;
		default:
			break;
	}
}

void ZWorldQuery3D::set_shapes(const TypedArray<WorldQueryShape> &p_shapes) {
	_disconnect_resource_signals();
	shapes = p_shapes;
	_connect_resource_signals();
	_editor_preview_update();
}

TypedArray<WorldQueryShape> ZWorldQuery3D::get_shapes() const {
	return shapes;
}

void ZWorldQuery3D::set_utilities(const TypedArray<WorldQueryUtility> &p_utilities) {
	_disconnect_resource_signals();
	utilities = p_utilities;
	_connect_resource_signals();
	_editor_preview_update();
}

TypedArray<WorldQueryUtility> ZWorldQuery3D::get_utilities() const {
	return utilities;
}

void ZWorldQuery3D::set_debug_draw(bool p_enabled) {
	debug_draw = p_enabled;
	if (!debug_draw) {
		_clear_debug_draw();
	} else {
		_editor_preview_update();
	}
}

bool ZWorldQuery3D::get_debug_draw() const {
	return debug_draw;
}

void ZWorldQuery3D::set_debug_follow_node(bool p_enabled) {
	debug_follow_node = p_enabled;
	_editor_preview_update();
}

bool ZWorldQuery3D::get_debug_follow_node() const {
	return debug_follow_node;
}

void ZWorldQuery3D::set_debug_sphere_radius(float p_radius) {
	debug_sphere_radius = p_radius;
	if (debug_instance != nullptr) {
		Ref<MultiMesh> mm = debug_instance->get_multimesh();
		if (mm.is_valid()) {
			Ref<SphereMesh> sphere = mm->get_mesh();
			if (sphere.is_valid()) {
				sphere->set_radius(debug_sphere_radius);
				sphere->set_height(debug_sphere_radius * 2.0f);
			}
		}
	}
}

float ZWorldQuery3D::get_debug_sphere_radius() const {
	return debug_sphere_radius;
}

void ZWorldQuery3D::set_debug_low_color(const Color &p_color) {
	debug_low_color = p_color;
	_editor_preview_update();
}

Color ZWorldQuery3D::get_debug_low_color() const {
	return debug_low_color;
}

void ZWorldQuery3D::set_debug_high_color(const Color &p_color) {
	debug_high_color = p_color;
	_editor_preview_update();
}

Color ZWorldQuery3D::get_debug_high_color() const {
	return debug_high_color;
}

void ZWorldQuery3D::_ensure_debug_instance() {
	if (debug_instance != nullptr) {
		return;
	}
	debug_instance = memnew(MultiMeshInstance3D);
	Ref<MultiMesh> mm;
	mm.instantiate();
	mm->set_transform_format(MultiMesh::TRANSFORM_3D);
	mm->set_use_colors(true);

	Ref<SphereMesh> sphere;
	sphere.instantiate();
	sphere->set_radius(debug_sphere_radius);
	sphere->set_height(debug_sphere_radius * 2.0f);

	Ref<StandardMaterial3D> mat;
	mat.instantiate();
	mat->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	sphere->set_material(mat);

	mm->set_mesh(sphere);
	debug_instance->set_multimesh(mm);
	add_child(debug_instance);
}

void ZWorldQuery3D::_clear_debug_draw() {
	if (debug_instance == nullptr) {
		return;
	}
	debug_instance->queue_free();
	debug_instance = nullptr;
}

void ZWorldQuery3D::_update_debug_draw(const PackedVector3Array &p_positions, const PackedFloat32Array &p_scores) {
	_ensure_debug_instance();

	// debug_follow_node false (default): render in world space, independent of
	// this node's own transform. true: parented normally, moves with this node.
	debug_instance->set_as_top_level(!debug_follow_node);
	if (!debug_follow_node) {
		debug_instance->set_transform(Transform3D());
	}

	Ref<MultiMesh> mm = debug_instance->get_multimesh();
	if (mm.is_null()) {
		return;
	}
	const int count = p_positions.size();
	mm->set_instance_count(count);
	for (int i = 0; i < count; i++) {
		Transform3D xf;
		// Instance transforms are local to debug_instance. With top_level +
		// identity transform, debug_instance's global transform is identity,
		// so world positions can be used directly (no to_local needed).
		xf.origin = debug_follow_node ? to_local(p_positions[i]) : p_positions[i];
		mm->set_instance_transform(i, xf);
		const float score = i < p_scores.size() ? p_scores[i] : 0.0f;
		mm->set_instance_color(i, debug_low_color.lerp(debug_high_color, score));
	}
}

void ZWorldQuery3D::_connect_resource_signals() {
	const Callable cb = callable_mp(this, &ZWorldQuery3D::_editor_preview_update);
	for (int i = 0; i < shapes.size(); i++) {
		Ref<Resource> res = shapes[i];
		if (res.is_valid() && !res->is_connected("changed", cb)) {
			res->connect("changed", cb);
		}
	}
	for (int i = 0; i < utilities.size(); i++) {
		Ref<Resource> res = utilities[i];
		if (res.is_valid() && !res->is_connected("changed", cb)) {
			res->connect("changed", cb);
		}
	}
}

void ZWorldQuery3D::_disconnect_resource_signals() {
	const Callable cb = callable_mp(this, &ZWorldQuery3D::_editor_preview_update);
	for (int i = 0; i < shapes.size(); i++) {
		Ref<Resource> res = shapes[i];
		if (res.is_valid() && res->is_connected("changed", cb)) {
			res->disconnect("changed", cb);
		}
	}
	for (int i = 0; i < utilities.size(); i++) {
		Ref<Resource> res = utilities[i];
		if (res.is_valid() && res->is_connected("changed", cb)) {
			res->disconnect("changed", cb);
		}
	}
}

void ZWorldQuery3D::_editor_preview_update() {
	if (!debug_draw) {
		return;
	}
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!is_inside_tree()) {
		return;
	}
	run_query(this);
}

Ref<WorldQueryResult> ZWorldQuery3D::run_query(Node *p_querier) {
	Ref<WorldQueryResult> result = world_query_run(shapes, utilities, p_querier);

	if (debug_draw) {
		_update_debug_draw(result->get_ordered_positions(), result->get_ordered_scores());
	}

	emit_signal("on_query_finish", result);
	return result;
}
