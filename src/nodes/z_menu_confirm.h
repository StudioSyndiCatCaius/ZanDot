#pragma once

#include "z_menu.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// Yes/no prompt. Opened with `ZEvent.Confirm(...)`; emits on_confirm(confirmed)
// and then closes, so the awaited signal resolves after the close animation.
//
// `action_confirm` answers yes, `action_close` answers no; the optional
// btn_yes / btn_no buttons do the same on press.
class ZMenu_Confirm : public ZMenu {
	GDCLASS(ZMenu_Confirm, ZMenu)

private:
	RichTextLabel *lbl_text = nullptr;
	Button *btn_yes = nullptr;
	Button *btn_no = nullptr;

	String text;
	String yes_text = "Yes";
	String no_text = "No";

	bool answered = false;

	void _apply_text();
	void _apply_button_text();
	void _connect_buttons();
	void _on_yes_pressed();
	void _on_no_pressed();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Public so godot-cpp can bind the Node virtual.
	void _unhandled_input(const Ref<InputEvent> &p_event) override;

	void set_lbl_text(RichTextLabel *p_label);
	RichTextLabel *get_lbl_text() const;

	void set_btn_yes(Button *p_button);
	Button *get_btn_yes() const;

	void set_btn_no(Button *p_button);
	Button *get_btn_no() const;

	void set_text(const String &p_text);
	String get_text() const;

	void set_yes_text(const String &p_text);
	String get_yes_text() const;

	void set_no_text(const String &p_text);
	String get_no_text() const;

	// Answers the prompt: emits on_confirm then closes. Only the first call counts.
	void confirm(bool p_confirmed);

	// Fallback UI used when no scene is set for this menu.
	static ZMenu_Confirm *create_default();

	ZMenu_Confirm();
	~ZMenu_Confirm();
};

} // namespace godot
