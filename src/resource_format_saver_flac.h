#pragma once

#include <godot_cpp/classes/resource_format_saver.hpp>

using namespace godot;

namespace flac {

class ResourceFormatSaverFLAC : public ResourceFormatSaver {
	GDCLASS(ResourceFormatSaverFLAC, ResourceFormatSaver);

protected:
	static void _bind_methods() {}

public:
	PackedStringArray _get_recognized_extensions(const Ref<Resource> &p_resource) const override;
	bool _recognize(const Ref<Resource> &p_resource) const override;
	Error _save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) override;

	ResourceFormatSaverFLAC() {}
};

}