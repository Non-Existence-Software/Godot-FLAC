#!/usr/bin/env python
import os
import sys

libname = "FLAC"
projectdir = "project"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

env = localEnv.Clone()

env = SConscript("godot-cpp/SConstruct", {"api_version": 4.5, "env": env, "customs": customs})

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Add WIN32 define on Windows (required by config.h)
if env["platform"] == "windows":
    env.Append(CPPDEFINES=["WIN32"])

env.Append(CPPDEFINES=[
    "HAVE_CONFIG_H",
])

env.Append(CPPPATH=[
    "src/", 
    "#thirdparty/dr_libs/"
])

sources = (
    Glob("src/*.cpp")
)

# Remove /fp:strict flag
if env["CCFLAGS"]:
    if "/fp:strict" in env["CCFLAGS"]:
        env["CCFLAGS"].remove("/fp:strict")
        # Add /fp:precise flag
        env.Append(CCFLAGS=["/fp:precise"])

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("src/doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

copy = env.Install("{}/addons/{}/bin/{}/".format(projectdir, libname, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)
