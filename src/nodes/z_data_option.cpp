#include "z_data_option.h"
#include "z_data_list.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void ZDataOption::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_source_object", "object"), &ZDataOption::set_source_object);
	ClassDB::bind_method(D_METHOD("get_source_object"), &ZDataOption::get_source_object);

	ClassDB::bind_method(D_METHOD("set_select_button", "button"), &ZDataOption::set_select_button);
	ClassDB::bind_method(D_METHOD("get_select_button"), &ZDataOption::get_select_button);

	ClassDB::bind_method(D_METHOD("set_label_title", "label"), &ZDataOption::set_label_title);
	ClassDB::bind_method(D_METHOD("get_label_title"), &ZDataOption::get_label_title);

	ClassDB::bind_method(D_METHOD("set_img_icon", "icon"), &ZDataOption::set_img_icon);
	ClassDB::bind_method(D_METHOD("get_img_icon"), &ZDataOption::get_img_icon);

	ClassDB::bind_method(D_METHOD("set_label_description", "label"), &ZDataOption::set_label_description);
	ClassDB::bind_method(D_METHOD("get_label_description"), &ZDataOption::get_label_description);

	ClassDB::bind_method(D_METHOD("set_use_button_icon", "enabled"), &ZDataOption::set_use_button_icon);
	ClassDB::bind_method(D_METHOD("get_use_button_icon"), &ZDataOption::get_use_button_icon);

	ClassDB::bind_method(D_METHOD("set_use_button_text", "enabled"), &ZDataOption::set_use_button_text);
	ClassDB::bind_method(D_METHOD("get_use_button_text"), &ZDataOption::get_use_button_text);

	ClassDB::bind_method(D_METHOD("set_override_title", "title"), &ZDataOption::set_override_title);
	ClassDB::bind_method(D_METHOD("get_override_title"), &ZDataOption::get_override_title);

	ClassDB::bind_method(D_METHOD("set_override_icon", "icon"), &ZDataOption::set_override_icon);
	ClassDB::bind_method(D_METHOD("get_override_icon"), &ZDataOption::get_override_icon);

	ClassDB::bind_method(D_METHOD("set_is_highlighted", "highlighted"), &ZDataOption::set_is_highlighted);
	ClassDB::bind_method(D_METHOD("get_is_highlighted"), &ZDataOption::get_is_highlighted);

	ClassDB::bind_method(D_METHOD("get_is_hovered"), &ZDataOption::get_is_hovered);
	ClassDB::bind_method(D_METHOD("get_is_selectable"), &ZDataOption::get_is_selectable);
	ClassDB::bind_method(D_METHOD("set_option_visible", "visible"), &ZDataOption::set_option_visible);

	ClassDB::bind_method(D_METHOD("select"), &ZDataOption::select);
	ClassDB::bind_method(D_METHOD("get_list"), &ZDataOption::get_list);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "source_object"), "set_source_object", "get_source_object");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "select_button", PROPERTY_HINT_NODE_TYPE, "Button"), "set_select_button", "get_select_button");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "label_title", PROPERTY_HINT_NODE_TYPE, "Label"), "set_label_title", "get_label_title");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "img_icon", PROPERTY_HINT_NODE_TYPE, "TextureRect"), "set_img_icon", "get_img_icon");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "label_description", PROPERTY_HINT_NODE_TYPE, "RichTextLabel"), "set_label_description", "get_label_description");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_button_icon"), "set_use_button_icon", "get_use_button_icon");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_button_text"), "set_use_button_text", "get_use_button_text");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "override_title"), "set_override_title", "get_override_title");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "override_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_override_icon", "get_override_icon");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_highlighted"), "set_is_highlighted", "get_is_highlighted");

	ADD_SIGNAL(MethodInfo("on_selected"));
	ADD_SIGNAL(MethodInfo("on_hovered", PropertyInfo(Variant::BOOL, "is_hovered")));
	ADD_SIGNAL(MethodInfo("on_highlighted", PropertyInfo(Variant::BOOL, "is_highlighted")));
}

ZDataOption::ZDataOption() {
	set_mouse_filter(Control::MOUSE_FILTER_STOP);
}

ZDataOption::~ZDataOption() {
	_disconnect_select_button();
	_disconnect_self_input();
}

void ZDataOption::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_refresh_input_wiring();
			_apply_source_visuals();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_disconnect_select_button();
			_disconnect_self_input();
		} break;
		default:
			break;
	}
}

void ZDataOption::_disconnect_select_button() {
	if (select_button == nullptr || !button_connected) {
		return;
	}
	const Callable press_cb = callable_mp(this, &ZDataOption::_on_select_button_pressed);
	const Callable enter_cb = callable_mp(this, &ZDataOption::_on_mouse_entered);
	const Callable exit_cb = callable_mp(this, &ZDataOption::_on_mouse_exited);
	if (select_button->is_connected("pressed", press_cb)) {
		select_button->disconnect("pressed", press_cb);
	}
	if (select_button->is_connected("mouse_entered", enter_cb)) {
		select_button->disconnect("mouse_entered", enter_cb);
	}
	if (select_button->is_connected("mouse_exited", exit_cb)) {
		select_button->disconnect("mouse_exited", exit_cb);
	}
	button_connected = false;
}

void ZDataOption::_connect_select_button() {
	_disconnect_select_button();
	if (select_button == nullptr) {
		return;
	}
	const Callable press_cb = callable_mp(this, &ZDataOption::_on_select_button_pressed);
	const Callable enter_cb = callable_mp(this, &ZDataOption::_on_mouse_entered);
	const Callable exit_cb = callable_mp(this, &ZDataOption::_on_mouse_exited);
	if (!select_button->is_connected("pressed", press_cb)) {
		select_button->connect("pressed", press_cb);
	}
	if (!select_button->is_connected("mouse_entered", enter_cb)) {
		select_button->connect("mouse_entered", enter_cb);
	}
	if (!select_button->is_connected("mouse_exited", exit_cb)) {
		select_button->connect("mouse_exited", exit_cb);
	}
	button_connected = true;
}

void ZDataOption::_disconnect_self_input() {
	if (!self_input_connected) {
		return;
	}
	const Callable enter_cb = callable_mp(this, &ZDataOption::_on_mouse_entered);
	const Callable exit_cb = callable_mp(this, &ZDataOption::_on_mouse_exited);
	if (is_connected("mouse_entered", enter_cb)) {
		disconnect("mouse_entered", enter_cb);
	}
	if (is_connected("mouse_exited", exit_cb)) {
		disconnect("mouse_exited", exit_cb);
	}
	self_input_connected = false;
}

void ZDataOption::_connect_self_input() {
	_disconnect_self_input();
	const Callable enter_cb = callable_mp(this, &ZDataOption::_on_mouse_entered);
	const Callable exit_cb = callable_mp(this, &ZDataOption::_on_mouse_exited);
	if (!is_connected("mouse_entered", enter_cb)) {
		connect("mouse_entered", enter_cb);
	}
	if (!is_connected("mouse_exited", exit_cb)) {
		connect("mouse_exited", exit_cb);
	}
	self_input_connected = true;
}

void ZDataOption::_refresh_input_wiring() {
	if (select_button != nullptr) {
		_disconnect_self_input();
		_connect_select_button();
	} else {
		_disconnect_select_button();
		_connect_self_input();
	}
}

void ZDataOption::_on_select_button_pressed() {
	select();
}

void ZDataOption::_on_mouse_entered() {
	set_hovered(true);
}

void ZDataOption::_on_mouse_exited() {
	set_hovered(false);
}

void ZDataOption::_gui_input(const Ref<InputEvent> &p_event) {
	// When no select_button is wired, the row Control itself is clickable.
	if (select_button != nullptr) {
		return;
	}
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MOUSE_BUTTON_LEFT) {
		select();
		accept_event();
	}
}

Ref<ZGeneralData> ZDataOption::_fetch_z_data() const {
	Object *src = get_source_object();
	if (src == nullptr || !src->has_method("get_z_data")) {
		return Ref<ZGeneralData>();
	}
	Variant result = src->call("get_z_data");
	if (result.get_type() != Variant::OBJECT) {
		return Ref<ZGeneralData>();
	}
	return Ref<ZGeneralData>(Object::cast_to<ZGeneralData>(result.operator Object *()));
}

void ZDataOption::_apply_color(const Color &p_color) {
	set_modulate(p_color);
	if (select_button != nullptr) {
		select_button->set_modulate(p_color);
	}
	if (label_title != nullptr) {
		label_title->set_modulate(p_color);
	}
	if (img_icon != nullptr) {
		img_icon->set_modulate(p_color);
	}
	if (label_description != nullptr) {
		label_description->set_modulate(p_color);
	}
}

void ZDataOption::_apply_source_visuals() {
	Ref<ZGeneralData> data = _fetch_z_data();

	String title = override_title;
	Ref<Texture2D> icon = override_icon;
	String description;
	Color color(1, 1, 1, 1);

	if (data.is_valid()) {
		if (title.is_empty()) {
			title = data->get_title();
		}
		if (icon.is_null()) {
			icon = data->get_icon();
		}
		description = data->get_description();
		color = data->get_color();
	}

	if (label_title != nullptr) {
		label_title->set_text(title);
	}
	if (label_description != nullptr) {
		label_description->set_text(description);
	}

	if (select_button != nullptr) {
		if (use_button_text) {
			select_button->set_text(title);
		}
		if (use_button_icon) {
			select_button->set_button_icon(icon);
		}
	}
	if (!use_button_icon && img_icon != nullptr) {
		img_icon->set_texture(icon);
	}

	_apply_color(color);
}

void ZDataOption::set_source_object(Object *p_object) {
	if (p_object == nullptr) {
		source_object_id = ObjectID();
	} else {
		source_object_id = ObjectID(p_object->get_instance_id());
	}
	_apply_source_visuals();

	Object *src = get_source_object();
	if (has_method("on_source_object_changed")) {
		call("on_source_object_changed", src);
	} else {
		on_source_object_changed(src);
	}
}

Object *ZDataOption::get_source_object() const {
	if (source_object_id.is_null()) {
		return nullptr;
	}
	return ObjectDB::get_instance(source_object_id);
}

void ZDataOption::set_select_button(Button *p_button) {
	if (select_button == p_button) {
		return;
	}
	_disconnect_select_button();
	select_button = p_button;
	if (is_inside_tree()) {
		_refresh_input_wiring();
	}
	_apply_source_visuals();
}

Button *ZDataOption::get_select_button() const {
	return select_button;
}

void ZDataOption::set_label_title(Label *p_label) {
	label_title = p_label;
	_apply_source_visuals();
}

Label *ZDataOption::get_label_title() const {
	return label_title;
}

void ZDataOption::set_img_icon(TextureRect *p_icon) {
	img_icon = p_icon;
	_apply_source_visuals();
}

TextureRect *ZDataOption::get_img_icon() const {
	return img_icon;
}

void ZDataOption::set_label_description(RichTextLabel *p_label) {
	label_description = p_label;
	_apply_source_visuals();
}

RichTextLabel *ZDataOption::get_label_description() const {
	return label_description;
}

void ZDataOption::set_use_button_icon(bool p_enabled) {
	if (use_button_icon == p_enabled) {
		return;
	}
	use_button_icon = p_enabled;
	_apply_source_visuals();
}

bool ZDataOption::get_use_button_icon() const {
	return use_button_icon;
}

void ZDataOption::set_use_button_text(bool p_enabled) {
	if (use_button_text == p_enabled) {
		return;
	}
	use_button_text = p_enabled;
	_apply_source_visuals();
}

bool ZDataOption::get_use_button_text() const {
	return use_button_text;
}

void ZDataOption::set_override_title(const String &p_title) {
	if (override_title == p_title) {
		return;
	}
	override_title = p_title;
	_apply_source_visuals();
}

String ZDataOption::get_override_title() const {
	return override_title;
}

void ZDataOption::set_override_icon(const Ref<Texture2D> &p_icon) {
	if (override_icon == p_icon) {
		return;
	}
	override_icon = p_icon;
	_apply_source_visuals();
}

Ref<Texture2D> ZDataOption::get_override_icon() const {
	return override_icon;
}

void ZDataOption::set_is_highlighted(bool p_highlighted) {
	if (is_highlighted == p_highlighted) {
		return;
	}
	is_highlighted = p_highlighted;
	emit_signal("on_highlighted", is_highlighted);
}

bool ZDataOption::get_is_highlighted() const {
	return is_highlighted;
}

bool ZDataOption::get_is_hovered() const {
	return is_hovered;
}

void ZDataOption::set_hovered(bool p_hovered) {
	if (is_hovered == p_hovered) {
		return;
	}
	is_hovered = p_hovered;
	emit_signal("on_hovered", is_hovered);

	ZDataList *list = get_list();
	if (list != nullptr && is_hovered) {
		const int idx = list->find_option_index(this);
		if (idx >= 0) {
			list->emit_signal("on_option_hovered", this, idx);
		}
	}
}

void ZDataOption::set_option_visible(bool p_visible) {
	set_visible(p_visible);
}

void ZDataOption::select() {
	ZDataList *list = get_list();
	if (list != nullptr) {
		list->select_option_node(this);
	} else {
		emit_signal("on_selected");
	}
}

bool ZDataOption::get_is_selectable() const {
	if (const_cast<ZDataOption *>(this)->has_method("_get_is_selectable")) {
		return (bool)const_cast<ZDataOption *>(this)->call("_get_is_selectable");
	}
	return on_get_is_selectable();
}

ZDataList *ZDataOption::get_list() const {
	if (list_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<ZDataList>(ObjectDB::get_instance(list_id));
}

void ZDataOption::_bind_to_list(ZDataList *p_list) {
	if (p_list == nullptr) {
		list_id = ObjectID();
	} else {
		list_id = ObjectID(p_list->get_instance_id());
	}
}

void ZDataOption::_notify_added_to_list(ZDataList *p_list, int p_index, Object *p_src_object) {
	_bind_to_list(p_list);
	if (has_method("on_added_to_list")) {
		call("on_added_to_list", p_list, p_index, p_src_object);
	} else {
		on_added_to_list(p_list, p_index, p_src_object);
	}
}

void ZDataOption::_notify_list_owner_changed(ZDataList *p_list, Object *p_new_owner) {
	if (has_method("on_list_owner_changed")) {
		call("on_list_owner_changed", p_list, p_new_owner);
	} else {
		on_list_owner_changed(p_list, p_new_owner);
	}
}

void ZDataOption::_notify_source_applied() {
	_apply_source_visuals();
}

void ZDataOption::on_added_to_list(ZDataList * /*p_list*/, int /*p_index*/, Object * /*p_src_object*/) {
}

void ZDataOption::on_source_object_changed(Object * /*p_new_object*/) {
}

void ZDataOption::on_list_owner_changed(ZDataList * /*p_list*/, Object * /*p_new_owner*/) {
}

bool ZDataOption::on_get_is_selectable() const {
	return true;
}
