#pragma once

namespace godot {

// Default InputMap actions for ZInputCommon.
//
// Every name is prefixed `z_` so it can never collide with a project's own
// bindings, and nothing is touched when the action already exists — a project
// that defines `z_move_up` itself keeps its own events.
//
// Actions are added to the live InputMap (so they work in an exported build
// with no project.godot entry) and, in the editor, mirrored into ProjectSettings
// so they show up under Project Settings → Input Map and can be re-bound there.
namespace ZInputDefaults {

// Registers any missing `z_*` action. Called once from initialize_zandot_module.
void register_all();

} // namespace ZInputDefaults

} // namespace godot
