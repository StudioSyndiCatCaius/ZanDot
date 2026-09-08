# Creatures & Abilities

Abilities are modular child nodes that encapsulate an actor action — jump, dash, attack, cast —
into a reusable component. They hang off a **`ZCreature`**, which owns them, routes input to them
when the actor is player-possessed, and can auto-pick them for AI.

`ZCreature` also owns the actor's **stats**: a `ZCreatureData` holding attributes, XP, inventory
and equipment.

```
Pawn (CharacterBody3D)
├── ZMover
└── ZCreature                ← root_owner = the pawn, data = ZCreatureData
    ├── ZAbility  "AbJump"   (input_action = "Jump")
    ├── ZAbility  "AbDash"   (input_action = "Dash")
    └── ZAbility  "AbBite"   (AI only — no input_action, scores via utility)
```

---

## ZCreature (Node)

The gameplay actor component: abilities, animation references, and the stat block.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `data` | `ZCreatureData` | — | Attributes / XP / inventory / equipment. Created on first write if unset. |
| `reset_attributes_on_ready` | `bool` | `true` | On ready, set every attribute already in `data` to its start value. |
| `root_owner` | `Node` | — | The actor this creature belongs to (usually the pawn). |
| `auto_find_root_owner` | `bool` | `true` | Resolve to the parent when `root_owner` is unset. |
| `autorun_utility` | `bool` | `false` | AI mode: periodically score and activate abilities. |
| `utility_frequency` | `float` | `0.2` | Seconds between scoring passes. |
| `utility_variance` | `float` | `0.05` | Random jitter added to the frequency (de-syncs crowds). |
| `animation_player` | `AnimationPlayer` | — | Convenience reference for abilities. |
| `animation_tree` | `AnimationTree` | — | Same. |

Turn `reset_attributes_on_ready` **off** when the creature is spawned with data restored from a
save — otherwise the pawn enters the tree at full health regardless of what was loaded. Assigning
`data` *after* the node is ready is never touched either way.

```gdscript
creature.get_abilities() -> Array[ZAbility]      # direct ZAbility children
creature.find_ability(name: String) -> ZAbility  # by node name; null if missing
creature.fire_notify(notify: String)             # broadcast to self + every child ability

signal on_ability_activated(ability: ZAbility, context)
signal on_ability_finished(ability: ZAbility, cancelled: bool)
signal on_creature_notify(notify: String)
```

### Input behaviour

- Input is **polled every frame** (not `_unhandled_input`).
- **Press edge** — every child ability whose `input_action` matches activates with
  `ability_activate(root_owner)`.
- **Release edge** — every active child ability with `cancel_ability_on_release = true` and a
  matching action calls `ability_stop(true)`.
- **Possession gate** — input is only processed when some `ZPlayer` in `Z.players` has
  `get_possessed_scene() == root_owner`. When possession is lost, held actions are flushed, which
  cancels release-sensitive abilities.

Game states do **not** route ability input; the creature does it itself.

### Utility AI

With `autorun_utility = true`, every `utility_frequency` (± `utility_variance`) seconds the
creature scores each non-active child ability via `get_utility_score(root_owner)` and activates
the highest scorer.

- Scores **≤ 0.0 are ignored** — that's the default, so abilities opt in by overriding.
- Ties break by tree order (first child wins).
- Scoring **never interrupts** a running ability; the ability must end itself.

### Notifies

`fire_notify("footstep")` broadcasts a string to the creature's `on_creature_notify` signal and to
every child ability's `_on_creature_notify`. Handy for driving abilities from animation tracks.

---

## Stats

Every stat API on `ZCreature` reads and writes `data`, and works whether or not `data` was assigned
— the creature makes one on first write. Each method has a PascalCase alias matching the ZanDot
naming standard (`attribute_damage` / `Attribute_Damage`); the snake_case name is shown here.

### Attributes

An **attribute** (`ZAttribute`) is a stat definition — health, stamina, mana, armour. The definition
is shared by every creature that uses it; the *current value* lives in `data.attributes`, keyed by
the resource.

```gdscript
creature.attribute_get_current(att: ZAttribute) -> float
creature.attribute_get_max(att: ZAttribute) -> float       # max_value scaled by rank
creature.attribute_get_percent(att: ZAttribute) -> float   # 0..1

creature.attribute_damage(att: ZAttribute, amount: float, instigator: ZCreature = null)
creature.attribute_set_current(att: ZAttribute, value: float)   # absolute, no signal
creature.attribute_reset(att: ZAttribute)                       # back to the start value
creature.attribute_reset_all()

signal on_attribute_damaged(att: ZAttribute, amount: float, instigator: ZCreature)
```

- `attribute_damage` **subtracts** `amount`; pass a negative number to heal.
- The result is clamped to `0 … max`, so the signal reports the amount **actually applied** —
  overkill and overheal don't produce phantom damage numbers. Nothing is emitted when the value
  doesn't move.
- Attributes with `is_static = true` can never be damaged: current always equals max, and
  `attribute_damage` / `attribute_set_current` / `attribute_reset` are no-ops on them.
- Before an attribute has been written to, `attribute_get_current` returns its start value rather
  than 0, so a creature is never accidentally born dead.

```gdscript
# Enemy takes 12 damage from the player.
enemy_creature.attribute_damage(preload("res://res/att_health.tres"), 12.0, player_creature)

func _on_attribute_damaged(att: ZAttribute, amount: float, instigator: ZCreature) -> void:
    if att.title == "Health":
        spawn_damage_number(amount, att.damage_color)
        if attribute_get_current(att) <= 0.0:
            die()
```

### Equipment (resource slots)

A **slot** is any `Resource` used as a key — typically a `ZResource` like `slot_weapon.tres`. What
goes in it is any `Resource` too, so the same system covers gear, spells and passives.

```gdscript
creature.rslot_equip(slot: Resource, res: Resource)   # null res unequips
creature.rslot_unequip(slot: Resource)
creature.rslot_unequip_all()
creature.rslot_get(slot: Resource) -> Resource        # null when empty

signal on_rslot_equip(creature: ZCreature, slot: Resource, res: Resource)
signal on_rslot_unequip(creature: ZCreature, slot: Resource, res: Resource)
```

Slots hold exactly one resource. Equipping over a full slot emits `on_rslot_unequip` for the old
resource first, then `on_rslot_equip` for the new one — so a listener that adds/removes stat
modifiers stays balanced.

### Inventory

`data.inventory` is a `Dictionary[Resource, int]` of item resource to held count.

```gdscript
creature.inventory_give(res: Resource, amount: int)          # negative takes away
creature.inventory_give_list(list: Dictionary)               # Dictionary[Resource, int]
creature.inventory_transfer(list: Dictionary, to: ZCreature)
creature.inventory_transfer_all(to: ZCreature)
creature.inventory_get_amount(res: Resource) -> int

signal on_inventory_edit(creature: ZCreature, res: Resource, amount_changed: int)
```

- Counts never go negative; a count reaching 0 removes the key entirely.
- `amount_changed` is the **real** delta. Taking 10 from a stack of 3 emits `-3`.
- Transfers are capped at what's actually held, and emit on both creatures (a negative edit on the
  giver, a positive one on the receiver).

```gdscript
# Loot the corpse.
corpse_creature.inventory_transfer_all(player_creature)
```

### Levelling

A **leveller** (`ZLeveler`) is a track that gains XP — character level, weapon proficiency, faction
standing. Level is *derived* from XP, never stored; `data.xp` holds the totals.

```gdscript
creature.leveling_xp_add(res: ZLeveler, xp: float)
creature.leveling_xp_set(res: ZLeveler, xp: float)

creature.leveling_level_set(res: ZLeveler, lvl: int)   # jumps XP to that level's threshold
creature.leveling_level_get(res: ZLeveler) -> int

creature.leveling_xp_get_current(res: ZLeveler) -> float
creature.leveling_xp_get_percent(res: ZLeveler) -> float  # progress through the current level, 0..1

creature.leveling_xp_min(res: ZLeveler) -> float   # XP threshold of the current level
creature.leveling_xp_max(res: ZLeveler) -> float   # threshold of the next level

signal on_xp_change(creature: ZCreature, res: ZLeveler, old_xp: float, new_xp: float)
signal on_level_change(creature: ZCreature, res: ZLeveler, old_lvl: int, new_lvl: int)
```

- XP is clamped to the curve's level-0 and max-level thresholds, so it can't run past the cap.
- `on_level_change` fires after `on_xp_change`, and only when the derived level actually moved —
  including downward, if you take XP away.
- `leveling_xp_get_percent` returns `1.0` at max level.

```gdscript
func _on_enemy_killed(enemy: ZCreature) -> void:
    player_creature.leveling_xp_add(lvl_hero, 25.0)

# XP bar
bar.value = player_creature.leveling_xp_get_percent(lvl_hero)
label.text = "%s %d" % [lvl_hero.level_name, player_creature.leveling_level_get(lvl_hero)]
```

---

## ZAttribute (ZResource)

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `max_value` | `float` | `100.0` | Ceiling before the rank curve is applied. |
| `is_static` | `bool` | `false` | `true`: never damageable; current always equals max. |
| `start_value_percent` | `float` | `1.0` | Fraction of max the attribute starts at. |
| `rank_value_curve` | `Curve` | — | Multiplier on `max_value`, sampled at the creature's `data.rank`. |
| `damage_color` | `Color` | red | For damage numbers / flashes. Read it, nothing reads it for you. |
| `round_to` | `int` | `2` | Decimal places values are snapped to. `0` = whole numbers, `-1` = nearest ten. |

Inherits the `ZResource` params (`title`, `icon`, `description`, `tags`, `color`, `meta`), so an
attribute carries its own display name and bar colour.

```gdscript
att.get_max_value_at_rank(rank: int) -> float
att.get_start_value_at_rank(rank: int) -> float
att.round_value(value: float) -> float
```

**Rank scaling.** Leave `rank_value_curve` empty and `max_value` is used as-is. Set one and the
sampled value multiplies `max_value` — a curve going 1.0 → 5.0 over rank 0 → 10 gives a rank-10
creature five times the health. Set the curve's **domain** to cover your rank range; a curve left on
the default 0–1 domain will clamp every rank above 1 to the same value.

## ZLeveler (ZResource)

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `level_name` | `String` | `""` | Display name ("Level", "Swordsmanship"). |
| `max_level` | `int` | `99` | XP is capped at this level's threshold. |
| `xp_curve` | `Curve` | — | Cumulative XP required to *reach* a level, sampled at the level number. |
| `round_xp_to` | `int` | `2` | Decimal places XP is snapped to. |

```gdscript
leveler.get_xp_for_level(level: int) -> float
leveler.get_level_for_xp(xp: float) -> int
leveler.round_xp(xp: float) -> float
```

Set the curve's **domain** to `0 … max_level` and its **range** to your XP scale. The curve should
increase — a flat or decreasing stretch resolves to the highest level whose threshold is met. With
no curve at all, one XP is one level.

## ZCreatureData (Resource)

The serializable half of a creature. Kept off the node so it can be dropped straight into a
`ZSave` and handed back to a freshly spawned pawn.

| Property | Type | Notes |
| --- | --- | --- |
| `rank` | `int` | Feeds `ZAttribute.rank_value_curve`. |
| `attributes` | `Dictionary[ZAttribute, float]` | Current value per attribute. |
| `xp` | `Dictionary[ZLeveler, float]` | Total XP per leveller. |
| `inventory` | `Dictionary[Resource, int]` | Item resource to held count. |
| `equipment` | `Dictionary[Resource, Resource]` | Slot resource to the resource in it. |
| `params` | `Dictionary` | Free-form game payload. |

```gdscript
data.duplicate_data() -> ZCreatureData   # fresh dictionaries, same resource keys/values
```

**Authoring in the editor.** Add a `ZCreatureData` to the `ZCreature`'s `data` property and put your
`ZAttribute` keys in `attributes` — the values you type there don't matter while
`reset_attributes_on_ready` is on, since each is set to its start value on ready. Attributes not
listed still work; they just report their start value until something writes to them.

**Persisting.** `ZSave.creature_data` is a `Dictionary[StringName, ZCreatureData]`, so a save carries
one entry per creature you care about:

```gdscript
Z.save_game.creature_data_set(&"player", player_creature.data)

# ... on load
var spawned := player_scene.instantiate()
var creature: ZCreature = spawned.get_node("ZCreature")
creature.reset_attributes_on_ready = false
creature.data = Z.save_game.creature_data_get(&"player")
```

---

## ZAbility (Node)

A discrete action. Must be a child of a `ZCreature`.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `input_action` | `String` | `""` | InputMap action that triggers it. Empty = AI/script only. |
| `cancel_ability_on_release` | `bool` | `false` | `true`: releasing the input calls `ability_stop(true)`. |
| `is_active` | `bool` | — | Read-only runtime state. |

### Callable API

```gdscript
ability.ability_activate(context = null) -> bool
# false if already active or _ability_can_activate rejected it.

ability.ability_stop(cancelled := false)
ability.ability_can_activate(context = null) -> bool
ability.get_utility_score(context = null) -> float
ability.creature_notify(notify: String)

ability.get_creature() -> ZCreature
ability.get_root_owner() -> Node        # shortcut to the creature's root_owner
```

PascalCase aliases exist: `Ability_Activate`, `Ability_Stop`, `Ability_CanActivate`,
`Ability_GetUtilityScore`.

### Overridables

```gdscript
func _ability_can_activate(context) -> bool      # false blocks activation
func _on_ability_activated(context) -> void      # once, on start
func _process_activated_ability(delta: float)    # every frame while is_active
func _on_ability_finished(cancelled: bool)       # after stopping
func _get_utility_score(context) -> float        # AI score; default 0.0 = never auto-picked
func _on_creature_notify(notify: String)         # from ZCreature.fire_notify
```

### Signals

```gdscript
signal on_ability_activated(context)
signal on_ability_finished(cancelled: bool)
```

---

## Writing an ability

### Instant (jump)

```gdscript
extends ZAbility
class_name ab_jump

@export var jump_speed: float = 10.0

func _ability_can_activate(context) -> bool:
    var mover := _get_mover()
    return mover != null and mover.is_on_floor()

func _on_ability_activated(context) -> void:
    _get_mover().launch(Vector3.UP, jump_speed)
    ability_stop(false)                       # instant: finish immediately

func _get_mover() -> ZMover:
    var actor := get_root_owner()
    return actor.get_node_or_null("ZMover") as ZMover if actor else null
```

### Durational (dash, hold-attack)

Set `cancel_ability_on_release = true` so releasing the input ends it.

```gdscript
extends ZAbility
class_name ab_dash

@export var dash_duration: float = 0.25
var _elapsed := 0.0

func _on_ability_activated(context) -> void:
    _elapsed = 0.0

func _process_activated_ability(delta: float) -> void:
    _elapsed += delta
    if _elapsed >= dash_duration:
        ability_stop(false)

func _on_ability_finished(cancelled: bool) -> void:
    # cancelled == true when stopped by input release or lost possession.
    pass
```

### AI-scored

```gdscript
func _get_utility_score(context) -> float:
    var target := _find_target()
    if target == null:
        return 0.0                            # ≤ 0 is never chosen
    var d := get_root_owner().global_position.distance_to(target.global_position)
    return clampf(1.0 - d / 8.0, 0.0, 1.0)
```

### Adding it in the editor

1. Open the actor scene, select the `ZCreature` node.
2. Add a child `ZAbility`, name it `Ab<Name>`.
3. Attach your script.
4. Set `input_action` to an existing InputMap action (or leave empty for AI-only).
5. Tune exported parameters on the node.

---

## Current limitations

- No tag/blocking system — multiple abilities can run at once. Enforce exclusivity yourself in
  `_ability_can_activate`.
- No automatic costs or cooldowns. Attributes give you the pool; spend it yourself in
  `_on_ability_activated` and gate it in `_ability_can_activate`.
- Utility AI is simple: no weighted random, no interruption of a running ability.
- Equipment doesn't modify attributes on its own — connect `on_rslot_equip` / `on_rslot_unequip`
  and apply the change yourself.

---

## ZSkinModel (Node)

Manages a character (or prop) mesh as a **swappable model slot**. Drop imported FBX/GLB/Blend
scenes in and out at runtime while keeping a shared `AnimationTree` wired to the active
`AnimationPlayer`.

```
Pawn
├── ZSkinModel
│   └── sk_c_stella          ← model_default (imported scene instance)
├── AnimationTree            ← anim_tree (stays put; AnimPlayer path is updated)
└── ZCreature
```

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `model_default` | `Node3D` | — | The main model instance currently in use. |
| `model_slots` | `Dictionary[StringName, Node3D]` | `{}` | Named modular slots (head, body, weapon, …). |
| `anim_tree` | `AnimationTree` | — | Reused across swaps; its AnimPlayer path is rewired. |

### Swapping models

```gdscript
# Replace the default model with another imported scene.
skin.set_model_from_scene(preload("res://chars/sk_stella_alt.glb"))

# Modular slot — only works when the key already exists in model_slots.
skin.model_slots[&"weapon"] = $WeaponAttach/Sword   # declare the slot first
skin.set_model_from_scene(preload("res://weapons/axe.glb"), &"weapon")

# Empty or unknown slot name falls back to model_default.
skin.set_model_from_scene(some_scene, &"missing")   # → default
```

- Instantiates the `PackedScene` (root must be a `Node3D`).
- Parents the new instance where the old one lived, copies its local transform, frees the old node.
- Calls `update_animation()` automatically.

PascalCase alias: `Set_Model_From_Scene`.

### Animation rewiring

```gdscript
skin.update_animation()   # or Update_Animation()
```

Finds the first `AnimationPlayer` under `model_default` (then under each slot) and sets
`anim_tree.anim_player` to the path from the tree to that player. Call this yourself if you
assign `model_default` / slots manually without going through `set_model_from_scene`.

### Slot helpers

```gdscript
skin.has_model_slot(&"weapon") -> bool
skin.get_model_slot(&"weapon") -> Node3D   # null if missing
```
