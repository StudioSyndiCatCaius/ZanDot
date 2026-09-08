#include "z_scene_transit.h"

#include "z.h"

#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/collision_object2d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/collision_polygon2d.hpp>
#include <godot_cpp/classes/collision_polygon3d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/cylinder_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

Ref<StandardMaterial3D> make_unshaded(const Color &p_color) {
	Ref<StandardMaterial3D> mat;
	mat.instantiate();
	mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	mat->set_albedo(p_color);
	mat->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	mat->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	return mat;
}

// True when p_node is a possessed pawn root or lives under one.
bool is_player_pawn(Node *p_node) {
	if (p_node == nullptr) {
		return false;
	}
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return false;
	}
	const TypedArray<ZPlayer> players = z->get_players();
	for (int i = 0; i < players.size(); i++) {
		Ref<ZPlayer> player = players[i];
		if (player.is_null()) {
			continue;
		}
		Node *pawn = player->get_possessed_scene();
		if (pawn == nullptr) {
			continue;
		}
		if (pawn == p_node || pawn->is_ancestor_of(p_node)) {
			return true;
		}
	}
	return false;
}

// A transit already running would re-enter on the outgoing frame's overlaps.
bool can_auto_transit() {
	Z *z = Z::get_singleton();
	return z != nullptr && !z->is_scene_transiting();
}

} // namespace

// --- ZSceneTransit3D ---

void ZSceneTransit3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linked_scene", "scene"), &ZSceneTransit3D::set_linked_scene);
	ClassDB::bind_method(D_METHOD("get_linked_scene"), &ZSceneTransit3D::get_linked_scene);
	ClassDB::bind_method(D_METHOD("set_link_id", "id"), &ZSceneTransit3D::set_link_id);
	ClassDB::bind_method(D_METHOD("get_link_id"), &ZSceneTransit3D::get_link_id);
	ClassDB::bind_method(D_METHOD("set_transit_on_player_enter", "enabled"), &ZSceneTransit3D::set_transit_on_player_enter);
	ClassDB::bind_method(D_METHOD("get_transit_on_player_enter"), &ZSceneTransit3D::get_transit_on_player_enter);
	ClassDB::bind_method(D_METHOD("transit", "skip_fade_out", "skip_fade_in"), &ZSceneTransit3D::transit, DEFVAL(false), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &ZSceneTransit3D::_on_body_entered);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "linked_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_linked_scene", "get_linked_scene");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "link_id"), "set_link_id", "get_link_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transit_on_player_enter"), "set_transit_on_player_enter", "get_transit_on_player_enter");
}

ZSceneTransit3D::ZSceneTransit3D() {
	set_monitoring(true);
}

ZSceneTransit3D::~ZSceneTransit3D() {
}

void ZSceneTransit3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		add_to_group("z_scene_transit_3d");
		if (Engine::get_singleton()->is_editor_hint()) {
			_ensure_editor_visual();
		}
	} else if (p_what == NOTIFICATION_READY) {
		_ensure_collision_shape();
		if (!Engine::get_singleton()->is_editor_hint()) {
			connect("body_entered", Callable(this, "_on_body_entered"));
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		remove_from_group("z_scene_transit_3d");
		_clear_editor_visual();
	}
}

void ZSceneTransit3D::_ensure_collision_shape() {
	if (auto_shape != nullptr) {
		return;
	}
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		if (Object::cast_to<CollisionShape3D>(child) != nullptr || Object::cast_to<CollisionPolygon3D>(child) != nullptr) {
			return;
		}
	}

	// No shape authored — match the editor gizmo box so the portal is usable out of the box.
	CollisionShape3D *cs = memnew(CollisionShape3D);
	cs->set_name("_ZSceneTransit3DShape");
	Ref<BoxShape3D> box;
	box.instantiate();
	box->set_size(Vector3(1.0f, 2.0f, 1.0f));
	cs->set_shape(box);
	cs->set_position(Vector3(0, 1.0f, 0));
	add_child(cs);
	cs->set_owner(nullptr);
	auto_shape = cs;
}

void ZSceneTransit3D::_on_body_entered(Node3D *p_body) {
	if (!transit_on_player_enter || !can_auto_transit()) {
		return;
	}
	if (!is_player_pawn(p_body)) {
		return;
	}
	transit();
}

void ZSceneTransit3D::_ensure_editor_visual() {
	if (editor_visual != nullptr || !is_inside_tree()) {
		return;
	}
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Node3D *root = memnew(Node3D);
	root->set_name("_ZSceneTransit3DGizmo");
	add_child(root);
	root->set_owner(nullptr);
	editor_visual = root;

	const Color col(0.2f, 0.75f, 1.0f, 0.45f);
	Ref<StandardMaterial3D> mat = make_unshaded(col);

	// Bounds box.
	MeshInstance3D *box_mi = memnew(MeshInstance3D);
	Ref<BoxMesh> box;
	box.instantiate();
	box->set_size(Vector3(1.0f, 2.0f, 1.0f));
	box_mi->set_mesh(box);
	box_mi->set_material_override(mat);
	box_mi->set_position(Vector3(0, 1.0f, 0));
	root->add_child(box_mi);

	// Forward arrow (along -Z / local forward in Godot).
	MeshInstance3D *shaft = memnew(MeshInstance3D);
	Ref<CylinderMesh> cyl;
	cyl.instantiate();
	cyl->set_top_radius(0.06f);
	cyl->set_bottom_radius(0.06f);
	cyl->set_height(0.9f);
	shaft->set_mesh(cyl);
	shaft->set_material_override(make_unshaded(Color(0.15f, 0.9f, 1.0f, 0.9f)));
	shaft->set_rotation_degrees(Vector3(90, 0, 0));
	shaft->set_position(Vector3(0, 1.0f, -0.55f));
	root->add_child(shaft);

	MeshInstance3D *head = memnew(MeshInstance3D);
	Ref<CylinderMesh> cone;
	cone.instantiate();
	cone->set_top_radius(0.0f);
	cone->set_bottom_radius(0.16f);
	cone->set_height(0.3f);
	head->set_mesh(cone);
	head->set_material_override(make_unshaded(Color(0.15f, 0.9f, 1.0f, 0.95f)));
	head->set_rotation_degrees(Vector3(90, 0, 0));
	head->set_position(Vector3(0, 1.0f, -1.15f));
	root->add_child(head);
}

void ZSceneTransit3D::_clear_editor_visual() {
	if (editor_visual != nullptr) {
		editor_visual->queue_free();
		editor_visual = nullptr;
	}
}

void ZSceneTransit3D::set_linked_scene(const Ref<PackedScene> &p_scene) {
	linked_scene = p_scene;
}

Ref<PackedScene> ZSceneTransit3D::get_linked_scene() const {
	return linked_scene;
}

void ZSceneTransit3D::set_link_id(const StringName &p_id) {
	link_id = p_id;
}

StringName ZSceneTransit3D::get_link_id() const {
	return link_id;
}

void ZSceneTransit3D::set_transit_on_player_enter(bool p_enabled) {
	transit_on_player_enter = p_enabled;
}

bool ZSceneTransit3D::get_transit_on_player_enter() const {
	return transit_on_player_enter;
}

void ZSceneTransit3D::transit(bool p_skip_fade_out, bool p_skip_fade_in) {
	if (linked_scene.is_null()) {
		UtilityFunctions::push_warning("ZSceneTransit3D.transit: linked_scene is null.");
		return;
	}
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return;
	}

	ZSceneTransitPending pending;
	pending.valid = true;
	pending.is_3d = true;
	pending.to_scene = linked_scene;
	pending.link_id = link_id;
	// from_scene: current scene resource if available.
	SceneTree *tree = get_tree();
	if (tree != nullptr && tree->get_current_scene() != nullptr) {
		const String path = tree->get_current_scene()->get_scene_file_path();
		if (!path.is_empty()) {
			pending.from_scene = ResourceLoader::get_singleton()->load(path);
		}
	}
	z->set_pending_scene_transit(pending);
	z->scene_transit(linked_scene, p_skip_fade_out, p_skip_fade_in);
}

// --- ZSceneTransit2D ---

void ZSceneTransit2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linked_scene", "scene"), &ZSceneTransit2D::set_linked_scene);
	ClassDB::bind_method(D_METHOD("get_linked_scene"), &ZSceneTransit2D::get_linked_scene);
	ClassDB::bind_method(D_METHOD("set_link_id", "id"), &ZSceneTransit2D::set_link_id);
	ClassDB::bind_method(D_METHOD("get_link_id"), &ZSceneTransit2D::get_link_id);
	ClassDB::bind_method(D_METHOD("set_transit_on_player_enter", "enabled"), &ZSceneTransit2D::set_transit_on_player_enter);
	ClassDB::bind_method(D_METHOD("get_transit_on_player_enter"), &ZSceneTransit2D::get_transit_on_player_enter);
	ClassDB::bind_method(D_METHOD("transit", "skip_fade_out", "skip_fade_in"), &ZSceneTransit2D::transit, DEFVAL(false), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &ZSceneTransit2D::_on_body_entered);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "linked_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_linked_scene", "get_linked_scene");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "link_id"), "set_link_id", "get_link_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transit_on_player_enter"), "set_transit_on_player_enter", "get_transit_on_player_enter");
}

ZSceneTransit2D::ZSceneTransit2D() {
	set_z_index(100);
	set_monitoring(true);
}

ZSceneTransit2D::~ZSceneTransit2D() {
}

void ZSceneTransit2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		add_to_group("z_scene_transit_2d");
		if (Engine::get_singleton()->is_editor_hint()) {
			queue_redraw();
		}
	} else if (p_what == NOTIFICATION_READY) {
		_ensure_collision_shape();
		if (!Engine::get_singleton()->is_editor_hint()) {
			connect("body_entered", Callable(this, "_on_body_entered"));
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		remove_from_group("z_scene_transit_2d");
	}
}

void ZSceneTransit2D::_ensure_collision_shape() {
	if (auto_shape != nullptr) {
		return;
	}
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		if (Object::cast_to<CollisionShape2D>(child) != nullptr || Object::cast_to<CollisionPolygon2D>(child) != nullptr) {
			return;
		}
	}

	// No shape authored — match the editor _draw box.
	CollisionShape2D *cs = memnew(CollisionShape2D);
	cs->set_name("_ZSceneTransit2DShape");
	Ref<RectangleShape2D> rect;
	rect.instantiate();
	rect->set_size(Vector2(64.0f, 96.0f));
	cs->set_shape(rect);
	add_child(cs);
	cs->set_owner(nullptr);
	auto_shape = cs;
}

void ZSceneTransit2D::_on_body_entered(Node2D *p_body) {
	if (!transit_on_player_enter || !can_auto_transit()) {
		return;
	}
	if (!is_player_pawn(p_body)) {
		return;
	}
	transit();
}

void ZSceneTransit2D::set_linked_scene(const Ref<PackedScene> &p_scene) {
	linked_scene = p_scene;
}

Ref<PackedScene> ZSceneTransit2D::get_linked_scene() const {
	return linked_scene;
}

void ZSceneTransit2D::set_link_id(const StringName &p_id) {
	link_id = p_id;
}

StringName ZSceneTransit2D::get_link_id() const {
	return link_id;
}

void ZSceneTransit2D::set_transit_on_player_enter(bool p_enabled) {
	transit_on_player_enter = p_enabled;
}

bool ZSceneTransit2D::get_transit_on_player_enter() const {
	return transit_on_player_enter;
}

void ZSceneTransit2D::transit(bool p_skip_fade_out, bool p_skip_fade_in) {
	if (linked_scene.is_null()) {
		UtilityFunctions::push_warning("ZSceneTransit2D.transit: linked_scene is null.");
		return;
	}
	Z *z = Z::get_singleton();
	if (z == nullptr) {
		return;
	}

	ZSceneTransitPending pending;
	pending.valid = true;
	pending.is_3d = false;
	pending.to_scene = linked_scene;
	pending.link_id = link_id;
	SceneTree *tree = get_tree();
	if (tree != nullptr && tree->get_current_scene() != nullptr) {
		const String path = tree->get_current_scene()->get_scene_file_path();
		if (!path.is_empty()) {
			pending.from_scene = ResourceLoader::get_singleton()->load(path);
		}
	}
	z->set_pending_scene_transit(pending);
	z->scene_transit(linked_scene, p_skip_fade_out, p_skip_fade_in);
}

void ZSceneTransit2D::_draw() {
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	const Color box_col(0.2f, 0.75f, 1.0f, 0.35f);
	const Color arrow_col(0.15f, 0.9f, 1.0f, 0.95f);
	const Rect2 box(-32, -48, 64, 96);
	draw_rect(box, box_col, true);
	draw_rect(box, arrow_col, false, 2.0f);
	// Arrow pointing up (local "forward" for 2D top-down / side).
	draw_line(Vector2(0, 20), Vector2(0, -36), arrow_col, 3.0f);
	PackedVector2Array head;
	head.push_back(Vector2(0, -44));
	head.push_back(Vector2(-10, -28));
	head.push_back(Vector2(10, -28));
	draw_colored_polygon(head, arrow_col);
}
