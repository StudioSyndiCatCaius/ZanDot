#include "z_menu_default.h"

#include "z_menu.h"

#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/panel_container.hpp>

using namespace godot;

CenterContainer *ZMenuDefault::build_backdrop(ZMenu *p_menu) {
	Control *root = memnew(Control);
	root->set_name("Root");
	root->set_anchors_preset(Control::PRESET_FULL_RECT);
	// Swallow clicks so gameplay UI underneath cannot be poked at.
	root->set_mouse_filter(Control::MOUSE_FILTER_STOP);
	p_menu->add_child(root);

	ColorRect *dim = memnew(ColorRect);
	dim->set_name("Dim");
	dim->set_anchors_preset(Control::PRESET_FULL_RECT);
	dim->set_color(Color(0, 0, 0, 0.6f));
	dim->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	root->add_child(dim);

	CenterContainer *center = memnew(CenterContainer);
	center->set_name("Center");
	center->set_anchors_preset(Control::PRESET_FULL_RECT);
	center->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	root->add_child(center);

	return center;
}

VBoxContainer *ZMenuDefault::build_panel(CenterContainer *p_center, const Vector2 &p_min_size) {
	PanelContainer *panel = memnew(PanelContainer);
	panel->set_name("Panel");
	panel->set_custom_minimum_size(p_min_size);
	p_center->add_child(panel);

	MarginContainer *margins = memnew(MarginContainer);
	margins->set_name("Margins");
	margins->add_theme_constant_override("margin_left", 16);
	margins->add_theme_constant_override("margin_right", 16);
	margins->add_theme_constant_override("margin_top", 12);
	margins->add_theme_constant_override("margin_bottom", 12);
	panel->add_child(margins);

	VBoxContainer *content = memnew(VBoxContainer);
	content->set_name("Content");
	content->add_theme_constant_override("separation", 12);
	margins->add_child(content);

	return content;
}

RichTextLabel *ZMenuDefault::build_label(Control *p_parent, const Vector2 &p_min_size) {
	RichTextLabel *label = memnew(RichTextLabel);
	label->set_name("Text");
	label->set_use_bbcode(true);
	label->set_fit_content(true);
	label->set_custom_minimum_size(p_min_size);
	label->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	p_parent->add_child(label);
	return label;
}

Button *ZMenuDefault::build_button(Control *p_parent, const String &p_text) {
	Button *button = memnew(Button);
	button->set_name(p_text.is_empty() ? String("Button") : p_text);
	button->set_text(p_text);
	button->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	p_parent->add_child(button);
	return button;
}
