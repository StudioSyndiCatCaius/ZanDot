# Building & Installing

## Requirements

- Python 3 + SCons
- A C++17 toolchain (MSVC on Windows, clang/gcc elsewhere)
- The `godot-cpp` submodule checked out under `godot-cpp/`

## Build

From the repo root:

```bash
scons platform=windows target=template_debug          # editor / debug builds
scons platform=windows target=template_release        # shipping
scons platform=windows target=template_debug -j8      # parallel
```

Output lands in `bin/`, e.g. `bin/libzandot.windows.template_debug.x86_64.dll`. The `.lib`/`.exp`
files next to it are link intermediates and are not needed by the game.

`build.bat` wraps the same command with the usual flags. If a `.bat` wrapper appears to do nothing
in your shell, call `scons` directly — that's the reliable path.

The editor plugin sources in `src/ed/` are compiled into `template_debug`/`editor` targets and
guarded out of release builds.

## Install into a game project

`copy_to_project.bat` copies everything into a Godot project. It defaults to the sibling
Liberty Belle project and takes an override path:

```bash
copy_to_project.bat "C:\path\to\project\addons\ZanDot"
```

It copies three things — do the same by hand if you prefer:

```
zandot.gdextension  →  <project>/addons/ZanDot/zandot.gdextension
bin/*.dll|so|dylib  →  <project>/addons/ZanDot/bin/
icons/*             →  <project>/addons/ZanDot/icons/
```

Then **restart the Godot editor** (GDExtension reloading is unreliable while the editor holds the
DLL open). If a build fails to overwrite the DLL, the editor still has it open — close Godot first.

## Adding a new class

1. Add `src/nodes/my_thing.{h,cpp}` (Node-derived) or `src/res/my_thing.{h,cpp}` (Resource-derived).
   SCons globs those directories, so no build-file edit is needed.
2. Declare it with `GDCLASS(MyThing, Base)` and implement `static void _bind_methods()`.
3. Register it in `src/register_types.cpp` with `GDREGISTER_CLASS(MyThing)` (or
   `GDREGISTER_INTERNAL_CLASS` to keep it out of the create dialogs).
4. Add an icon line to `zandot.gdextension` under `[icons]`, using the variant matching its base
   type — see [README.md](README.md#editor-icons):

   ```ini
   MyThing = "res://addons/ZanDot/icons/z_icon_3d.svg"
   ```
5. Rebuild, copy to the project, restart the editor.
6. Document it — add it to [class-index.md](class-index.md) and the relevant topic page.

### Binding conventions used in this codebase

- Bind both a PascalCase spec name and a snake_case alias for gameplay-facing calls.
- Expose script overrides with `GDVIRTUAL*` macros and dispatch from a public non-virtual method
  (`state_enter` → `_on_state_enter`).
- Runtime-only object references are stored as `ObjectID` and resolved through
  `ObjectDB::get_instance()`, so freed nodes read back as `null` instead of dangling.
