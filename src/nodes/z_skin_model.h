#pragma once

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

// Manager for treating an imported model scene (fbx/glb/blend, …) as a swappable
// model or named model slot, while reusing a shared AnimationTree.
class ZSkinModel : public Node {
	GDCLASS(ZSkinModel, Node)

private:
	Node3D *model_default = nullptr;
	// Dictionary[StringName, Node3D] — named modular slots (head, body, weapon, …).
	Dictionary model_slots;
	AnimationTree *anim_tree = nullptr;

	static Dictionary _as_model_slots_dict(const Dictionary &p_from);
	static AnimationPlayer *_find_animation_player(Node *p_root);
	Node3D *_replace_model_node(Node3D *p_old, Node3D *p_new);

protected:
	static void _bind_methods();

public:
	void set_model_default(Node3D *p_model);
	Node3D *get_model_default() const;

	void set_model_slots(const Dictionary &p_slots);
	Dictionary get_model_slots() const;

	void set_anim_tree(AnimationTree *p_anim_tree);
	AnimationTree *get_anim_tree() const;

	// Instantiate p_scene and install it as the default model, or into a named
	// slot when p_slot is a known key. Empty / unknown slot → default.
	// Returns the new Node3D root, or null on failure.
	Node3D *set_model_from_scene(const Ref<PackedScene> &p_scene, const StringName &p_slot = StringName());

	// Rewire anim_tree's AnimationPlayer path to the AnimationPlayer found under
	// the active default model (falls back to first slot that has one).
	void update_animation();

	// Convenience getters for slots.
	Node3D *get_model_slot(const StringName &p_slot) const;
	bool has_model_slot(const StringName &p_slot) const;

	ZSkinModel();
	~ZSkinModel();
};

} // namespace godot
