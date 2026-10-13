#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/resource_uid.hpp"
#include "godot_cpp/classes/resource_loader.hpp"

#include <dr_flac.h>

#include "resource_format_loader_flac.h"
#include "audio_stream_flac.h"

using namespace godot;

namespace flac {

PackedStringArray ResourceFormatLoaderFLAC::_get_recognized_extensions() const {
	return { "flacstr" };
}

bool ResourceFormatLoaderFLAC::_handles_type(const StringName &p_type) const {
	return p_type == SNAME_AudioStreamFLAC || ClassDB::is_parent_class(p_type, SNAME_AudioStreamFLAC);
}

String ResourceFormatLoaderFLAC::_get_resource_type(const String &p_path) const {
	String extension = p_path.get_extension().to_lower();
	if (extension == "flacstr") {
		return "AudioStreamFLAC";
	}
	return "";
}

Variant ResourceFormatLoaderFLAC::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
	// Unfortunately, ResourceFormatLoaderBinaryInstance::load is not exposed, so we load manually

	static const uint32_t flacstr_version = 1;

	Ref<FileAccess> load_file_access = FileAccess::open(p_path, FileAccess::READ);
	if (load_file_access.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	uint32_t version = load_file_access->get_32();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	if (version != flacstr_version) {
		return ERR_FILE_UNRECOGNIZED;
	}
	bool loop = load_file_access->get_8() != 0;
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	double loop_offset = load_file_access->get_double();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	double bpm = load_file_access->get_double();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	int beat_count = (int)load_file_access->get_64();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	int bar_beats = (int)load_file_access->get_64();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	uint64_t data_size = load_file_access->get_64();
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}
	if (data_size > (load_file_access->get_length() - load_file_access->get_position())) {
		return ERR_FILE_CORRUPT;
	}
	PackedByteArray data = load_file_access->get_buffer(data_size);
	if (load_file_access->get_error()) {
		return ERR_FILE_CANT_READ;
	}

	Ref<AudioStreamFLAC> flac_stream = AudioStreamFLAC::load_from_buffer(data);
	if (flac_stream.is_null()) {
		return ERR_FILE_CANT_READ;
	}

	flac_stream->set_loop(loop);
	flac_stream->set_loop_offset(loop_offset);
	flac_stream->set_bpm(bpm);
	flac_stream->set_beat_count(beat_count);
	flac_stream->set_bar_beats(bar_beats);

	return flac_stream;
}

}