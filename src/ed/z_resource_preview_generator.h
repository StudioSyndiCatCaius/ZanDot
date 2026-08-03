#pragma once

#ifdef DEBUG_ENABLED

#include <godot_cpp/classes/editor_resource_preview_generator.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2i.hpp>

namespace godot {

// FileSystem / file-dialog thumbnails for ZResource (and subclasses) using the `icon` property.
class ZResourcePreviewGenerator : public EditorResourcePreviewGenerator {
	GDCLASS(ZResourcePreviewGenerator, EditorResourcePreviewGenerator)

protected:
	static void _bind_methods() {}

public:
	virtual bool _handles(const String &p_type) const override;
	virtual Ref<Texture2D> _generate(const Ref<Resource> &p_resource, const Vector2i &p_size, const Dictionary &p_metadata) const override;
	virtual bool _generate_small_preview_automatically() const override;
	virtual bool _can_generate_small_preview() const override;

	ZResourcePreviewGenerator() = default;
};

} // namespace godot

#endif // DEBUG_ENABLED
