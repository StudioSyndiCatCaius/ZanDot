#include "z_skin_model.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

// Inspector hint for Dictionary[StringName, Node3D].
String model_slots_hint() {
	return String::num_int64(Variant::STRING_NAME) + "/" + String::num_int64(PROPERTY_HINT_NONE) + ":;" +
			String::num_int64(Variant::OBJECT) + "/" + String::num_int64(PROPERTY_HINT_NODE_TYPE) + ":Node3D";
}

} // namespace

Dictionary ZSkinModel::_as_model_slots_dict(const Dictionary &p_from) {
	return Dictionary(p_from, Variant::STRING_NAME, StringName(), Variant(), Variant::OBJECT, StringName("Node3D"), Variant());
}

AnimationPlayer *ZSkinModel::_find_animation_player(Node *p_root) {
	if (p_root == nullptr) {
		return nullptr;
	}
	if (AnimationPlayer *self = Object::cast_to<AnimationPlayer>(p_root)) {
		return self;
	}
	// owned=false so instances from PackedScene (not owned by the editor scene) still match.
	TypedArray<Node> found = p_root->find_children("*", "AnimationPlayer", true, false);
	if (found.size() > 0) {
		return Object::cast_to<AnimationPlayer>(found[0]);
	}
	return nullptr;
}

void ZSkinModel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_model_default", "model"), &ZSkinModel::set_model_default);
	ClassDB::bind_method(D_METHOD("get_model_default"), &ZSkinModel::get_model_default);

	ClassDB::bind_method(D_METHOD("set_model_slots", "slots"), &ZSkinModel::set_model_slots);
	ClassDB::bind_method(D_METHOD("get_model_slots"), &ZSkinModel::get_model_slots);

	ClassDB::bind_method(D_METHOD("set_anim_tree", "anim_tree"), &ZSkinModel::set_anim_tree);
	ClassDB::bind_method(D_METHOD("get_anim_tree"), &ZSkinModel::get_anim_tree);

	ClassDB::bind_method(D_METHOD("set_model_from_scene", "scene", "slot"), &ZSkinModel::set_model_from_scene, DEFVAL(StringName()));
	ClassDB::bind_method(D_METHOD("Set_Model_From_Scene", "scene", "slot"), &ZSkinModel::set_model_from_scene, DEFVAL(StringName()));

	ClassDB::bind_method(D_METHOD("update_animation"), &ZSkinModel::update_animation);
	ClassDB::bind_method(D_METHOD("Update_Animation"), &ZSkinModel::update_animation);

	ClassDB::bind_method(D_METHOD("get_model_slot", "slot"), &ZSkinModel::get_model_slot);
	ClassDB::bind_method(D_METHOD("has_model_slot", "slot"), &ZSkinModel::has_model_slot);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model_default", PROPERTY_HINT_NODE_TYPE, "Node3D"), "set_model_default", "get_model_default");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "model_slots", PROPERTY_HINT_DICTIONARY_TYPE, model_slots_hint()), "set_model_slots", "get_model_slots");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "anim_tree", PROPERTY_HINT_NODE_TYPE, "AnimationTree"), "set_anim_tree", "get_anim_tree");
}

ZSkinModel::ZSkinModel() {
	model_slots = _as_model_slots_dict(Dictionary());
}

ZSkinModel::~ZSkinModel() {
}

void ZSkinModel::set_model_default(Node3D *p_model) {
	model_default = p_model;
}

Node3D *ZSkinModel::get_model_default() const {
	return model_default;
}

void ZSkinModel::set_model_slots(const Dictionary &p_slots) {
	model_slots = _as_model_slots_dict(p_slots);
}

Dictionary ZSkinModel::get_model_slots() const {
	return model_slots;
}

void ZSkinModel::set_anim_tree(AnimationTree *p_anim_tree) {
	anim_tree = p_anim_tree;
}

AnimationTree *ZSkinModel::get_anim_tree() const {
	return anim_tree;
}

Node3D *ZSkinModel::get_model_slot(const StringName &p_slot) const {
	if (p_slot.is_empty() || !model_slots.has(p_slot)) {
		return nullptr;
	}
	return Object::cast_to<Node3D>(model_slots[p_slot]);
}

bool ZSkinModel::has_model_slot(const StringName &p_slot) const {
	return !p_slot.is_empty() && model_slots.has(p_slot);
}

Node3D *ZSkinModel::_replace_model_node(Node3D *p_old, Node3D *p_new) {
	if (p_new == nullptr) {
		return nullptr;
	}

	Node *parent = this;
	int index = -1;
	Transform3D xform;
	bool has_xform = false;
	Node *scene_owner = nullptr;

	if (p_old != nullptr && p_old != p_new) {
		if (p_old->get_parent() != nullptr) {
			parent = p_old->get_parent();
			index = p_old->get_index();
		}
		xform = p_old->get_transform();
		has_xform = true;
		scene_owner = p_old->get_owner();

		parent->remove_child(p_old);
		p_old->queue_free();
	}

	if (p_new->get_parent() != nullptr) {
		p_new->get_parent()->remove_child(p_new);
	}

	parent->add_child(p_new);
	if (index >= 0 && index < parent->get_child_count()) {
		parent->move_child(p_new, index);
	}
	if (has_xform) {
		p_new->set_transform(xform);
	}
	if (scene_owner != nullptr) {
		p_new->set_owner(scene_owner);
	}

	return p_new;
}

Node3D *ZSkinModel::set_model_from_scene(const Ref<PackedScene> &p_scene, const StringName &p_slot) {
	if (p_scene.is_null()) {
		UtilityFunctions::push_warning("ZSkinModel.set_model_from_scene: scene is null.");
		return nullptr;
	}

	Node *inst = p_scene->instantiate();
	if (inst == nullptr) {
		UtilityFunctions::push_warning("ZSkinModel.set_model_from_scene: failed to instantiate scene.");
		return nullptr;
	}

	Node3D *model = Object::cast_to<Node3D>(inst);
	if (model == nullptr) {
		UtilityFunctions::push_warning("ZSkinModel.set_model_from_scene: scene root is not a Node3D.");
		inst->queue_free();
		return nullptr;
	}

	const bool use_slot = !p_slot.is_empty() && model_slots.has(p_slot);
	if (use_slot) {
		Node3D *old = Object::cast_to<Node3D>(model_slots[p_slot]);
		_replace_model_node(old, model);
		model_slots[p_slot] = model;
	} else {
		_replace_model_node(model_default, model);
		model_default = model;
	}

	update_animation();
	return model;
}

void ZSkinModel::update_animation() {
	if (anim_tree == nullptr) {
		return;
	}

	AnimationPlayer *player = _find_animation_player(model_default);

	// Fall back: first slot that contains an AnimationPlayer.
	if (player == nullptr) {
		Array keys = model_slots.keys();
		for (int i = 0; i < keys.size(); i++) {
			Node3D *slot_model = Object::cast_to<Node3D>(model_slots[keys[i]]);
			player = _find_animation_player(slot_model);
			if (player != nullptr) {
				break;
			}
		}
	}

	if (player == nullptr) {
		return;
	}

	// AnimationTree and player must share a tree for NodePath resolution.
	if (!anim_tree->is_inside_tree() || !player->is_inside_tree()) {
		return;
	}
	if (anim_tree->get_tree() != player->get_tree()) {
		UtilityFunctions::push_warning("ZSkinModel.update_animation: anim_tree and AnimationPlayer are in different SceneTrees.");
		return;
	}

	const NodePath path = anim_tree->get_path_to(player);
	anim_tree->set_animation_player(path);
}
