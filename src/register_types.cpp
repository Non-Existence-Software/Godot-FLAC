/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include "audio_stream_flac.h"
#include "resource_format_loader_flac.h"
#include "resource_format_saver_flac.h"
#include "editor_import_plugin_flac.h"
#include "editor_inspector_plugin_audio_stream_flac.h"
#include "editor_plugin_flac.h"

using namespace godot;
using namespace flac;

static Ref<ResourceFormatLoaderFLAC> resource_format_loader_flac;
static Ref<ResourceFormatSaverFLAC> resource_format_saver_flac;

void initialize_flac_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(AudioStreamPlaybackFLAC);
		GDREGISTER_CLASS(AudioStreamFLAC);
		GDREGISTER_CLASS(ResourceFormatLoaderFLAC);
		GDREGISTER_CLASS(ResourceFormatSaverFLAC);

		resource_format_loader_flac.instantiate();
		ResourceLoader::get_singleton()->add_resource_format_loader(resource_format_loader_flac, true);

		resource_format_saver_flac.instantiate();
		ResourceSaver::get_singleton()->add_resource_format_saver(resource_format_saver_flac, true);
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(EditorImportPluginFLAC);
		GDREGISTER_CLASS(EditorInspectorPluginAudioStreamFLACEditor);
		GDREGISTER_CLASS(EditorInspectorPluginAudioStreamFLAC);
		GDREGISTER_CLASS(EditorPluginFLAC);
		
		EditorPlugins::add_by_type<EditorPluginFLAC>();
	}
}

void uninitialize_flac_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ResourceLoader::get_singleton()->remove_resource_format_loader(resource_format_loader_flac);
		resource_format_loader_flac.unref();

		ResourceSaver::get_singleton()->remove_resource_format_saver(resource_format_saver_flac);
		resource_format_saver_flac.unref();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::remove_by_type<EditorPluginFLAC>();
	}
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT flac_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_flac_module);
	init_obj.register_terminator(uninitialize_flac_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}