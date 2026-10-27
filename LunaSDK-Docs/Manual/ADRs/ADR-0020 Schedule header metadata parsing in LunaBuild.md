## Status
Approved.

## Last updated
2026/10/5

## Background
LunaMetaTool currently parses every reflected header in a target serially inside one build action. Each header constructs a separate Clang AST. On a 20-logical-processor Windows machine, the metadata dependency chain accounts for approximately 145 seconds of a 158-second Debug SDK build. LunaBuild already supports parallel ready-node scheduling, but cannot distribute work inside a target's metadata action.

The requested change is to expose header parsing to LunaBuild while retaining a module-level merge barrier. PCH, shared preambles, finer C++ compilation barriers, timestamp caching, and build-tool configuration changes are outside this decision.

## Decision
Replace the generated target-level `luna.meta` action with one `luna.meta.parse` file action per reflected header and one `luna.meta.merge` action per target. Retain `luna.meta` as the action-configuration name and support the existing standalone tool invocation.

Each Parse action runs LunaMetaTool for exactly one header and writes a versioned JSON intermediate containing its reflected declarations, source identity, language, and target identity. Parse actions have separate outputs and dependency files and use the existing LunaBuild parallelism limit. They depend on the tool, the source header, configuration inputs, and upstream module metadata. Clang's actual included files are recorded for incremental invalidation.

Parsing must not publish or truncate shared generated headers. Each process maps the current target's generated headers to empty virtual files through Clang's file-content mappings, including on a clean build. Existing metadata from another header in the same target must not affect parsing, and upstream generated metadata remains available through the ordinary include paths.

Merge depends on every header intermediate. It validates intermediate versions and identities, consumes headers in stable path order, generates the per-header metadata and target registration files, preserves dependency-based registration ordering, and finally writes the target metadata stamp. Only Merge owns these public generated outputs. C++ compilation and downstream metadata continue to wait for the completed target stamp.

## Impact
LunaBuild can interleave metadata parsing with ordinary compilation and shader work without introducing a separate worker pool or oversubscribing the machine. Intermediate files allow unchanged header parses to be reused when their explicit and discovered dependencies remain current.

The JSON format is an internal, versioned build artifact. It must preserve record, enum, interface, property-offset, inheritance, interface-implementation, and registration-dependency information. Failed parsing prevents Merge and downstream compilation. Merge must reject incompatible or misidentified intermediates rather than emitting partial metadata from them.

More compiler processes increase instantaneous CPU and memory demand, bounded by LunaBuild's existing global task limit. Public reflection declarations, generated file names, target-rule authoring, and Runtime registration behavior remain compatible. Parsing still repeats common C++ includes; eliminating that repeated work is deferred.

## Alternatives considered
An internal LunaMetaTool thread pool would hide work from LunaBuild and require coordination with its global task budget. Header-level graph actions reuse the existing scheduler and cache instead.

A preparation action that truncates generated headers before parsing would add another phase and mutate outputs needed by concurrent or incremental builds. Virtual file mappings provide the parser's placeholders without publishing shared files.

Combining headers into one AST or introducing PCH/preamble reuse may reduce total parse work, but changes parsing semantics and compiler-context management. It is deferred until this change has been measured.

## Remarks
This refines the module-level metadata pipeline described by [[ADR-0002 Introduce LunaMetaTool for C++ reflection metadata]].

## Version history
* **2026/10/5** Approved following the requested Header-level Parse / module-level Merge design.
