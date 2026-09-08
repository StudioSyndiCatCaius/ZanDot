#pragma once

#include "z_menu.h"

#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

// Busy/progress menu. Opened with `ZEvent.Process(text, update)`; every frame it
// asks get_process_percent() for a 0–1 value, drives process_progress_bar with
// it, and closes itself once the value reaches 1.
//
// The percentage comes from the `update` Callable by default; GDScript
// subclasses can override `_get_process_percent()` instead.
class ZMenu_Process : public ZMenu {
	GDCLASS(ZMenu_Process, ZMenu)

private:
	RichTextLabel *lbl_text = nullptr;
	ProgressBar *process_progress_bar = nullptr;
	Callable update;
	String text;
	float percent = 0.0f;
	bool finished = false;

	void _apply_text();
	void _apply_percent(float p_percent);

protected:
	static void _bind_methods();
	void _notification(int p_what);

	GDVIRTUAL0RC(float, _get_process_percent)

public:
	void _process(double p_delta) override;

	void set_lbl_text(RichTextLabel *p_label);
	RichTextLabel *get_lbl_text() const;

	void set_process_progress_bar(ProgressBar *p_bar);
	ProgressBar *get_process_progress_bar() const;

	void set_text(const String &p_text);
	String get_text() const;

	void set_update(const Callable &p_update);
	Callable get_update() const;

	// 0–1 completion. Overridable from GDScript as `_get_process_percent()`.
	float get_process_percent();

	// Last percentage read this frame.
	float get_percent() const;

	// Fallback UI used when no scene is set for this menu.
	static ZMenu_Process *create_default();

	ZMenu_Process();
	~ZMenu_Process();
};

} // namespace godot
