/**************************************************************************/
/*  audio_stream_flac.cpp                                                 */
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

#define DR_FLAC_NO_OGG
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO

#include "audio_stream_flac.h"

#include "core/io/file_access.h"
#include "core/object/class_db.h"

#include <thirdparty/dr_libs/dr_bridge.h>

int AudioStreamPlaybackFLAC::_mix_internal(AudioFrame *p_buffer, int p_frames) {
	if (!active) {
		return 0;
	}

	int todo = p_frames;

	int frames_mixed_this_step = p_frames;

	int beat_length_frames = -1;
	bool use_loop = looping_override ? looping : flac_stream->loop;

	bool beat_loop = flac_stream->has_loop() && flac_stream->get_bpm() > 0 && flac_stream->get_beat_count() > 0;
	if (beat_loop) {
		beat_length_frames = flac_stream->get_beat_count() * flac_stream->sample_rate * 60 / flac_stream->get_bpm();
	}

	while (todo && active) {
		float buf_frame[2];
		
		int samples_mixed  = drflac_read_pcm_frames_f32(&flacd, 1, buf_frame);

		if (samples_mixed) {
			p_buffer[p_frames - todo] = AudioFrame(buf_frame[0], buf_frame[flacd.channels - 1]);
			if (loop_fade_remaining < FADE_SIZE) {
				p_buffer[p_frames - todo] += loop_fade[loop_fade_remaining] * (float(FADE_SIZE - loop_fade_remaining) / float(FADE_SIZE));
				loop_fade_remaining++;
			}
			--todo;
			++frames_mixed;

			if (beat_loop && (int)frames_mixed >= beat_length_frames) {
				for (int i = 0; i < FADE_SIZE; i++) {
					samples_mixed = drflac_read_pcm_frames_f32(&flacd, 1, buf_frame);
					loop_fade[i] = AudioFrame(buf_frame[0], buf_frame[flacd.channels - 1]);
					if (!samples_mixed) {
						break;
					}
				}
				loop_fade_remaining = 0;
				seek(flac_stream->loop_offset);
				loops++;
			}
		}

		else {
			//EOF
			if (use_loop) {
				seek(flac_stream->loop_offset);
				loops++;
			} else {
				frames_mixed_this_step = p_frames - todo;
				//fill remainder with silence
				for (int i = p_frames - todo; i < p_frames; i++) {
					p_buffer[i] = AudioFrame(0, 0);
				}
				active = false;
				todo = 0;
			}
		}
	}
	return frames_mixed_this_step;
}

float AudioStreamPlaybackFLAC::get_stream_sampling_rate() {
	return flac_stream->sample_rate;
}

void AudioStreamPlaybackFLAC::start(double p_from_pos) {
	active = true;
	seek(p_from_pos);
	loops = 0;
	begin_resample();
}

void AudioStreamPlaybackFLAC::stop() {
	active = false;
}

bool AudioStreamPlaybackFLAC::is_playing() const {
	return active;
}

int AudioStreamPlaybackFLAC::get_loop_count() const {
	return loops;
}

double AudioStreamPlaybackFLAC::get_playback_position() const {
	return double(frames_mixed) / flac_stream->sample_rate;
}

void AudioStreamPlaybackFLAC::seek(double p_time) {
	if (!active) {
		return;
	}

	if (p_time >= flac_stream->get_length()) {
		p_time = 0;
	}

	frames_mixed = uint32_t(flac_stream->sample_rate * p_time);
	drflac_seek_to_pcm_frame(&flacd, (uint64_t)frames_mixed);
}

void AudioStreamPlaybackFLAC::tag_used_streams() {
	flac_stream->tag_used(get_playback_position());
}

void AudioStreamPlaybackFLAC::set_is_sample(bool p_is_sample) {
	_is_sample = p_is_sample;
}

bool AudioStreamPlaybackFLAC::get_is_sample() const {
	return _is_sample;
}

Ref<AudioSamplePlayback> AudioStreamPlaybackFLAC::get_sample_playback() const {
	return sample_playback;
}

void AudioStreamPlaybackFLAC::set_sample_playback(const Ref<AudioSamplePlayback> &p_playback) {
	sample_playback = p_playback;
	if (sample_playback.is_valid()) {
		sample_playback->stream_playback = Ref<AudioStreamPlayback>(this);
	}
}

void AudioStreamPlaybackFLAC::set_parameter(const StringName &p_name, const Variant &p_value) {
	if (p_name == SNAME("looping")) {
		if (p_value == Variant()) {
			looping_override = false;
			looping = false;
		} else {
			looping_override = true;
			looping = p_value;
		}
	}
}

Variant AudioStreamPlaybackFLAC::get_parameter(const StringName &p_name) const {
	if (looping_override && p_name == SNAME("looping")) {
		return looping;
	}
	return Variant();
}

AudioStreamPlaybackFLAC::~AudioStreamPlaybackFLAC() {
	drflac_close(&flacd);
}

Ref<AudioStreamPlayback> AudioStreamFLAC::instantiate_playback() {
	Ref<AudioStreamPlaybackFLAC> flacs;

	ERR_FAIL_COND_V_MSG(data.is_empty(), flacs,
			"This AudioStreamFLAC does not have an audio file assigned "
			"to it. AudioStreamFLAC should not be created from the "
			"inspector or with `.new()`. Instead, load an audio file.");

	flacs.instantiate();
	flacs->flac_stream = Ref<AudioStreamFLAC>(this);

	drflac *flacd = drflac_open_memory(data.ptr(), data_len, (drflac_allocation_callbacks *)&dr_alloc_calls);
	flacs->flacd = *flacd;
	
	flacs->frames_mixed = 0;
	flacs->active = false;
	flacs->loops = 0;
	
	ERR_FAIL_COND_V(!flacd, Ref<AudioStreamPlaybackFLAC>());

	return flacs;
}

String AudioStreamFLAC::get_stream_name() const {
	return ""; //return stream_name;
}

void AudioStreamFLAC::clear_data() {
	data.clear();
}

void AudioStreamFLAC::set_data(const Vector<uint8_t> &p_data) {
	int src_data_len = p_data.size();

	drflac *flacd = drflac_open_memory(p_data.ptr(), src_data_len, (drflac_allocation_callbacks *)&dr_alloc_calls);
	if (!flacd || flacd->sampleRate == 0){
		ERR_FAIL_MSG("Failed to decode FLAC file. Make sure it is a valid FLAC audio file.");
	}

	channels = flacd->channels;
	sample_rate = flacd->sampleRate;
	length = float(flacd->totalPCMFrameCount) / (flacd->sampleRate);

	drflac_close(flacd);

	data = p_data;
	data_len = src_data_len;
}

Vector<uint8_t> AudioStreamFLAC::get_data() const {
	return Vector<uint8_t>(data);
}

void AudioStreamFLAC::set_loop(bool p_enable) {
	loop = p_enable;
}

bool AudioStreamFLAC::has_loop() const {
	return loop;
}

void AudioStreamFLAC::set_loop_offset(double p_seconds) {
	loop_offset = p_seconds;
}

double AudioStreamFLAC::get_loop_offset() const {
	return loop_offset;
}

double AudioStreamFLAC::get_length() const {
	return length;
}

bool AudioStreamFLAC::is_monophonic() const {
	return false;
}

void AudioStreamFLAC::get_parameter_list(List<Parameter> *r_parameters) {
	r_parameters->push_back(Parameter(PropertyInfo(Variant::BOOL, "looping", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CHECKABLE), Variant()));
}

void AudioStreamFLAC::set_bpm(double p_bpm) {
	ERR_FAIL_COND(p_bpm < 0);
	bpm = p_bpm;
	emit_changed();
}

double AudioStreamFLAC::get_bpm() const {
	return bpm;
}

void AudioStreamFLAC::set_beat_count(int p_beat_count) {
	ERR_FAIL_COND(p_beat_count < 0);
	beat_count = p_beat_count;
	emit_changed();
}

int AudioStreamFLAC::get_beat_count() const {
	return beat_count;
}

void AudioStreamFLAC::set_bar_beats(int p_bar_beats) {
	ERR_FAIL_COND(p_bar_beats < 0);
	bar_beats = p_bar_beats;
	emit_changed();
}

int AudioStreamFLAC::get_bar_beats() const {
	return bar_beats;
}

Ref<AudioSample> AudioStreamFLAC::generate_sample() const {
	Ref<AudioSample> sample;
	sample.instantiate();
	sample->stream = this;
	sample->loop_mode = loop
			? AudioSample::LoopMode::LOOP_FORWARD
			: AudioSample::LoopMode::LOOP_DISABLED;
	sample->loop_begin = loop_offset;
	sample->loop_end = 0;
	return sample;
}

Ref<AudioStreamFLAC> AudioStreamFLAC::load_from_buffer(const Vector<uint8_t> &p_stream_data) {
	Ref<AudioStreamFLAC> flac_stream;
	flac_stream.instantiate();
	flac_stream->set_data(p_stream_data);
	ERR_FAIL_COND_V_MSG(flac_stream->get_data().is_empty(), Ref<AudioStreamFLAC>(), "FLAC decoding failed. Check that your data is a valid FLAC audio stream.");
	return flac_stream;
}

Ref<AudioStreamFLAC> AudioStreamFLAC::load_from_file(const String &p_path) {
	const Vector<uint8_t> stream_data = FileAccess::get_file_as_bytes(p_path);
	ERR_FAIL_COND_V_MSG(stream_data.is_empty(), Ref<AudioStreamFLAC>(), vformat("Cannot open file '%s'.", p_path));
	return load_from_buffer(stream_data);
}


void AudioStreamFLAC::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamFLAC", D_METHOD("load_from_buffer", "stream_data"), &AudioStreamFLAC::load_from_buffer);
	ClassDB::bind_static_method("AudioStreamFLAC", D_METHOD("load_from_file", "path"), &AudioStreamFLAC::load_from_file);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamFLAC::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamFLAC::get_data);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamFLAC::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamFLAC::has_loop);

	ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamFLAC::set_loop_offset);
	ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamFLAC::get_loop_offset);

	ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &AudioStreamFLAC::set_bpm);
	ClassDB::bind_method(D_METHOD("get_bpm"), &AudioStreamFLAC::get_bpm);

	ClassDB::bind_method(D_METHOD("set_beat_count", "count"), &AudioStreamFLAC::set_beat_count);
	ClassDB::bind_method(D_METHOD("get_beat_count"), &AudioStreamFLAC::get_beat_count);

	ClassDB::bind_method(D_METHOD("set_bar_beats", "count"), &AudioStreamFLAC::set_bar_beats);
	ClassDB::bind_method(D_METHOD("get_bar_beats"), &AudioStreamFLAC::get_bar_beats);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bpm", PROPERTY_HINT_RANGE, "0,400,0.01,or_greater"), "set_bpm", "get_bpm");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_count", PROPERTY_HINT_RANGE, "0,512,1,or_greater"), "set_beat_count", "get_beat_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_beats", PROPERTY_HINT_RANGE, "2,32,1,or_greater"), "set_bar_beats", "get_bar_beats");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset"), "set_loop_offset", "get_loop_offset");
}

AudioStreamFLAC::AudioStreamFLAC() {
}

AudioStreamFLAC::~AudioStreamFLAC() {
	clear_data();
}