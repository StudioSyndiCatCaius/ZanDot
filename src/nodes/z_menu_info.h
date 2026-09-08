#pragma once

#include "z_menu.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// Read-and-dismiss message. Opened with `ZEvent.Info(text)`; either
// `action_confirm` or `action_close` (or btn_close) shuts it.
class ZMenu_Info : public ZMenu {
	GDCLASS(ZMenu_Info, ZMenu)

private:
	RichTextLabel *lbl_text = nullptr;
	Button *btn_close = nullptr;
	String text;

	void _apply_text();
	void _connect_button();
	void _on_close_pressed();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Public so godot-cpp can bind the Node virtual.
	void _unhandled_input(const Ref<InputEvent> &p_event) override;

	void set_lbl_text(RichTextLabel *p_label);
	RichTextLabel *get_lbl_text() const;

	void set_btn_close(Button *p_button);
	Button *get_btn_close() const;

	void set_text(const String &p_text);
	String get_text() const;

	// Fallback UI used when no scene is set for this menu.
	static ZMenu_Info *create_default();

	ZMenu_Info();
	~ZMenu_Info();
};

} // namespace godot
