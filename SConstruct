#!/usr/bin/env python
"""
ZanDot GDExtension build file.

Usage (from this directory):
  scons platform=windows target=template_debug
  scons platform=windows target=template_release
"""

import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources += Glob("src/res/*.cpp")
sources += Glob("src/nodes/*.cpp")
# Editor plugin (compiled for debug/editor targets via DEBUG_ENABLED guards in source).
sources += Glob("src/ed/*.cpp")

# Optional class reference docs (Godot 4.3+).
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference (pre-4.3 godot-cpp baseline).")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/libzandot.{}.{}.framework/libzandot.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "bin/libzandot{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
