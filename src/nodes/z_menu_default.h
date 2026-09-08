#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/center_container.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {

class ZMenu;

// Node-building helpers for the fallback menus ZEvent spins up when no scene is
// configured under Project Settings → ZanDot → Menus. They are deliberately
// plain: enough to be usable and readable, not a look any game should ship.
namespace ZMenuDefault {

// Full-rect dimmed backdrop under p_menu; returns the CenterContainer to fill.
CenterContainer *build_backdrop(ZMenu *p_menu);

// Panel + margins centred in p_center; returns the VBox content column.
VBoxContainer *build_panel(CenterContainer *p_center, const Vector2 &p_min_size);

RichTextLabel *build_label(Control *p_parent, const Vector2 &p_min_size);

Button *build_button(Control *p_parent, const String &p_text);

} // namespace ZMenuDefault

} // namespace godot
