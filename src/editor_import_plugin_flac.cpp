/**************************************************************************/
/*  resource_importer_flac.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/


#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include "editor_import_plugin_flac.h"
#include "audio_stream_flac.h"

String EditorImportPluginFLAC::_get_importer_name() const {
	return "FLACImporter";
}

String EditorImportPluginFLAC::_get_visible_name() const {
	return "FLAC";
}

PackedStringArray EditorImportPluginFLAC::_get_recognized_extensions() const {
	return { "flac" };
}

String EditorImportPluginFLAC::_get_save_extension() const {
	return "flacstr";
}

String EditorImportPluginFLAC::_get_resource_type() const {
	return "AudioStreamFLAC";
}

int32_t EditorImportPluginFLAC::_get_preset_count() const {
	return 0;
}

String EditorImportPluginFLAC::_get_preset_name(int32_t p_idx) const {
	return "";
}

TypedArray<Dictionary> EditorImportPluginFLAC::_get_import_options(const String &p_path, int32_t p_preset) const {
	TypedArray<Dictionary> options;

	Dictionary loop;
	loop["name"] = "loop";
	loop["default_value"] = false;
	options.push_back(loop);

	Dictionary loop_offset;
	loop_offset["name"] = "loop_offset";
	loop_offset["default_value"] = 0.0;
	options.push_back(loop_offset);

	Dictionary bpm;
	bpm["name"] = "bpm";
	bpm["default_value"] = 0.0;
	bpm["property_hint"] = PROPERTY_HINT_RANGE;
	bpm["hint_string"] = "0,400,0.01,or_greater";
	options.push_back(bpm);

	Dictionary beat_count;
	beat_count["name"] = "beat_count";
	beat_count["default_value"] = 0;
	beat_count["property_hint"] = PROPERTY_HINT_RANGE;
	beat_count["hint_string"] = "0,512,or_greater";
	options.push_back(beat_count);

	Dictionary bar_beats;
	bar_beats["name"] = "bar_beats";
	bar_beats["default_value"] = 4;
	bar_beats["property_hint"] = PROPERTY_HINT_RANGE;
	bar_beats["hint_string"] = "2,32,or_greater";
	options.push_back(bar_beats);

	return options;
}

Error EditorImportPluginFLAC::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	bool loop = p_options["loop"];
	float loop_offset = p_options["loop_offset"];
	double bpm = p_options["bpm"];
	float beat_count = p_options["beat_count"];
	float bar_beats = p_options["bar_beats"];

	Ref<AudioStreamFLAC> flac_stream = AudioStreamFLAC::load_from_file(p_source_file);
	if (flac_stream.is_null()) {
		return ERR_CANT_OPEN;
	}
	
	flac_stream->set_loop(loop);
	flac_stream->set_loop_offset(loop_offset);
	flac_stream->set_bpm(bpm);
	flac_stream->set_beat_count(beat_count);
	flac_stream->set_bar_beats(bar_beats);

	return ResourceSaver::get_singleton()->save(flac_stream, p_save_path + String(".") + _get_save_extension());
}
