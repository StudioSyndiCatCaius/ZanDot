#include "z_player_start_3d.h"

#include <godot_cpp/classes/cylinder_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {

Ref<StandardMaterial3D> make_player_start_mat(const Color &p_color) {
	Ref<StandardMaterial3D> mat;
	mat.instantiate();
	mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	mat->set_albedo(p_color);
	mat->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	mat->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	return mat;
}

} // namespace

void ZPlayerStart3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_player_index", "index"), &ZPlayerStart3D::set_player_index);
	ClassDB::bind_method(D_METHOD("get_player_index"), &ZPlayerStart3D::get_player_index);

	ClassDB::bind_method(D_METHOD("set_linked_map", "map"), &ZPlayerStart3D::set_linked_map);
	ClassDB::bind_method(D_METHOD("get_linked_map"), &ZPlayerStart3D::get_linked_map);

	ClassDB::bind_method(D_METHOD("set_link_id", "id"), &ZPlayerStart3D::set_link_id);
	ClassDB::bind_method(D_METHOD("get_link_id"), &ZPlayerStart3D::get_link_id);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "player_index", PROPERTY_HINT_RANGE, "0,7,1"), "set_player_index", "get_player_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "linked_map", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_linked_map", "get_linked_map");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "link_id"), "set_link_id", "get_link_id");
}

ZPlayerStart3D::ZPlayerStart3D() {
}

ZPlayerStart3D::~ZPlayerStart3D() {
}

void ZPlayerStart3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		add_to_group("z_player_start");
		if (Engine::get_singleton()->is_editor_hint()) {
			_ensure_editor_visual();
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		remove_from_group("z_player_start");
		_clear_editor_visual();
	}
}

void ZPlayerStart3D::_ensure_editor_visual() {
	if (editor_visual != nullptr || !is_inside_tree()) {
		return;
	}
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Node3D *root = memnew(Node3D);
	root->set_name("_ZPlayerStartGizmo");
	add_child(root);
	// Not saved with the scene.
	root->set_owner(nullptr);
	editor_visual = root;

	const Color body_col(0.25f, 0.85f, 0.45f, 0.55f);
	const Color arrow_col(0.95f, 0.9f, 0.2f, 0.95f);

	// Body cylinder (standing capsule-like).
	MeshInstance3D *body = memnew(MeshInstance3D);
	Ref<CylinderMesh> cyl;
	cyl.instantiate();
	cyl->set_top_radius(0.35f);
	cyl->set_bottom_radius(0.35f);
	cyl->set_height(1.6f);
	body->set_mesh(cyl);
	body->set_material_override(make_player_start_mat(body_col));
	body->set_position(Vector3(0, 0.8f, 0));
	root->add_child(body);

	// Forward arrow along -Z (Godot forward).
	MeshInstance3D *shaft = memnew(MeshInstance3D);
	Ref<CylinderMesh> shaft_mesh;
	shaft_mesh.instantiate();
	shaft_mesh->set_top_radius(0.05f);
	shaft_mesh->set_bottom_radius(0.05f);
	shaft_mesh->set_height(0.7f);
	shaft->set_mesh(shaft_mesh);
	shaft->set_material_override(make_player_start_mat(arrow_col));
	shaft->set_rotation_degrees(Vector3(90, 0, 0));
	shaft->set_position(Vector3(0, 0.9f, -0.55f));
	root->add_child(shaft);

	MeshInstance3D *head = memnew(MeshInstance3D);
	Ref<CylinderMesh> cone;
	cone.instantiate();
	cone->set_top_radius(0.0f);
	cone->set_bottom_radius(0.14f);
	cone->set_height(0.28f);
	head->set_mesh(cone);
	head->set_material_override(make_player_start_mat(arrow_col));
	head->set_rotation_degrees(Vector3(90, 0, 0));
	head->set_position(Vector3(0, 0.9f, -1.05f));
	root->add_child(head);

	Label3D *label = memnew(Label3D);
	label->set_text("Player Start");
	label->set_position(Vector3(0, 1.9f, 0));
	label->set_pixel_size(0.01f);
	label->set_modulate(Color(1, 1, 1, 0.95f));
	label->set_outline_modulate(Color(0, 0, 0, 0.8f));
	label->set_draw_flag(Label3D::FLAG_DISABLE_DEPTH_TEST, true);
	root->add_child(label);
}

void ZPlayerStart3D::_clear_editor_visual() {
	if (editor_visual != nullptr) {
		editor_visual->queue_free();
		editor_visual = nullptr;
	}
}

void ZPlayerStart3D::set_player_index(int p_index) {
	player_index = p_index;
}

int ZPlayerStart3D::get_player_index() const {
	return player_index;
}

void ZPlayerStart3D::set_linked_map(const Ref<PackedScene> &p_map) {
	linked_map = p_map;
}

Ref<PackedScene> ZPlayerStart3D::get_linked_map() const {
	return linked_map;
}

void ZPlayerStart3D::set_link_id(const StringName &p_id) {
	link_id = p_id;
}

StringName ZPlayerStart3D::get_link_id() const {
	return link_id;
}
