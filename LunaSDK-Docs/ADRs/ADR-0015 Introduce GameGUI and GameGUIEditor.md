## Status
Proposed.

## Last updated
2026/8/29

## Background
LunaSDK separates its current GUI infrastructure into two layers. `GUI` is the low-level, data-oriented foundation
that owns one typeless element execution tree for each frame, while `EditorGUI` is an immediate API package designed
for editors, DCC tools, office software and other data-oriented desktop applications. This division is defined by
[[ADR-0004 Introduce GUI foundation layer]].

Games, console interfaces and mobile applications need a different high-level model. Their interface structure must
survive across frames, be visually authored, be loaded through the Asset module, support reusable nested content and
allow application modules to add domain-specific node types. Their visual and interaction policies also should not
depend on EditorGUI's compact editor-oriented controls.

The earlier [[ADR-0003 GUI Visual Editing]] proposed `GUIAsset` and `GUIEditor`. That proposal predates the finalized
`GUI`/`EditorGUI` boundary and assumes that asset generation calls high-level EditorGUI widget APIs. It also assigns
one operation stack to the editor process and proposes a custom TCP MCP path. Those choices no longer fit the current
GUI foundation, multi-document requirement, Application as a Service pattern or existing MCP module.

A retained GameGUI tree must not become a second low-level GUI runtime. `GUI::IContext::begin_frame` rebuilds the
typeless element tree, callback records and draw commands every frame, and GUI element handles are frame-local. The
retained tree must therefore be a persistent semantic source that compiles into the sole low-level GUI execution tree.

The initial scope also follows the current GUI interaction and geometry model:

1. GUI selects one interaction stream at a time. Controller and keyboard/pointer modes are not operated concurrently,
   and pointer identifiers do not create independent simultaneous capture or hover state.
2. Canvas layout may place children arbitrarily in two dimensions, but GUI elements remain axis-aligned and do not
   have general rotation or scale.
3. Asset changes are applied by explicit reload. LunaSDK does not watch files or automatically replace live asset
   instances when files change.

## Decision

### Module and dependency boundary
Introduce three build targets grouped into two product components:

1. `GameGUI`, a runtime module under `Modules/Luna/GameGUI`.
2. `GameGUIEditorService`, a headless service library under `Programs/GameGUIEditor/Service`.
3. `GameGUIEditor`, an application under `Programs/GameGUIEditor` that links the service library.

The simplified high-level dependency graph is:

```text
Game and application code  --> GameGUI --> GUI
GameGUIEditorService        --> GameGUI
GameGUIEditor GUI           --> GameGUIEditorService
GameGUIEditor GUI           --> GameGUI
GameGUIEditor GUI           --> EditorGUI --> GUI
CLI/MCP adapter             --> GameGUIEditorService
```

`GameGUI` directly depends on `GUI`, `Asset`, `VFS` and `Runtime`; its loader reads and writes the current cooked
runtime representation. It must not depend on `EditorGUI` or authoring-format compatibility code. `GameGUIEditorService` depends on `GameGUI`,
`Asset`, `Frontend`, `VariantUtils` and `VFS`, but not on `EditorGUI`, `Window` or swap-chain state. The
`GameGUIEditor` application uses EditorGUI for its own editor chrome and GameGUI for previews. An optional protocol
adapter depends on `MCP` to expose selected service operations through standard IO or Streamable HTTP.

GameGUI may own persistent semantic nodes, compiled node data and per-instance state. It must not define another
low-level GUI context, execution tree, layout engine, input router or renderer. Each frame, GameGUI submits one or
more GUI elements for every semantic node that contributes content to that frame.

### GameGUI asset data units
Register one stable GameGUI asset type with the Asset module. Following
[[ADR-0014 Introduce multiple data units to assets]], each GameGUI asset uses two data units with the same asset GUID
and path:

1. The implicit main data unit uses the `Luna.GameGUI.Runtime` loader and stores the cooked runtime document.
2. The named `Authoring` data unit uses the `Luna.GameGUIEditor.Authoring` loader and stores versioned JSON source.

The initial physical-file convention is `<asset>.cooked` for the compact binary main unit and `<asset>.json` for the
human-readable Authoring unit. The cooked stream has a fixed identifying magic but no compatibility version field;
the cooker and runtime codec change together.

The main data unit contains only the latest semantic node representation expected by the matching GameGUI code. It
has no document-format or node-payload version fields, migration callbacks, editor schemas, extension records or
unknown-provider preservation contract. Loading still performs integrity and topology validation. Because runtime
assets and code are cooked and shipped together, incompatible authoring data never reaches this loader.

The `Authoring` unit contains the canonical editor source model:

The canonical document model contains:

1. An authoring document format version.
2. The stable GUID of the root node.
3. A flat collection of node records addressed by stable node GUID.
4. Optional extension data that older editors can preserve without interpreting.

Each node record contains:

1. A stable node GUID that does not change across save and load.
2. A stable node-type GUID and node-type schema version.
3. A human-readable name.
4. A raw `Variant` property object.
5. Ordered child links.

Each child link identifies the child node, an optional named slot and parent-owned layout attachment data. Canvas
anchors, table cells and similar placement policies belong to this parent-child link rather than to the child node's
general properties.

The disk model stores raw property Variants instead of directly serializing polymorphic Runtime `Any` objects. A
known node type may compile that Variant into typed cached data after loading. GameGUIEditor must retain the original
type GUID, version, properties and children for an unknown node type so that opening and saving a document without an
optional node provider does not destroy its data. Runtime instantiation reports a diagnostic when a required node type
is unavailable.

GameGUI owns only the current cooked codec and main-unit loader. GameGUIEditorService owns the authoring codec,
authoring loader, migrations, unknown-record preservation and cooking operation. Saving an editor document writes the
`Authoring` unit only. Cooking validates and migrates a snapshot, rejects unsupported node types, produces a current
`GameGUI::Document`, and writes the main unit. Preview cooking happens in memory and never publishes the dirty preview
snapshot into the main Asset data unit. Source and cooked saves are intentionally independent; cooked data is derived
and can always be rebuilt.

GameGUIEditorService exposes both document-snapshot cooking and a headless asset cooking helper so an asset cooking
pipeline can rebuild the main unit from the loaded Authoring unit. ADR-0014 does not provide a shipping-strip
operation: packaging must deliberately include the main data and emit metadata without the `Authoring` descriptor or
source file. It must not blindly package every path returned by generic asset-file enumeration.

The document owns persistent semantic data only. GUI element handles, callback pointers, RHI pointers, interaction
state, selection, editor history and preview state are never serialized into the asset.

### Runtime node registry and editor schema
GameGUI provides a public runtime node-type registry. Every runtime node type descriptor contains:

1. A stable type GUID and programmatic name.
2. Current-payload validation and optional typed-cache preparation.
3. Per-instance state construction and the frame-generation callback.
4. Direct asset-reference collection.

GameGUIEditorService provides the corresponding authoring node-type registry. Its descriptors contain display name,
category, current authoring payload version, schemas, default properties and stepwise migration. The editor descriptor
refers to the same stable node type GUID but is not linked into runtime-only applications.

User modules may register matching runtime and editor descriptors without modifying either framework. Duplicate stable
type identities are errors rather than silent replacement. Reflection metadata and property attributes may generate
default editor schemas, while custom editor adapters may provide richer controls. Neither schema nor EditorGUI draw
callbacks are stored in the GameGUI runtime registry.

Authoring loading performs raw JSON decoding, document-format migration and generic topology validation. Topology
validation rejects duplicate node GUIDs, missing roots or children, multiple parents and cycles in the document tree.
Only nodes with a registered editor provider and a supported schema version undergo node-type migration. Unknown node
types and nodes newer than the installed provider remain raw records and do not prevent lossless editor round trips.
Cooking rejects them, so runtime instance preparation receives only current payloads.

### Nested GameGUI assets
A built-in asset-instance node stores the stable GUID of another GameGUI asset as a canonical string in its
Variant/JSON record.
During typed-cache construction and dependency resolution, GameGUI resolves that GUID to an `Asset::asset_t`; the raw
Variant never stores the process-local asset handle. The referenced asset remains separate and is not copied into the
parent document.

The initial design treats a mounted asset as a closed subtree and does not provide cross-asset property overrides.
The same asset may be mounted more than once. A single-document decode cannot detect an indirect cross-asset cycle.
After referenced assets have been explicitly loaded, dependency resolution or instance preparation traverses the
loaded document graph and rejects direct or indirect cycles with a diagnostic that identifies the complete mount
chain. This traversal performs no file I/O.

GameGUI registers direct referred-asset enumeration for nested GameGUI assets and for resources reported by node type
descriptors. The Asset module does not recursively load this graph. Hosts explicitly load or reload the required
assets before instance generation; GameGUI does not synchronously load dependencies from the render or GUI-build path.

Asset refresh is explicit. `Asset::load_asset_data_unit(asset, Name(), true)` or an equivalent host operation replaces the registered
asset data, after which the host rebuilds or rebinds affected GameGUI instances. Automatic file watching, implicit
reload, asset revision notifications and live instance patching are outside the initial design.

### Instance and frame generation
A GameGUI instance refers to an asset document or an equivalent in-memory semantic document and owns all mutable
per-instance state. Mutating one instance must not mutate a shared loaded asset definition or another instance.

Every frame after `GUI::IContext::begin_frame`, the instance traverses the complete relevant semantic tree and emits
GUI elements. It may cache parsed properties, resolved node descriptors, immutable resource data and other source-level
compilation results, but it does not retain GUI element handles or patch the previous frame's element tree.

The frame contract has explicit phases:

1. The host begins the GUI frame, queues input and asks each GameGUI instance to build its elements.
2. The host performs the normal GUI layout and input-routing passes.
3. The instance resolves high-level interactions from the routed GUI state, returns symbolic actions to the host and
   may request another layout pass after non-structural state changes such as scrolling.
4. The host performs any requested relayout, then generates draw commands and renders through GUI.

GameGUI does not begin the context frame, route input or invoke the renderer on the host's behalf. Frame-local root
handles and sidecar mappings may exist between these phases, but they are discarded before the next `begin_frame`.

Runtime GUI identities are derived from:

1. A caller-provided root instance scope.
2. The complete nested-asset mount-node path.
3. The asset-local stable node GUID.
4. A node-generator-defined element role such as `owner`, `content`, `label` or `thumb`.

This identity path lets one node emit multiple GUI elements and lets the same asset be mounted repeatedly without
duplicate core IDs. GameGUI keeps frame-local sidecar mappings between semantic node instances and generated GUI
element IDs for event resolution, diagnostics and editor selection.

GameGUI resolves its high-level interactions after `GUI::IContext::route_input`. Asset data stores symbolic event or
action identifiers rather than callback pointers. The host binds those identifiers to application behavior. Structural
changes requested by an interaction take effect when the next frame is built; they do not mutate the already-routed
GUI tree or callback userdata of the current frame.

GameGUI follows GUI's selected single-input-stream policy. It does not add concurrent multi-pointer capture, simultaneous
controller and keyboard/pointer operation, or a gesture system on top of the current router. A host may map one touch
contact into the existing pointer stream; GameGUI does not require a concurrent-touch redesign.

### Layout and transformation scope
GameGUI reuses GUI layout primitives. Canvas layout supports arbitrary axis-aligned child placement. The initial
GameGUI model does not provide general per-element rotation or scale and does not emulate those operations only in
drawing, because layout, clipping, hit testing and interaction would disagree with the rendered result.

If transformed subtrees become a demonstrated requirement, they must be designed as an explicit specialized layout
type that owns transform-aware layout, clipping, hit testing and rendering behavior. General transform fields are not
added to every GUI element or every GameGUI node.

### GameGUIEditor service
GameGUIEditor follows the Application as a Service pattern. The `GameGUIEditorService` library implements document and
editing behavior before the GUI adapter. The service has no dependency on EditorGUI, Window or the application swap
chain.

Each open document has one service-owned document ID and independent:

1. Editable GameGUI working copy.
2. Monotonic document revision.
3. Undo/redo history and current history state.
4. Saved history state and dirty status.
5. Validation diagnostics.

The service provides create, open, save, save-as and close operations. An untitled document has a document ID but no
asset binding until save-as succeeds. Closing a dirty document without saving requires an explicit discard request;
the service never infers that choice from GUI state.

Opening an asset that is already open returns its existing document ID rather than creating another mutable copy; the
GUI adapter decides which tab becomes active. The editor may keep several different documents open. Dirty state is
determined by whether the current history state equals the saved history state, so undoing back to the save point
restores a clean document.

Opening loads and copies the `Authoring` data unit into service-owned editable state. Frontend queries expose immutable
snapshots tagged with the document revision for preview generation; no adapter receives the mutable working copy.
Edits and source saves do not mutate GameGUI definitions already published in the main data unit. Save writes the
`Authoring` unit and advances the document's save point. An explicit cook writes the main unit; runtime users adopt the
new definition only through the explicit reload and instance
rebuild/rebind path. This keeps live preview responsive without introducing implicit runtime hot reload.

Edits are expressed as semantic commands addressed by document ID and stable node GUID. Structural edits, property
changes and batches validate an expected document revision. Continuous pointer drags and text input may coalesce into
one committed history item. `VariantUtils::diff` may be used internally to construct inverse data, but serialized diffs
are not the public editing protocol.

Document revision and history state are distinct. The revision is a concurrency token that increases on every accepted
edit, undo, redo or save-state transition and never moves backward. History state identifies the current content
position, may move in either direction and may branch after an undo.

The service exposes operations through one `Frontend::IFrontend` instance using stable resource URLs and Variant
parameters. Dynamic document IDs are parameters rather than parts of registered URLs. GUI actions invoke those
Frontend functions instead of mutating document objects directly. Automation uses the same functions. Access to the
service and Frontend is serialized on the owning thread.

The existing MCP module is the protocol shell when MCP access is requested. Standard IO serves CLI-only mode. A GUI
process may service the existing loopback Streamable HTTP server with `poll(0)` and a bounded number of iterations per
frame. This makes the readiness wait non-blocking, but Frontend handlers still execute synchronously and can stall the
frame; operations exported to the GUI-hosted MCP server must therefore have bounded execution time. Long-running or
cancellable work requires a future asynchronous job contract. GameGUIEditor does not implement a separate TCP MCP
protocol.

### GameGUIEditor GUI and preview
The GUI application uses EditorGUI DockSpace or tabs for multiple open documents. Its hierarchy, node palette,
property inspector and validation views consume GameGUIEditor authoring node schemas rather than hard-coded knowledge of built-in
node C++ types. All mutations pass through the service command surface.

Each open document owns independent preview state. When its preview is visible, the GUI adapter lazily creates or
resumes an independent `GUI::IContext`, GameGUI instance and offscreen render target, displays that target inside
EditorGUI and maps viewport input into the preview context. Only visible previews need to render; inactive targets may
be released or pooled. Separate contexts isolate focus, capture, popup layers, styles and stable IDs between documents
and between the preview and editor chrome. When a document revision changes, the adapter obtains the corresponding
immutable snapshot through the Frontend query surface and rebinds the preview instance; it never receives a mutable
service-owned document pointer.

## Impact
Expected benefits:

1. Games and mobile applications gain a retained, asset-backed GUI model without coupling to EditorGUI visuals.
2. GUI remains the only low-level execution, layout, input and rendering runtime.
3. Stable node identities and editor-only schemas support visual editing, automation, migration and custom providers
   without increasing runtime registry or asset size.
4. Nested assets allow reusable interface composition without copying source trees.
5. AaaS gives the GUI, CLI and MCP adapters one tested editing behavior.
6. Per-document history and save points make simultaneous asset editing predictable.

Costs and risks:

1. The semantic tree and generated GUI tree are intentionally different representations and require a compiler,
   identity mapping and diagnostics.
2. Complete frame generation has CPU cost and needs large-tree benchmarks and frame-storage discipline.
3. Maintaining separate authoring and cooked representations requires explicit cooking and stale-output diagnostics.
4. Nested assets require explicit dependency loading and cycle diagnostics because Asset does not provide them.
5. Separate preview contexts and render targets increase editor GPU memory and require a dedicated input bridge.
6. The selected single-input-stream model does not support concurrent touch points or simultaneous input modes.
7. Axis-aligned elements cannot express rotated or scaled subtrees without a future specialized layout design.
8. Explicit reload does not update instances until the host performs the reload and rebuild/rebind operation.

## Alternatives considered

### Implement game interfaces with EditorGUI
* Status: rejected.
* Pros:
    1. Reuses the existing widget package without a new runtime module.
* Cons:
    1. EditorGUI's immediate API, density and rendering policy target editor and data-oriented desktop software.
    2. It does not provide a persistent asset-authored semantic tree.
    3. Game-specific visuals and composition would accumulate inside the wrong package.

### Make GameGUI another immediate API package
* Status: rejected as the primary model.
* Pros:
    1. Closely follows EditorGUI's implementation pattern.
    2. Avoids maintaining a persistent semantic tree.
* Cons:
    1. Cannot directly provide asset-authored hierarchy, stable node identity or multi-document visual editing.
    2. Moves persistent state and composition back into application code.

### Make retained GameGUI nodes the low-level GUI runtime
* Status: rejected.
* Pros:
    1. Uses one object tree for authoring and execution.
* Cons:
    1. Reintroduces widget-specific node types into the primitive GUI foundation.
    2. Duplicates or bypasses GUI's context, layout, input, state and rendering responsibilities.
    3. Makes the asset schema change at the pace of low-level execution internals.

### Serialize GUI element records directly
* Status: rejected.
* Pros:
    1. Avoids a separate generation step.
* Cons:
    1. GUI elements and callback attachments are frame-local execution data.
    2. Element handles, runtime pointers and draw data are not stable asset content.
    3. The format would expose private core changes to artists and editor tools.

### Serialize polymorphic node objects directly through Runtime Any
* Status: rejected for the canonical disk model.
* Pros:
    1. Known node types deserialize directly into typed C++ objects.
* Cons:
    1. Missing node providers make unknown type GUIDs impossible to materialize.
    2. An editor could not open and save unknown nodes without losing data.
    3. Explicit raw-property migration is clearer across node-type versions.

### Add general transform fields to every GUI element
* Status: rejected.
* Pros:
    1. Makes arbitrary rotation and scale available to every node.
* Cons:
    1. Complicates hit testing, clipping, culling and layout semantics.
    2. Enlarges hot element and rendering data for a capability most interfaces do not require.
    3. A future specialized layout can keep the cost and semantics local to transformed subtrees.

### Add concurrent multi-pointer and mixed input modes as a GameGUI prerequisite
* Status: rejected for the initial scope.
* Pros:
    1. Supports simultaneous touch gestures and concurrent input sources.
* Cons:
    1. Requires a broader GUI input-router and capture-state redesign.
    2. The current product interaction model intentionally selects one controller or keyboard/pointer stream.

### Add automatic asset watching and live instance patching
* Status: rejected for the initial scope.
* Pros:
    1. External file changes appear without an explicit reload operation.
* Cons:
    1. Requires generic Asset revision, notification, file-watching and instance migration contracts.
    2. Explicit reload is sufficient for the initial runtime and editor workflow.

### Let the GUI adapter mutate documents directly
* Status: rejected.
* Pros:
    1. Requires less initial service code.
* Cons:
    1. Duplicates behavior across GUI, CLI and automation paths.
    2. Makes validation, history, revision checks and headless testing UI-dependent.
    3. Conflicts with the Application as a Service pattern.

### Implement a custom TCP MCP server in GameGUIEditor
* Status: rejected.
* Pros:
    1. Matches the transport wording in the retired [[ADR-0003 GUI Visual Editing]] proposal.
* Cons:
    1. Duplicates the existing MCP, HTTP and standard IO modules.
    2. Creates another protocol and lifecycle implementation to maintain.

## Remarks
This ADR supersedes the retired [[ADR-0003 GUI Visual Editing]].
[[ADR-0004 Introduce GUI foundation layer]] remains authoritative for GUI core responsibilities and has been revised
to distinguish GameGUI's persistent semantic tree from GUI's sole low-level per-frame execution tree.

Manual programming documentation for GameGUI and GameGUIEditor is intentionally deferred until the main application
framework, service boundary and preview lifecycle are sufficiently stable. This proposal and its implementation tests
define the initial contract during that period.

## Version history
* **2026/8/29** Renumbered to ADR-0015 after ADR-0014 was assigned to Asset data units; split each GameGUI asset into
  an editor-owned versioned `Authoring` unit and a latest-only cooked main unit.
* **2026/8/27** Renumbered from ADR-0012 to ADR-0014 after restoring the approved application-menu and message-box ADR numbers.
* **2026/8/25** Proposed.
