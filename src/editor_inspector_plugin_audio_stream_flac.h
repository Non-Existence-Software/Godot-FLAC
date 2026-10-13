// This file is adapted from https://github.com/godotengine/godot/blob/master/editor/audio/audio_stream_editor_plugin.h

#pragma once

#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/label.hpp>

using namespace godot;

namespace flac {

class EditorInspectorPluginAudioStreamFLACEditor : public ColorRect {
	GDCLASS(EditorInspectorPluginAudioStreamFLACEditor, ColorRect);

	Ref<AudioStream> stream;

	AudioStreamPlayer *_player = nullptr;
	Label *_current_label = nullptr;
	Label *_duration_label = nullptr;

	Button *_play_button = nullptr;
	Button *_stop_button = nullptr;

	float _current = 0;
	bool _dragging = false;
	bool _pausing = false;

	const StringName SNAME_status_source = "status_source";
	const StringName SNAME_EditorFonts = "EditorFonts";
	const StringName SNAME_EditorIcons = "EditorIcons";
	const StringName SNAME_Editor = "Editor";
	const StringName SNAME_MainPlay = "MainPlay";
	const StringName SNAME_Stop = "Stop";
	const StringName SNAME_Pause = "Pause";
	const StringName SNAME_font = "font";
	const StringName SNAME_dark_color_1 = "dark_color_1";
	const StringName SNAME_dark_color_2 = "dark_color_2";
	const StringName SNAME_finished = "finished";
	const StringName SNAME_pressed = "pressed";
	const StringName SNAME_changed = "changed";

	Ref<Texture2D> _editor_get_theme_icon(const StringName &p_icon_name);
	float _editor_get_scale();

protected:
	void _notification(int p_what);
	void _play();
	void _stop();
	void _on_finished();
	void _stream_changed();

	static void _bind_methods() {}

public:
	void set_stream(const Ref<AudioStream> &p_stream);

	EditorInspectorPluginAudioStreamFLACEditor();
};

class EditorInspectorPluginAudioStreamFLAC : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginAudioStreamFLAC, EditorInspectorPlugin);

protected:
	static void _bind_methods() {}

public:
	virtual void _parse_begin(Object *p_object) override;
	virtual bool _can_handle(Object *p_object) const override;
};

}