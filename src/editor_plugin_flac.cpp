#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/editor_interface.hpp>

#include "editor_plugin_flac.h"

namespace flac {

void EditorPluginFLAC::_enter_tree() {
	editor_import_plugin_flac.instantiate();
	add_import_plugin(editor_import_plugin_flac);

	editor_inspector_plugin_audio_stream_flac.instantiate();
	add_inspector_plugin(editor_inspector_plugin_audio_stream_flac);
}

void EditorPluginFLAC::_exit_tree() {
	remove_import_plugin(editor_import_plugin_flac);
	editor_import_plugin_flac.unref();

	remove_inspector_plugin(editor_inspector_plugin_audio_stream_flac);
	editor_inspector_plugin_audio_stream_flac.unref();
}

}