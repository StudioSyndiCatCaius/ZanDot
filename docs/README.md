# ZanDot Documentation

ZanDot is a gameplay framework GDExtension for **Godot 4** (min 4.3), written in C++ with
godot-cpp. It provides an Unreal-inspired gameplay skeleton — a global `Z` singleton, game
modes and states, players and pawns, movement, abilities, saves, flow graphs (dialogue),
and EQS-style world queries.

Everything here is available to GDScript with no extra setup: install the addon, restart the
editor, and the `Z*` classes appear in the node/resource create dialogs.

## Read this first

| If you want to… | Go to |
| --- | --- |
| Understand the big picture and boot order | [core.md](core.md) |
| Spawn a level, switch game states, change scenes | [core.md](core.md) |
| Route input to a pawn / UI, work with `ZPlayer` | [input.md](input.md) |
| Move a character, tune locomotion, camera lag / look-at follower | [movement.md](movement.md) |
| Give an actor abilities (jump, dash, attack) or AI; swap skins | [creatures.md](creatures.md) |
| Save/load a game or global settings | [saves.md](saves.md) |
| Build dialogue / branching logic | [flow.md](flow.md) |
| Pop a confirm / info / progress / choice menu and await it | [menus.md](menus.md) |
| Ask "where should the AI stand?" | [worldquery.md](worldquery.md) |
| Play game music, loop points, crossfades | [audio.md](audio.md) |
| Interp helpers, name-vars, CSV/YAML/INI/TOML, one-shot sounds, debug roots, UI lists, world-space canvas | [utilities.md](utilities.md) |
| Compile the extension and install it into a game | [building.md](building.md) |
| Copy-paste answers to common tasks | [recipes.md](recipes.md) |

There is also a flat [class-index.md](class-index.md) listing every registered class in one place.

## The 60-second version

```gdscript
# Z is an Engine singleton — always available, no autoload needed.
var player: ZPlayer = Z.players[0]
var pawn: Node = Z.Player_Get_Pawn()          # currently possessed pawn for player 0

# Levels run under one ZGameMode, which runs one ZGameState at a time.
Z.current_game_mode.change_state_scene(preload("res://states/gs_pause.tscn"))

# Scene changes go through Z so fades and player-start placement work.
Z.Scene_Transit(preload("res://levels/level_2.tscn"))
```

Structure of a typical game:

```
Level scene (root)
└── ZGameMode                 ← one per level; spawns pawns, owns the active state
    └── ZGameState            ← the current "mode of play" (explore / pause / dialogue)
ZPlayerStart3D                ← placed in the level; where pawns spawn
Pawn scene (CharacterBody3D)
├── ZMover                    ← locomotion
└── ZCreature                 ← owns abilities
    ├── ZAbility (AbJump)
    └── ZAbility (AbDash)
```

## Naming conventions

Most gameplay APIs are bound **twice**: a spec-style PascalCase name and a Godot-native
snake_case alias. They are the same function — pick one style and stay consistent.

```gdscript
Z.SaveGame_Create()      == Z.save_game_create()
mover.Move_ByInput(v)    == mover.move_by_input(v)
ability.Ability_Stop()   == ability.ability_stop()
```

Overridable hooks follow Godot's convention: implement `_on_state_enter`, `_ability_can_activate`,
`_check`, `_define`, … in GDScript; the engine calls them through the matching public method
(`state_enter`, `ability_can_activate`, `check`, `rebuild_ports`).

## Editor icons

Every ZanDot class uses the Z icon with a **type dot** in the bottom-right corner so you can
tell at a glance what a class derives from:

| Dot | Base type |
| --- | --- |
| ⚪ white | `Object` / `RefCounted` / plain `Node` |
| 🔵 blue | `Node2D` |
| 🔴 red | `Node3D` |
| 🟢 green | `Control` |
| 🟣 purple | `Resource` |

## Source layout

```
src/
├── nodes/     Node-derived classes (Z, ZMover, ZGameMode, …)
├── res/       Resource-derived classes (ZPlayer, ZMoveMode, flow nodes, query shapes, …)
├── ed/        Editor-only plugins (Flow editor, resource previews)
└── register_types.cpp   class registration + singleton creation
icons/         SVG editor icons
docs/          this documentation
```
