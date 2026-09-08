# Menus & global events

`ZEvent` is the front door: a static class (like `Z_BGM`) whose helpers open a menu, hand back a
signal, and let you `await` it. `ZMenu` is the base class those menus derive from.

For a lightweight string+dict broadcast (no menu), use `Z.gEvent` / `Z.on_gEvent` instead —
see [core.md](core.md#z-engine-singleton).

```gdscript
await ZEvent.Info("Game saved.")
await ZEvent.Confirm("Quit to title?", _quit_to_title, Callable(), "Quit", "Stay")
```

Every helper returns the menu's **`tree_exited`** signal, so an `await` resumes once the menu is
*totally* gone — after the close animation — not at the moment it was answered. Use the callbacks
(or the menu's own signals) for the answer; use the `await` for "the menu is out of the way now".

---

## ZEvent (RefCounted, static)

| Call | Returns | Notes |
| --- | --- | --- |
| `ZEvent.Confirm(text, on_yes := Callable(), on_no := Callable(), yes_text := "Yes", no_text := "No")` | `Signal` | Yes/no prompt. Exactly one callback fires. |
| `ZEvent.Info(text)` | `Signal` | Message with a single dismiss. |
| `ZEvent.Process(text, update)` | `Signal` | `update` is a `Callable` returning 0–1; the menu closes at 1. |
| `ZEvent.Choice(choices, on_select := Callable(), override_menu := null)` | `Signal` | `choices: Array[ZFlowNode_Choice]`; `on_select` gets the index into that array. |
| `ZEvent.OpenMenu(scene)` | `ZMenu` | Opens any `ZMenu` scene; `await menu.tree_exited` yourself. |
| `ZEvent.Menus_Get_Open()` | `Array[ZMenu]` | Menus in the tree, oldest first. |
| `ZEvent.Menu_Get_Top()` | `ZMenu` | Front-most menu that is not closing, or `null`. |
| `ZEvent.Menu_Is_Open()` | `bool` | |

snake_case aliases exist for all of them (`ZEvent.confirm`, `ZEvent.open_menu`, …).

Menus are parented to the **root viewport**, so they survive a scene change.

```gdscript
# Confirm — the callbacks answer, the await waits for the menu to finish closing.
func _on_quit_pressed() -> void:
    await ZEvent.Confirm("Quit to title?", _do_quit)
    print("menu fully closed")

# Process — poll something until it reports 1.0.
var loaded := 0.0
await ZEvent.Process("Loading…", func() -> float: return loaded)
```

### Which scene each helper opens

`Project Settings → ZanDot → Menus` holds one scene path per built-in menu:

| Setting | Opened by | Scene root must be |
| --- | --- | --- |
| `zandot/menus/menu_confirm` | `ZEvent.Confirm` | `ZMenu_Confirm` |
| `zandot/menus/menu_info` | `ZEvent.Info` | `ZMenu_Info` |
| `zandot/menus/menu_process` | `ZEvent.Process` | `ZMenu_Process` |
| `zandot/menus/menu_choice` | `ZEvent.Choice` | `ZMenu_Choice` |

Leave a slot empty and ZanDot builds a plain fallback in code instead — grey panel, plain buttons —
so the call still does something while you are prototyping. The same fallback is used (with a
warning) when the configured scene fails to load or its root is the wrong type. Ship your own
scenes.

---

## ZMenu (CanvasLayer)

Base class for anything `ZEvent` opens. Subclass it in C++ or GDScript for menus of your own.

| Property | Type | Default | Notes |
| --- | --- | --- | --- |
| `animation_player` | `AnimationPlayer` | — | Drives both phases. Unset = open/close resolve instantly. |
| `anim_opened` | `StringName` | `&"Open"` | Played on ready. |
| `anim_closed` | `StringName` | `&"Open"` | Played on `CloseMenu()`. |
| `anim_closed_reversed` | `bool` | `true` | Play `anim_closed` backwards — which is why both names default to the same animation. |
| `pause_gameplay` | `bool` | `true` | Pauses the tree while open; the menu sets its own process mode to `ALWAYS`, so it naturally excludes itself. |
| `action_confirm` | `StringName` | `&"ui_select"` | Used by the built-in subclasses. |
| `action_close` | `StringName` | `&"ui_cancel"` | Same. |
| `is_opening` | `bool` | — | Read-only; open animation is playing. |
| `is_closing` | `bool` | — | Read-only; close animation is playing, or the menu is about to free. |
| `is_open` | `bool` | — | Read-only; opened and not yet closing. |

```gdscript
menu.CloseMenu()            # == menu.close_menu(); safe to call twice
ZMenu.Menu_Get_Top()        # front-most menu (also on ZEvent)
```

Signals: `on_menu_opened` (open animation finished) and `on_menu_closed` (closing began).
GDScript overrides: `_on_menu_opened()` / `_on_menu_closed()`.

Nested menus stack: only the front-most one reacts to `action_confirm` / `action_close`, and the
tree stays paused until the last pausing menu is gone.

---

## Built-in menus

### ZMenu_Confirm

| Property | Type | Notes |
| --- | --- | --- |
| `lbl_text` | `RichTextLabel` | Shows `text` (BBCode). |
| `btn_yes` / `btn_no` | `Button` | Optional; captioned from `yes_text` / `no_text`. |
| `text` / `yes_text` / `no_text` | `String` | |

`action_confirm` answers yes, `action_close` answers no. Signal `on_confirm(confirmed: bool)` fires
once, then the menu closes. `confirm(true/false)` does the same from code.

### ZMenu_Info

| Property | Type | Notes |
| --- | --- | --- |
| `lbl_text` | `RichTextLabel` | Shows `text`. |
| `btn_close` | `Button` | Optional. |
| `text` | `String` | |

Either `action_confirm` or `action_close` dismisses it.

### ZMenu_Process

| Property | Type | Notes |
| --- | --- | --- |
| `lbl_text` | `RichTextLabel` | Shows `text`. |
| `process_progress_bar` | `ProgressBar` | Driven between its own `min` and `max`. |
| `update` | `Callable` | Script-only; `() -> float` in 0–1. Set by `ZEvent.Process`. |

`get_process_percent()` is polled every frame; it calls `update` by default, and closes the menu
once it returns `>= 1.0`. Override `_get_process_percent()` in a GDScript subclass to compute the
value yourself instead. Signal `on_progress(percent: float)` fires each frame.

### ZMenu_Choice

| Property | Type | Notes |
| --- | --- | --- |
| `choice_list` | `ZDataList` | Rows come from `ZFlowNode_Choice.get_z_data()`. |
| `choices` | `Array[ZFlowNode_Choice]` | Script-only; set by `ZEvent.Choice`. |

Choices whose `conditions` fail are left out of the list. `on_choice_selected(index)` reports the
index into the array that was **passed in**, not the filtered row, then the menu closes.

---

## Choices in a flow graph

Two `ZFlowNode` types wire the choice menu into [flow graphs](flow.md).

```
… ──▶ [Choice Hub] ──┬──▶ [Choice "Yes, take it"] ──▶ …
                     ├──▶ [Choice "Not now"]      ──▶ …
                     └──▶ [Choice "Who are you?"] ──▶ …
```

**`ZFlowNode_ChoiceHUB`** — on enter, collects every `ZFlowNode_Choice` connected to its `out` port,
drops the unavailable ones, and opens a `ZMenu_Choice`. It stays active until the player picks, then
enters the chosen node.

| Property | Type | Notes |
| --- | --- | --- |
| `override_menu` | `PackedScene` | Root must be a `ZMenu_Choice`; overrides `zandot/menus/menu_choice` for this hub only. |

**`ZFlowNode_Choice`** — one option. Entering it triggers its own `out` immediately, so the branch
behind the choice just runs.

| Property | Type | Notes |
| --- | --- | --- |
| `text` | `String` | The option label (exposed on the graph node). |
| `conditions` | `Array[ZGlobalCondition]` | All must pass or the choice is hidden. |

It implements `get_z_data()`, so `ZDataOption` renders it with no glue code — the same convention
`ZResource` uses (see [utilities.md](utilities.md#zdataoption-control)).

With no available choice the hub warns and stops, ending that branch of the flow.
