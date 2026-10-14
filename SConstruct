#!/usr/bin/env python
import os
import sys

libname = "FLAC"
projectdir = "project"

env = SConscript("godot-cpp/SConstruct", {"api_version": "4.7"})

env.Append(CPPPATH=[
    "src/", 
    "#thirdparty/dr_libs/"
])

sources = (
    Glob("src/*.cpp")
)

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("src/doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "project/addons/{}/bin/{}/{}".format(libname, env['platform'], lib_filename),
    source=sources,
)

Default(library)
