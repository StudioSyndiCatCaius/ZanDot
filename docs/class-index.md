# Class Index

Every class registered by the extension, its base type, and where it's documented.

## Singleton

| Class | Base | Docs |
| --- | --- | --- |
| `Z` | `Object` (Engine singleton) | [core.md](core.md#z-engine-singleton) |

## Game flow

| Class | Base | Docs |
| --- | --- | --- |
| `ZGameMode` | `Node` | [core.md](core.md#zgamemode-node) |
| `ZGameState` | `Node` | [core.md](core.md#zgamestate-node) |
| `ZMasterScene` | `Node` | [core.md](core.md#zmasterscene-node) |
| `ZSceneTransit2D` | `Area2D` | [core.md](core.md#zscenetransit2d--zscenetransit3d--portals) |
| `ZSceneTransit3D` | `Area3D` | [core.md](core.md#zscenetransit2d--zscenetransit3d--portals) |
| `ZPlayerStart3D` | `Marker3D` | [core.md](core.md#zplayerstart3d-marker3d) |

## Players & input

| Class | Base | Docs |
| --- | --- | --- |
| `ZPlayer` | `Resource` | [input.md](input.md#zplayer-resource) |
| `ZInputListener` | `Node` | [input.md](input.md#zinputlistener-node) |
| `ZInputCommon` | `Node` | [input.md](input.md#zinputcommon-node) |

## UI

| Class | Base | Docs |
| --- | --- | --- |
| `ZCanvas3D` | `Node3D` | [utilities.md](utilities.md#zcanvas3d-node3d) |
| `ZDataList` | `Control` | [utilities.md](utilities.md#zdatalist-control) |
| `ZDataOption` | `Control` | [utilities.md](utilities.md#zdataoption-control) |

## Menus & global events

| Class | Base | Docs |
| --- | --- | --- |
| `ZEvent` | `RefCounted` (static) | [menus.md](menus.md#zevent-refcounted-static) |
| `ZMenu` | `CanvasLayer` | [menus.md](menus.md#zmenu-canvaslayer) |
| `ZMenu_Confirm` | `ZMenu` | [menus.md](menus.md#zmenu_confirm) |
| `ZMenu_Info` | `ZMenu` | [menus.md](menus.md#zmenu_info) |
| `ZMenu_Process` | `ZMenu` | [menus.md](menus.md#zmenu_process) |
| `ZMenu_Choice` | `ZMenu` | [menus.md](menus.md#zmenu_choice) |

## Movement

| Class | Base | Docs |
| --- | --- | --- |
| `ZMover` | `Node` | [movement.md](movement.md#zmover-node) |
| `ZMoveMode` | `Resource` | [movement.md](movement.md#zmovemode-resource) |
| `ZLagger3D` | `Node3D` | [movement.md](movement.md#zlagger3d-node3d) |
| `ZCameraFollower3D` | `Camera3D` | [movement.md](movement.md#zcamerafollower3d-camera3d) |

## Creatures & abilities

| Class | Base | Docs |
| --- | --- | --- |
| `ZCreature` | `Node` | [creatures.md](creatures.md#zcreature-node) |
| `ZAbility` | `Node` | [creatures.md](creatures.md#zability-node) |
| `ZSkinModel` | `Node` | [creatures.md](creatures.md#zskinmodel-node) |
| `ZCreatureData` | `Resource` | [creatures.md](creatures.md#zcreaturedata-resource) |
| `ZAttribute` | `ZResource` | [creatures.md](creatures.md#zattribute-zresource) |
| `ZLeveler` | `ZResource` | [creatures.md](creatures.md#zleveler-zresource) |

## Saves

| Class | Base | Docs |
| --- | --- | --- |
| `ZSave` | `Resource` | [saves.md](saves.md#zsave-base) |
| `ZSaveGame` | `ZSave` | [saves.md](saves.md#zsavegame) |
| `ZSaveGlobal` | `ZSave` | [saves.md](saves.md) |

## Flow graphs

| Class | Base | Docs |
| --- | --- | --- |
| `ZFlowGraph` | `Resource` | [flow.md](flow.md#zflowgraph-resource) |
| `ZFlowNode` | `Resource` | [flow.md](flow.md#zflownode-resource) |
| `ZFlowPlayer` | `Node` | [flow.md](flow.md#zflowplayer-node) |
| `FlowBasicNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `FlowStartNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `FlowEndNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `ZFlowHubNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `ZFlowToHubNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `ZFlowCheckGlobalConditionsNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `ZFlowDoGlobalScriptNode` | `ZFlowNode` | [flow.md](flow.md#built-in-node-types) |
| `ZFlowNode_ChoiceHUB` | `ZFlowNode` | [menus.md](menus.md#choices-in-a-flow-graph) |
| `ZFlowNode_Choice` | `ZFlowNode` | [menus.md](menus.md#choices-in-a-flow-graph) |

## World query

| Class | Base | Docs |
| --- | --- | --- |
| `ZWorldQuery2D` | `Node2D` | [worldquery.md](worldquery.md) |
| `ZWorldQuery3D` | `Node3D` | [worldquery.md](worldquery.md) |
| `WorldQueryResult` | `Resource` | [worldquery.md](worldquery.md#worldqueryresult-resource) |
| `WorldQueryShape` | `Resource` | [worldquery.md](worldquery.md#shapes) |
| `WorldQueryShape_Grid` | `WorldQueryShape` | [worldquery.md](worldquery.md#shapes) |
| `WorldQueryShape_Donut` | `WorldQueryShape` | [worldquery.md](worldquery.md#shapes) |
| `WorldQueryUtility` | `Resource` | [worldquery.md](worldquery.md#utilities) |
| `WorldQueryUtility_Distance` | `WorldQueryUtility` | [worldquery.md](worldquery.md#utilities) |
| `WorldQueryUtility_Angle` | `WorldQueryUtility` | [worldquery.md](worldquery.md#utilities) |
| `WorldQueryUtility_Trace` | `WorldQueryUtility` | [worldquery.md](worldquery.md#utilities) |

## Audio

| Class | Base | Docs |
| --- | --- | --- |
| `Z_BGM` | `RefCounted` (static) | [audio.md](audio.md#z_bgm-refcounted-static) |
| `ZBgmTrack` | `ZResource` | [audio.md](audio.md#zbgmtrack-zresource) |
| `ZBgmAudio` | `Resource` | [audio.md](audio.md#zbgmaudio-resource) |

## Utilities

| Class | Base | Docs |
| --- | --- | --- |
| `Z_Math` | `RefCounted` (static) | [utilities.md](utilities.md#z_math-refcounted-static) |
| `Z_Obj` | `RefCounted` (static) | [utilities.md](utilities.md#z_obj-refcounted-static--namevars) |
| `Z_Parse` | `RefCounted` (static) | [utilities.md](utilities.md#z_parse-refcounted-static) |
| `Z_Node` | `RefCounted` (static) | [utilities.md](utilities.md#z_node-refcounted-static) |
| `Z_Sound` | `RefCounted` (static) | [utilities.md](utilities.md#z_sound-refcounted-static) |
| `ZGeneralData` | `RefCounted` | [utilities.md](utilities.md#zgeneraldata-refcounted) |
| `ZResource` | `Resource` | [utilities.md](utilities.md#zresource-resource) |
| `ZGlobalCondition` | `Resource` | [utilities.md](utilities.md#zglobalcondition--zglobalscript-resource) |
| `ZGlobalScript` | `Resource` | [utilities.md](utilities.md#zglobalcondition--zglobalscript-resource) |
| `ZDebugRoot2D` | `Node2D` | [utilities.md](utilities.md#zdebugroot2d--zdebugroot3d--zdebugrootui) |
| `ZDebugRoot3D` | `Node3D` | [utilities.md](utilities.md#zdebugroot2d--zdebugroot3d--zdebugrootui) |
| `ZDebugRootUI` | `Control` | [utilities.md](utilities.md#zdebugroot2d--zdebugroot3d--zdebugrootui) |

## Editor-only

Not usable from game code; listed for completeness.

| Class | Base | Purpose |
| --- | --- | --- |
| `FlowEditorPlugin` | `EditorPlugin` | The Flow main-screen tab. |
| `FlowPaletteList` | `ItemList` | Node palette in the Flow editor. |
| `FlowGraphCanvas` | `GraphEdit` | Graph canvas in the Flow editor. |
| `FlowGraphLoader` | `ResourceFormatLoader` | Loads legacy `.gdflow` files. |
| `ZResourcePreviewGenerator` | `EditorResourcePreviewGenerator` | FileSystem previews for `ZResource`. |
| `ZWorldQueryDebugCanvas2D` | `Node2D` | Debug drawing for `ZWorldQuery2D`. |
