## Status
Retired.

## Last updated
2026/8/25

## Background
Currently LunaSDK has its own GUI module, but all GUI interfaces are hard-coded in C++. Since now we need to offer our new GUI module to UX designers, we need to offer a new visual GUI design tool and its runtime representation so that UX designers can design GUI interface visually without knowing the real C++ code behind.

## Decision
This proposal was never approved and is now retired and superseded by [[ADR-0012 Introduce GameGUI and GameGUIEditor]]. The former
`GUIAsset`/`GUIEditor` naming and the assumption that asset generation should call `EditorGUI` APIs no longer match
the finalized `GUI`/`EditorGUI` boundary established by [[ADR-0004 Introduce GUI foundation layer]]. The text below
is retained as the historical proposal.

We introduce two new targets in LunaSDK: `GUIAsset` and `GUIEditor`.

### GUIAsset
`GUIAsset` is a new runtime module that depends on `GUI` module, it provides the following features:
1. A new `GUIAsset` type, which stores the GUI design file created by UX designer.
2. `GUIAsset` is registered to `Asset` module as an asset, so that it can be loaded and managed as other assets. One `GUIAsset` can refer other `GUIAsset` using `asset_t`. 
3. `GUIAsset` uses its own node types to maintain the editable widget tree, not uses GUI description node types defined in GUI module. 
4. Every node in `GUIAsset` must have a stable ID generated when the node is created and will not change after saving/loading.
5. GUI asset node types are registered to GUIAsset runtime module, user module can also define and register new GUI asset types.
6. GUI asset node supports mounting another `GUIAsset` as its sub-node, it calls `generate` of the referred node to generate child content.
7. `GUIAsset` provides a `generate` function, which calls GUI widget APIs to build runtime GUI description tree based on the GUI asset data.
8. GUI asset node type descriptors include a design-time property schema. Visual editors, MCP clients and other external tools should use this schema to inspect and edit widget-specific properties instead of hard-coding GUIEditor-only knowledge.
9. `GUIAsset` can be saved and loaded as JSON files.

### GUIEditor
`GUIEditor` is a new application target in `Programs`, it provides a one-stop visual editing environment for `GUIAsset`. `GUIEditor` has two modes: CLI mode and GUI mode. `GUIEditor` uses `Frontend` to expose its services, CLI and GUI are two application interfaces that interact with the service. Whether GUIEditor is launched using GUI or CLI mode, it uses only one process to contain both the service and the application interface.

The services of `GUIEditor` include:
1. Create, open, save `GUIAsset` files.
2. Edit and inspect widget trees contained by `GUIAsset` file.
3. Every operation to `GUIAsset` file is stored as an `GUIEditOp`, which supports `undo` and `redo` methods. The editor maintains one operation stack that tracks all operations since file load. `undo` and `redo` is virtual in `GUIEditOp` base class, every concrete operation derived from `GUIEditOp` can override `undo` and `redo` to match its own editing logic.
4. Render image of the current GUI widget tree.
5. Supports MCP server via TCP. MCP server should be started if `--mcp` is specified in program startup parameters. Use `Network` and `Frontend` module to implement MCP.

In CLI mode, the editor behaves like a server process, and MCP is the only way to interact with the editor. In GUI mode, the editor behaves like a normal GUI application, and MCP still works if `--mcp` is specified. The CLI mode is activated if `--cli` is specified when starting the program, if this is not specified, GUI mode will be used.
## Impact
`GUIAsset` and `GUIEditor` are fundamental components for most other GUI-based applications and games. It simplifies GUI development process a lot and makes `GUI` usable in most serious projects.

## Alternatives considered

### Stores GUI description node directly as asset data
* Status: rejected
* Pros: We don't need to invent a new GUI node tree any sync between two trees.
* Cons: 
	1. We already decided to make concrete GUI description node invisible outside of GUI module. Doing so will force us to bring GUI node types back to visible outside of the module, which breaks the rule that **widget APIs shall be the only stable and ground-truth for widget building**.
	2. GUI design data and runtime data will not be 100% match at most cases, the runtime data structure version will change rapidly, but the design data is much slower. Use one schema to describe both design data and runtime data will be troublesome.

## Version history
* **2026/8/25** Retired this proposal in favor of [[ADR-0012 Introduce GameGUI and GameGUIEditor]], which introduces `GameGUI` and `GameGUIEditor` on the finalized GUI foundation.
* **2026/6/9** Proposed.
