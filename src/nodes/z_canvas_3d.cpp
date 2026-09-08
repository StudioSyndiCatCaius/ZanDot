#include "z_canvas_3d.h"

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void ZCanvas3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_clamp_to_screen", "enabled"), &ZCanvas3D::set_clamp_to_screen);
	ClassDB::bind_method(D_METHOD("get_clamp_to_screen"), &ZCanvas3D::get_clamp_to_screen);

	ClassDB::bind_method(D_METHOD("set_screen_clamp_margins", "margins"), &ZCanvas3D::set_screen_clamp_margins);
	ClassDB::bind_method(D_METHOD("get_screen_clamp_margins"), &ZCanvas3D::get_screen_clamp_margins);

	ClassDB::bind_method(D_METHOD("set_screen_offset", "offset"), &ZCanvas3D::set_screen_offset);
	ClassDB::bind_method(D_METHOD("get_screen_offset"), &ZCanvas3D::get_screen_offset);

	ClassDB::bind_method(D_METHOD("set_center_content", "enabled"), &ZCanvas3D::set_center_content);
	ClassDB::bind_method(D_METHOD("get_center_content"), &ZCanvas3D::get_center_content);

	ClassDB::bind_method(D_METHOD("set_scale_by_distance", "enabled"), &ZCanvas3D::set_scale_by_distance);
	ClassDB::bind_method(D_METHOD("get_scale_by_distance"), &ZCanvas3D::get_scale_by_distance);

	ClassDB::bind_method(D_METHOD("set_distance_scale_reference", "distance"), &ZCanvas3D::set_distance_scale_reference);
	ClassDB::bind_method(D_METHOD("get_distance_scale_reference"), &ZCanvas3D::get_distance_scale_reference);

	ClassDB::bind_method(D_METHOD("set_distance_scale_min", "min_scale"), &ZCanvas3D::set_distance_scale_min);
	ClassDB::bind_method(D_METHOD("get_distance_scale_min"), &ZCanvas3D::get_distance_scale_min);

	ClassDB::bind_method(D_METHOD("set_distance_scale_max", "max_scale"), &ZCanvas3D::set_distance_scale_max);
	ClassDB::bind_method(D_METHOD("get_distance_scale_max"), &ZCanvas3D::get_distance_scale_max);

	ClassDB::bind_method(D_METHOD("set_use_distance_fade", "enabled"), &ZCanvas3D::set_use_distance_fade);
	ClassDB::bind_method(D_METHOD("get_use_distance_fade"), &ZCanvas3D::get_use_distance_fade);

	ClassDB::bind_method(D_METHOD("set_distance_fade_begin", "distance"), &ZCanvas3D::set_distance_fade_begin);
	ClassDB::bind_method(D_METHOD("get_distance_fade_begin"), &ZCanvas3D::get_distance_fade_begin);

	ClassDB::bind_method(D_METHOD("set_distance_fade_end", "distance"), &ZCanvas3D::set_distance_fade_end);
	ClassDB::bind_method(D_METHOD("get_distance_fade_end"), &ZCanvas3D::get_distance_fade_end);

	ClassDB::bind_method(D_METHOD("set_hide_when_behind_camera", "enabled"), &ZCanvas3D::set_hide_when_behind_camera);
	ClassDB::bind_method(D_METHOD("get_hide_when_behind_camera"), &ZCanvas3D::get_hide_when_behind_camera);

	ClassDB::bind_method(D_METHOD("get_screen_position"), &ZCanvas3D::get_screen_position);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clamp_to_screen"), "set_clamp_to_screen", "get_clamp_to_screen");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "screen_clamp_margins"), "set_screen_clamp_margins", "get_screen_clamp_margins");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "screen_offset"), "set_screen_offset", "get_screen_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "center_content"), "set_center_content", "get_center_content");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_when_behind_camera"), "set_hide_when_behind_camera", "get_hide_when_behind_camera");

	ADD_GROUP("Distance Scale", "distance_scale_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scale_by_distance"), "set_scale_by_distance", "get_scale_by_distance");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_scale_reference", PROPERTY_HINT_RANGE, "0.01,1000,0.01,or_greater"), "set_distance_scale_reference", "get_distance_scale_reference");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_scale_min", PROPERTY_HINT_RANGE, "0.01,10,0.01,or_greater"), "set_distance_scale_min", "get_distance_scale_min");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_scale_max", PROPERTY_HINT_RANGE, "0.01,10,0.01,or_greater"), "set_distance_scale_max", "get_distance_scale_max");

	ADD_GROUP("Distance Fade", "distance_fade_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_distance_fade"), "set_use_distance_fade", "get_use_distance_fade");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_fade_begin", PROPERTY_HINT_RANGE, "0,1000,0.1,or_greater"), "set_distance_fade_begin", "get_distance_fade_begin");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_fade_end", PROPERTY_HINT_RANGE, "0.01,1000,0.1,or_greater"), "set_distance_fade_end", "get_distance_fade_end");
}

ZCanvas3D::ZCanvas3D() {
}

ZCanvas3D::~ZCanvas3D() {
}

void ZCanvas3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_process(true);
		} break;
		case NOTIFICATION_READY: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				_prepare_canvas_children();
			}
		} break;
		case NOTIFICATION_CHILD_ORDER_CHANGED: {
			if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
				_prepare_canvas_children();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (Engine::get_singleton()->is_editor_hint()) {
				break;
			}
			_update_canvas();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			child_cache.clear();
		} break;
		default:
			break;
	}
}

void ZCanvas3D::_prepare_canvas_children() {
	const int count = get_child_count();
	for (int i = 0; i < count; i++) {
		CanvasItem *ci = Object::cast_to<CanvasItem>(get_child(i));
		if (ci == nullptr) {
			continue;
		}
		_prepare_child(ci);
	}
}

void ZCanvas3D::_prepare_child(CanvasItem *p_item) {
	if (p_item == nullptr) {
		return;
	}

	const ObjectID id(p_item->get_instance_id());
	ChildCache *existing = child_cache.getptr(id);
	if (existing != nullptr && existing->prepared) {
		return;
	}

	ChildCache cache;
	if (Control *c = Object::cast_to<Control>(p_item)) {
		cache.base_offset = c->get_position();
		cache.base_scale = c->get_scale();
		cache.base_modulate = c->get_modulate();
	} else if (Node2D *n = Object::cast_to<Node2D>(p_item)) {
		cache.base_offset = n->get_position();
		cache.base_scale = n->get_scale();
		cache.base_modulate = n->get_modulate();
	} else {
		cache.base_offset = Vector2();
		cache.base_scale = Vector2(1, 1);
		cache.base_modulate = p_item->get_modulate();
	}
	cache.prepared = true;
	child_cache[id] = cache;

	// Top-level so the CanvasItem draws in viewport/screen space while remaining
	// a child of this Node3D in the scene tree (local offset is preserved above).
	if (!p_item->is_set_as_top_level()) {
		p_item->set_as_top_level(true);
	}
}

CanvasItem *ZCanvas3D::_first_canvas_child() const {
	const int count = get_child_count();
	for (int i = 0; i < count; i++) {
		if (CanvasItem *ci = Object::cast_to<CanvasItem>(get_child(i))) {
			return ci;
		}
	}
	return nullptr;
}

Vector2 ZCanvas3D::_content_size() const {
	// Combined bounds of direct Control children using authored offsets (not
	// top-level global positions).
	Vector2 max_extent;
	bool any = false;
	const int count = get_child_count();
	for (int i = 0; i < count; i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (c == nullptr) {
			continue;
		}
		const ChildCache *cache = child_cache.getptr(ObjectID(c->get_instance_id()));
		const Vector2 off = cache != nullptr ? cache->base_offset : Vector2();
		const Vector2 cs = c->get_size();
		const Vector2 ms = c->get_combined_minimum_size();
		const Vector2 use(Math::max(cs.x, ms.x), Math::max(cs.y, ms.y));
		max_extent.x = Math::max(max_extent.x, off.x + use.x);
		max_extent.y = Math::max(max_extent.y, off.y + use.y);
		any = true;
	}
	if (!any) {
		return Vector2();
	}
	return max_extent;
}

void ZCanvas3D::_update_canvas() {
	// Pick up newly added canvas children (dynamic add after ready).
	_prepare_canvas_children();

	Viewport *vp = get_viewport();
	if (vp == nullptr) {
		return;
	}
	Camera3D *cam = vp->get_camera_3d();
	const Vector3 world_pos = get_global_position();

	bool show = true;
	Vector2 screen;
	float dist_scale = 1.0f;
	float fade_alpha = 1.0f;

	if (cam == nullptr) {
		show = false;
	} else {
		if (hide_when_behind_camera && cam->is_position_behind(world_pos)) {
			show = false;
		} else {
			screen = cam->unproject_position(world_pos) + screen_offset;
			const float dist = cam->get_global_position().distance_to(world_pos);

			if (scale_by_distance) {
				const float ref = Math::max(distance_scale_reference, 0.001f);
				const float d = Math::max(dist, 0.001f);
				dist_scale = ref / d;
				const float lo = Math::min(distance_scale_min, distance_scale_max);
				const float hi = Math::max(distance_scale_min, distance_scale_max);
				dist_scale = Math::clamp(dist_scale, lo, hi);
			}

			if (use_distance_fade) {
				const float begin = Math::max(distance_fade_begin, 0.0f);
				const float span = Math::max(distance_fade_end, 0.001f);
				if (dist <= begin) {
					fade_alpha = 1.0f;
				} else if (dist >= begin + span) {
					fade_alpha = 0.0f;
					show = false;
				} else {
					fade_alpha = 1.0f - ((dist - begin) / span);
				}
			}

			Vector2 content_size = _content_size() * dist_scale;
			if (center_content) {
				screen -= content_size * 0.5f;
			}

			if (clamp_to_screen && show) {
				const Rect2 visible = vp->get_visible_rect();
				const float left = visible.position.x + screen_clamp_margins.position.x;
				const float top = visible.position.y + screen_clamp_margins.position.y;
				const float right = visible.position.x + visible.size.x - screen_clamp_margins.size.x;
				const float bottom = visible.position.y + visible.size.y - screen_clamp_margins.size.y;

				const float max_x = Math::max(left, right - content_size.x);
				const float max_y = Math::max(top, bottom - content_size.y);
				screen.x = Math::clamp(screen.x, left, max_x);
				screen.y = Math::clamp(screen.y, top, max_y);
			}
		}
	}

	const int count = get_child_count();
	for (int i = 0; i < count; i++) {
		CanvasItem *ci = Object::cast_to<CanvasItem>(get_child(i));
		if (ci == nullptr) {
			continue;
		}

		const ObjectID id(ci->get_instance_id());
		const ChildCache *cache = child_cache.getptr(id);
		if (cache == nullptr) {
			continue;
		}

		ci->set_visible(show);
		if (!show) {
			continue;
		}

		const Vector2 scaled_offset = cache->base_offset * dist_scale;
		const Vector2 pos = screen + scaled_offset;
		const Vector2 scale = cache->base_scale * dist_scale;
		Color modulate = cache->base_modulate;
		modulate.a = cache->base_modulate.a * fade_alpha;

		if (Control *c = Object::cast_to<Control>(ci)) {
			c->set_global_position(pos);
			c->set_scale(scale);
			c->set_modulate(modulate);
		} else if (Node2D *n = Object::cast_to<Node2D>(ci)) {
			n->set_global_position(pos);
			n->set_scale(scale);
			n->set_modulate(modulate);
		} else {
			ci->set_modulate(modulate);
		}
	}
}

Vector2 ZCanvas3D::get_screen_position() const {
	Viewport *vp = get_viewport();
	if (vp == nullptr) {
		return Vector2();
	}
	Camera3D *cam = vp->get_camera_3d();
	if (cam == nullptr) {
		return Vector2();
	}
	const Vector3 world_pos = get_global_position();
	if (hide_when_behind_camera && cam->is_position_behind(world_pos)) {
		return Vector2();
	}
	return cam->unproject_position(world_pos) + screen_offset;
}

void ZCanvas3D::set_clamp_to_screen(bool p_enabled) {
	clamp_to_screen = p_enabled;
}

bool ZCanvas3D::get_clamp_to_screen() const {
	return clamp_to_screen;
}

void ZCanvas3D::set_screen_clamp_margins(const Rect2 &p_margins) {
	screen_clamp_margins = p_margins;
}

Rect2 ZCanvas3D::get_screen_clamp_margins() const {
	return screen_clamp_margins;
}

void ZCanvas3D::set_screen_offset(const Vector2 &p_offset) {
	screen_offset = p_offset;
}

Vector2 ZCanvas3D::get_screen_offset() const {
	return screen_offset;
}

void ZCanvas3D::set_center_content(bool p_enabled) {
	center_content = p_enabled;
}

bool ZCanvas3D::get_center_content() const {
	return center_content;
}

void ZCanvas3D::set_scale_by_distance(bool p_enabled) {
	scale_by_distance = p_enabled;
}

bool ZCanvas3D::get_scale_by_distance() const {
	return scale_by_distance;
}

void ZCanvas3D::set_distance_scale_reference(float p_distance) {
	distance_scale_reference = Math::max(p_distance, 0.001f);
}

float ZCanvas3D::get_distance_scale_reference() const {
	return distance_scale_reference;
}

void ZCanvas3D::set_distance_scale_min(float p_min) {
	distance_scale_min = Math::max(p_min, 0.001f);
}

float ZCanvas3D::get_distance_scale_min() const {
	return distance_scale_min;
}

void ZCanvas3D::set_distance_scale_max(float p_max) {
	distance_scale_max = Math::max(p_max, 0.001f);
}

float ZCanvas3D::get_distance_scale_max() const {
	return distance_scale_max;
}

void ZCanvas3D::set_use_distance_fade(bool p_enabled) {
	use_distance_fade = p_enabled;
}

bool ZCanvas3D::get_use_distance_fade() const {
	return use_distance_fade;
}

void ZCanvas3D::set_distance_fade_begin(float p_distance) {
	distance_fade_begin = Math::max(p_distance, 0.0f);
}

float ZCanvas3D::get_distance_fade_begin() const {
	return distance_fade_begin;
}

void ZCanvas3D::set_distance_fade_end(float p_distance) {
	distance_fade_end = Math::max(p_distance, 0.001f);
}

float ZCanvas3D::get_distance_fade_end() const {
	return distance_fade_end;
}

void ZCanvas3D::set_hide_when_behind_camera(bool p_enabled) {
	hide_when_behind_camera = p_enabled;
}

bool ZCanvas3D::get_hide_when_behind_camera() const {
	return hide_when_behind_camera;
}
