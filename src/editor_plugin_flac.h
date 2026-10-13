#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>

#include "editor_import_plugin_flac.h"
#include "editor_inspector_plugin_audio_stream_flac.h"

using namespace godot;

namespace flac {

class EditorPluginFLAC : public EditorPlugin {
	GDCLASS(EditorPluginFLAC, EditorPlugin);

private:
	Ref<EditorImportPluginFLAC> editor_import_plugin_flac;
	Ref<EditorInspectorPluginAudioStreamFLAC> editor_inspector_plugin_audio_stream_flac;

protected:
	static void _bind_methods() {}

public:
	void _enter_tree() override;
	void _exit_tree() override;
};

}