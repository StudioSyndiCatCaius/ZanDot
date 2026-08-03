#ifdef DEBUG_ENABLED

#include "z_resource_preview_generator.h"

#include "z_resource.h"

#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

bool ZResourcePreviewGenerator::_handles(const String &p_type) const {
	if (p_type.is_empty()) {
		return false;
	}
	// Native class and any ClassDB subclass of ZResource.
	// Guard ClassDB lookups — unknown/script-only type names can be noisy mid-load.
	if (p_type == String("ZResource")) {
		return true;
	}
	if (ClassDB::class_exists(p_type) && ClassDB::is_parent_class(p_type, StringName("ZResource"))) {
		return true;
	}
	return false;
}

Ref<Texture2D> ZResourcePreviewGenerator::_generate(const Ref<Resource> &p_resource, const Vector2i &p_size, const Dictionary & /*p_metadata*/) const {
	if (p_resource.is_null()) {
		return Ref<Texture2D>();
	}

	// Scripted ZResource assets still report get_class() as ZResource.
	ZResource *zres = Object::cast_to<ZResource>(p_resource.ptr());
	if (!zres) {
		return Ref<Texture2D>();
	}

	Ref<Texture2D> icon = zres->get_icon();
	if (icon.is_null()) {
		return Ref<Texture2D>();
	}

	// Called from a worker thread — only use already-decoded image data.
	// Compressed/streamed textures may return null here; fall back to the icon as-is.
	Ref<Image> img = icon->get_image();
	if (img.is_null() || img->is_empty()) {
		return icon;
	}

	img = img->duplicate();
	const int tw = p_size.x > 0 ? p_size.x : img->get_width();
	const int th = p_size.y > 0 ? p_size.y : img->get_height();
	if (img->get_width() != tw || img->get_height() != th) {
		img->resize(tw, th, Image::INTERPOLATE_LANCZOS);
	}

	return ImageTexture::create_from_image(img);
}

bool ZResourcePreviewGenerator::_generate_small_preview_automatically() const {
	return true;
}

bool ZResourcePreviewGenerator::_can_generate_small_preview() const {
	return true;
}

#endif // DEBUG_ENABLED
