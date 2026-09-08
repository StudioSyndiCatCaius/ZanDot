# Audio

One-shot sound effects live in [utilities.md](utilities.md#z_sound-refcounted-static) (`Z_Sound`).
This page covers **game music**: `Z_BGM` and the two resources it plays.

## The shape of it

```
ZBgmTrack  (ZResource)   ← what you hand to Z_BGM.Play()
 ├─ audio_default : ZBgmAudio
 └─ audio_named   : Dictionary[StringName, ZBgmAudio]   ← alternate arrangements

ZBgmAudio  (Resource)    ← one stream + its loop behaviour
```

`Z_BGM` is static, like `Z_Sound` — there is no node to place. The first call creates a hidden
player under the root viewport, so music keeps going across scene changes and while the tree is
paused.

## Z_BGM (RefCounted, static)

```gdscript
Z_BGM.Play(bgm: ZBgmTrack, start_time := 0.0, fade_previous := true,
           fade_duration := 0.5, force_restart := false, variant := &"") -> bool
Z_BGM.Stop(fade := true, fade_duration := 1.0) -> bool
Z_BGM.Lock(locked: bool)
Z_BGM.RestoreFromSave() -> bool
```

| | |
| --- | --- |
| `start_time` | Seconds into the stream to begin at. |
| `fade_previous` | Crossfade with whatever is already playing (over `fade_duration`). With nothing playing, the new track starts at full volume immediately. |
| `force_restart` | Playing the track that is **already** playing does nothing — unless this is set. |
| `variant` | Picks an entry from the track's `audio_named`. Empty (the default) uses `audio_default`. |

`Play` returns `false` when it did nothing: locked, no playable audio, or an unforced replay of the
current track.

Read-only helpers: `is_playing()`, `is_locked()`, `get_current_track()`, `get_current_variant()`,
`get_position()`. Every method also has a snake_case alias (`play`, `stop`, `lock`,
`restore_from_save`).

```gdscript
Z_BGM.Play(preload("res://res/bgm/bgm_overworld.tres"))

# Same track, "combat" arrangement, hard cut:
Z_BGM.Play(bgm_overworld, 0.0, false, 0.0, true, &"combat")

Z_BGM.Stop(true, 2.0)      # 2-second fade out
```

### Lock

```gdscript
Z_BGM.Lock(true)     # boss theme cannot be stopped or changed
…
Z_BGM.Lock(false)
```

While locked, `Play` and `Stop` are no-ops and return `false`. Useful for a cutscene or boss track
that ambient triggers must not clobber. `RestoreFromSave` drops the lock, restores, then re-applies
whatever lock state the save held.

## ZBgmTrack (ZResource)

| Property | Type | Meaning |
| --- | --- | --- |
| `audio_default` | `ZBgmAudio` | Plays when no `variant` is given. |
| `audio_named` | `Dictionary[StringName, ZBgmAudio]` | Alternate arrangements — `&"combat"`, `&"muffled"`, … |

Being a `ZResource`, it also has `title` / `icon` / `description` for the FileSystem dock preview.

Helpers: `get_audio(name := &"")` (named entry, falling back to `audio_default`) and
`has_audio(name)`.

## ZBgmAudio (Resource)

| Property | Type | Default | Meaning |
| --- | --- | --- | --- |
| `sound_source` | `AudioStream` | — | The stream to play. |
| `looping` | `bool` | `true` | Restart instead of stopping when playback runs out. |
| `loop_start` | `float` | `0.0` | Where a wrap jumps back to. Skipped on the first play. |
| `loop_end` | `float` | `0.0` | Wrap point, in seconds. `0` (or `<= loop_start`) means "wrap at the end of the stream". |
| `loop_blend_time` | `float` | `0.0` | Crossfade length for the wrap. `0` = hard seek. |

Loop points are driven by ZanDot, not by the `AudioStream`'s own loop flag, so the same imported
file can back several tracks with different intros and loop regions. With `loop_blend_time > 0` the
blend *ends* at `loop_end`: a second voice starts at `loop_start` and fades in while the outgoing
one fades out.

An intro-then-loop track, for example, is `loop_start = 8.0` with `loop_end` at the end of the
musical phrase — the 8-second intro plays once and never returns.

## Saves

`Z_BGM` mirrors its state onto `Z.save_game` as it changes:

| `ZSaveGame` property | |
| --- | --- |
| `bgm_track` | `ZBgmTrack` currently playing (null when stopped) |
| `bgm_variant` | `StringName` variant in use |
| `bgm_locked` | lock state |

So a slot written mid-boss-fight remembers the boss theme. After loading, resume it with:

```gdscript
Z.SaveGame_Load_FromSlot(0)
Z_BGM.RestoreFromSave()
```

Because `bgm_track` is a resource reference, the track `.tres` is referenced by path in the save
file — keep tracks as saved resources rather than building them in code if you want this to survive.

## Project settings

| Setting | Default | Meaning |
| --- | --- | --- |
| `zandot/audio/bgm_bus` | `"Master"` | Audio bus the music voices route to. Falls back to `Master` if the named bus does not exist. |
