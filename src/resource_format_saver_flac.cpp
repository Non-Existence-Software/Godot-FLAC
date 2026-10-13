#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include <dr_flac.h>

#include "audio_stream_flac.h"
#include "resource_format_saver_flac.h"

using namespace godot;

namespace flac {

PackedStringArray ResourceFormatSaverFLAC::_get_recognized_extensions(const Ref<Resource> &p_resource) const {
	return { "flacstr" };
}

bool ResourceFormatSaverFLAC::_recognize(const Ref<Resource> &p_resource) const {
	return p_resource.is_valid() && p_resource->is_class("AudioStreamFLAC");
}

Error ResourceFormatSaverFLAC::_save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	// Unfortunately, ResourceFormatSaverBinaryInstance::save is not exposed, so we save manually

	static const uint32_t flacstr_version = 1;

	Ref<AudioStreamFLAC> flac_stream = p_resource;
	if (flac_stream.is_null()) {
		return ERR_INVALID_DATA;
	}

	Ref<FileAccess> save_file_access = FileAccess::open(p_path, FileAccess::WRITE);
	if (save_file_access.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	if (!save_file_access->store_32(flacstr_version)) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_8(flac_stream->has_loop() ? 1 : 0)) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_double(flac_stream->get_loop_offset())) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_double(flac_stream->_get_bpm())) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_64((uint64_t)flac_stream->_get_beat_count())) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_64((uint64_t)flac_stream->get_bar_beats())) {
		return ERR_FILE_CANT_WRITE;
	}
	PackedByteArray flac_stream_data = flac_stream->get_data();
	if (!save_file_access->store_64((uint64_t)flac_stream_data.size())) {
		return ERR_FILE_CANT_WRITE;
	}
	if (!save_file_access->store_buffer(flac_stream_data)) {
		return ERR_FILE_CANT_WRITE;
	}
	save_file_access->close();
	return OK;
}

}