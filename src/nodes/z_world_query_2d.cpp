#include "z_world_query_2d.h"

#include "res/world_query_common.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void ZWorldQuery2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shapes", "shapes"), &ZWorldQuery2D::set_shapes);
	ClassDB::bind_method(D_METHOD("get_shapes"), &ZWorldQuery2D::get_shapes);

	ClassDB::bind_method(D_METHOD("set_utilities", "utilities"), &ZWorldQuery2D::set_utilities);
	ClassDB::bind_method(D_METHOD("get_utilities"), &ZWorldQuery2D::get_utilities);

	ClassDB::bind_method(D_METHOD("set_debug_draw", "enabled"), &ZWorldQuery2D::set_debug_draw);
	ClassDB::bind_method(D_METHOD("get_debug_draw"), &ZWorldQuery2D::get_debug_draw);

	ClassDB::bind_method(D_METHOD("set_debug_follow_node", "enabled"), &ZWorldQuery2D::set_debug_follow_node);
	ClassDB::bind_method(D_METHOD("get_debug_follow_node"), &ZWorldQuery2D::get_debug_follow_node);

	ClassDB::bind_method(D_METHOD("set_debug_circle_radius", "radius"), &ZWorldQuery2D::set_debug_circle_radius);
	ClassDB::bind_method(D_METHOD("get_debug_circle_radius"), &ZWorldQuery2D::get_debug_circle_radius);

	ClassDB::bind_method(D_METHOD("set_debug_low_color", "color"), &ZWorldQuery2D::set_debug_low_color);
	ClassDB::bind_method(D_METHOD("get_debug_low_color"), &ZWorldQuery2D::get_debug_low_color);

	ClassDB::bind_method(D_METHOD("set_debug_high_color", "color"), &ZWorldQuery2D::set_debug_high_color);
	ClassDB::bind_method(D_METHOD("get_debug_high_color"), &ZWorldQuery2D::get_debug_high_color);

	ClassDB::bind_method(D_METHOD("run_query", "querier"), &ZWorldQuery2D::run_query);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "shapes", PROPERTY_HINT_ARRAY_TYPE, "WorldQueryShape"), "set_shapes", "get_shapes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "utilities", PROPERTY_HINT_ARRAY_TYPE, "WorldQueryUtility"), "set_utilities", "get_utilities");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_draw"), "set_debug_draw", "get_debug_draw");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_follow_node"), "set_debug_follow_node", "get_debug_follow_node");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "debug_circle_radius", PROPERTY_HINT_RANGE, "1,64,0.5,or_greater"), "set_debug_circle_radius", "get_debug_circle_radius");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "debug_low_color"), "set_debug_low_color", "get_debug_low_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "debug_high_color"), "set_debug_high_color", "get_debug_high_color");

	ADD_SIGNAL(MethodInfo("on_query_finish", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "WorldQueryResult")));
}

ZWorldQuery2D::ZWorldQuery2D() {
	set_notify_transform(true);
}

ZWorldQuery2D::~ZWorldQuery2D() {
}

void ZWorldQuery2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
		case NOTIFICATION_TRANSFORM_CHANGED: {
			_editor_preview_update();
		} break;
		default:
			break;
	}
}

void ZWorldQuery2D::set_shapes(const TypedArray<WorldQueryShape> &p_shapes) {
	_disconnect_resource_signals();
	shapes = p_shapes;
	_connect_resource_signals();
	_editor_preview_update();
}

TypedArray<WorldQueryShape> ZWorldQuery2D::get_shapes() const {
	return shapes;
}

void ZWorldQuery2D::set_utilities(const TypedArray<WorldQueryUtility> &p_utilities) {
	_disconnect_resource_signals();
	utilities = p_utilities;
	_connect_resource_signals();
	_editor_preview_update();
}

TypedArray<WorldQueryUtility> ZWorldQuery2D::get_utilities() const {
	return utilities;
}

void ZWorldQuery2D::set_debug_draw(bool p_enabled) {
	debug_draw = p_enabled;
	if (!debug_draw) {
		_clear_debug_draw();
	} else {
		_editor_preview_update();
	}
}

bool ZWorldQuery2D::get_debug_draw() const {
	return debug_draw;
}

void ZWorldQuery2D::set_debug_follow_node(bool p_enabled) {
	debug_follow_node = p_enabled;
	_editor_preview_update();
}

bool ZWorldQuery2D::get_debug_follow_node() const {
	return debug_follow_node;
}

void ZWorldQuery2D::set_debug_circle_radius(float p_radius) {
	debug_circle_radius = p_radius;
	_editor_preview_update();
}

float ZWorldQuery2D::get_debug_circle_radius() const {
	return debug_circle_radius;
}

void ZWorldQuery2D::set_debug_low_color(const Color &p_color) {
	debug_low_color = p_color;
	_editor_preview_update();
}

Color ZWorldQuery2D::get_debug_low_color() const {
	return debug_low_color;
}

void ZWorldQuery2D::set_debug_high_color(const Color &p_color) {
	debug_high_color = p_color;
	_editor_preview_update();
}

Color ZWorldQuery2D::get_debug_high_color() const {
	return debug_high_color;
}

void ZWorldQuery2D::_ensure_debug_canvas() {
	if (debug_canvas != nullptr) {
		return;
	}
	debug_canvas = memnew(ZWorldQueryDebugCanvas2D);
	add_child(debug_canvas);
}

void ZWorldQuery2D::_clear_debug_draw() {
	if (debug_canvas == nullptr) {
		return;
	}
	debug_canvas->queue_free();
	debug_canvas = nullptr;
}

void ZWorldQuery2D::_update_debug_draw(const PackedVector3Array &p_positions, const PackedFloat32Array &p_scores) {
	_ensure_debug_canvas();

	// debug_follow_node false (default): render in world space, independent of
	// this node's own transform. true: parented normally, moves with this node.
	debug_canvas->set_as_top_level(!debug_follow_node);
	if (!debug_follow_node) {
		debug_canvas->set_transform(Transform2D());
	}

	debug_canvas->set_points(p_positions, p_scores, debug_circle_radius, debug_low_color, debug_high_color);
}

void ZWorldQuery2D::_connect_resource_signals() {
	const Callable cb = callable_mp(this, &ZWorldQuery2D::_editor_preview_update);
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

void ZWorldQuery2D::_disconnect_resource_signals() {
	const Callable cb = callable_mp(this, &ZWorldQuery2D::_editor_preview_update);
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

void ZWorldQuery2D::_editor_preview_update() {
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

Ref<WorldQueryResult> ZWorldQuery2D::run_query(Node *p_querier) {
	Ref<WorldQueryResult> result = world_query_run(shapes, utilities, p_querier);

	if (debug_draw) {
		_update_debug_draw(result->get_ordered_positions(), result->get_ordered_scores());
	}

	emit_signal("on_query_finish", result);
	return result;
}
