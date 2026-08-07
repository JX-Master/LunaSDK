## Status
Proposed.

## Last updated
2026/8/4

## Background
LunaSDK applications currently live in the LunaSDK repository and are discovered together with LunaSDK targets from one repository root. This couples every application checkout to one SDK checkout, duplicates SDK source across applications, and prevents an application from selecting or switching an SDK by path.

LunaBuild also assumes one project in several independent subsystems. `BuildWorkspace` combines the source root, build root, rule compilation root, tool root, and SDK root. Project rules are merged into one property set, targets are identified only by unqualified names, `BuildGraph` contains one global `BuildOptions` value, and several executors infer LunaSDK-specific tools and directories from the workspace root. Merely scanning another directory would therefore leak configuration between projects, create ambiguous targets, and write imported-project outputs into the imported source tree.

The required model has one host project and zero or more imported projects. Each project owns its rules, properties, options, targets, and source paths. The host selects and configures imports explicitly. Targets from all reachable projects form one dependency graph and are built by one MakeSystem invocation. By default, imported-project build outputs belong to the host build tree, while imported source trees remain read-only.

## Decision
LunaBuild will introduce a build-session model with first-class projects and project configurations.

### Project discovery and import lifecycle

Every project root contains exactly one concrete `ProjectRules` type declared by root-level `*.Project.cs` files. The rule's declared project name and the canonical project root must both be unique in a build session. Project names and target names cannot contain `.`, a path separator, or `..`.

The host project is discovered from `--root` or an ancestor of the current directory. Its command-line options and property overrides are resolved before imports are configured. A project rule can import another project by calling `Project.ImportProject(path)` during `ConfigureProject`. A relative import path is resolved from the importing project's source root. The returned project exposes its property definition, default options, primary options, and build directory so the importer can map configuration explicitly.

Imports use a two-phase lifecycle:

1. Load the project's unique root rule and property schema without configuring targets.
2. Resolve the host command-line configuration.
3. Execute the importing project's `ConfigureProject` method.
4. Let the importer explicitly assign every imported project's primary options and optional build directory.
5. Freeze the imported configuration, then recursively execute its `ConfigureProject` method.
6. Freeze the import tree and instantiate fresh rules for each project configuration.
7. Discover targets and generate one graph from the selected host roots.

Importing the same canonical root or the same project name more than once is an error in v1. Cycles and diamond imports report the complete conflicting import chains. A project can reference targets in itself and its complete import subtree, but not an ancestor or sibling project.

### Configuration isolation and compatibility

Every project owns a complete `BuildOptions` value, including platform, architecture, mode, linkage, RHI, project properties, global defines, global undefines, and global include directories. No value is inherited across an import boundary. The importer must map values intentionally, including values that are commonly kept equal for ABI compatibility.

Project-local target references use an unqualified target name. Cross-project references use `project_name.target_name`. An unqualified CLI target selects only a host target; an external target must be selected with its qualified name. `--all` and category selection choose host roots only, while their dependency closure can include imported targets.

Before creating an ordinary native link edge, LunaBuild verifies that the producer and consumer use the same normalized platform, architecture, mode, linkage, and RHI configuration. Header-only, external, tool, resource, and order-only relationships may cross configurations. Project-specific properties do not participate in the generic compatibility check; a target must publish any property that affects its ABI as an explicit usage requirement.

Named project configurations and explicit option transitions may create host-tool variants for cross-compilation. A project/configuration/target tuple, rather than a target name alone, is the internal identity of a target.

### Build graph and execution

BuildGraph v2 adds project and project-configuration tables. Every actionable node records its project and configuration identity. MakeSystem obtains options from the node's configuration instead of the host's global options. Node IDs, target IDs, action identities, and cache keys include project and configuration identity.

Target resolution fails for unknown or unsupported dependencies instead of silently omitting them. Two actions cannot produce the same path. Duplicate node IDs are accepted only when they describe the same read-only file node; otherwise graph construction fails.

MakeSystem's dependency validation, ready-queue scheduling, incremental execution, and cleaning traversal remain shared across the complete graph.

### Source roots, build roots, and output layout

The host keeps its existing default output layout. An imported project defaults to a namespaced directory under the host build root:

```text
<host>/build/LunaBuild/Projects/<project>-<canonical-root-hash>/
  Rules/
  <platform>/<architecture>/<mode>/<configuration-hash>/
    obj/
    generated/
    bin/
    dotnet/
    package/
```

An importer can override an imported project's build directory with a normalized absolute path before configuration is frozen. Two projects cannot register the same build directory in one session. All generated rules, objects, generated source, binaries, managed output, response files, package staging, and caches use registered build roots. Source files, public headers, resource files, and prebuilt libraries remain rooted in their owning project's source tree.

Clean and package operations can only delete paths inside registered build roots. Install rejects source mappings containing `..` and reports destination collisions. Runtime libraries are staged beside consuming executables; two different files cannot silently stage to the same destination.

### Generic tool and resource binding

LunaBuild Core does not assign special meaning to the names `LunaSDK`, `Modules`, `CPPSL`, or `LunaMetaTool`. Projects publish global include directories and named build-action configurations. An action configuration binds an executable target or an existing executable file plus any resource files or directories that affect the action.

Shader and metadata graph generation consumes those bindings. Tool executables and resources are ordinary graph inputs, so rebuilding or switching a tool invalidates the corresponding action. LunaSDK project rules are responsible for publishing its module include root, CPPSL compiler and native extractor, CPPSL standard library, shader backends, LunaMetaTool, and LLVM resources.

`TargetBuildOutputs` formally distinguishes the primary executable, native link input, and runtime files so tools and consumers do not infer outputs from filenames.

### Compatibility and delivery

The existing `ConfigureProperties(BuildWorkspace)` and `Configure(BuildWorkspace, BuildOptions)` overrides remain available for one compatibility period and are invoked by the new lifecycle. Existing in-repository Programs remain in place during v1.

Build, run, clean, install, package, inspect, compile-commands generation, and IDE generation accept qualified targets. Generated IDE tasks record the actual LunaBuild runner project path and the host `--root`; they do not assume the host contains `LunaBuild.csproj`.

This ADR changes how ADR-0002's LunaMetaTool is located and scheduled, but does not change its reflection metadata design.

## Impact
Applications can live in independent repositories, select an SDK checkout by path, and share that checkout without writing build artifacts into it. Multiple projects can define the same local target or property name without collision, and their rules are loaded in isolated assemblies.

The main cost is a broader internal identity model and migration of path ownership from a global workspace to projects and configurations. Graph dumps and caches move to version 2. IDE generators must handle files outside the host root. Project authors must explicitly map imported configurations and declare tool bindings that LunaBuild previously inferred.

Rejecting duplicate imports prevents diamond dependency sharing in v1. Supporting multiple aliases or configurations of one canonical project will require a later decision that defines instance identity and output sharing.

## Alternatives considered
### Scan external target files into the existing workspace
This was rejected because it retains one property set, one options value, one source/build root, and unqualified target identities. It cannot provide configuration isolation or prevent writes to imported source trees.

### Build each project in a separate LunaBuild invocation
This was rejected because it loses one dependency graph, cross-project incremental scheduling, tool dependencies, usage-requirement propagation, and reliable runtime staging. It also makes cross-project compatibility failures late linker errors.

### Implicitly inherit host options in every import
This was rejected because it violates project isolation and prevents importers from making configuration transitions intentionally. Examples and helpers may show explicit one-to-one mappings, but Core will not apply them automatically.

### Treat LunaSDK as a special SDK package
This was rejected because external-project support is intended to be generic. SDK location, public includes, tools, resources, and feature properties belong in LunaSDK rules rather than LunaBuild Core.

## Remarks
The LunaBuild front-end's own .NET `bin` and `obj` directories are produced before the imported-project graph runs and are outside this graph's source-tree write guarantee.

Shader/metadata executor plugins, multiple aliases of one imported project, and concurrent sharing of an explicitly overridden build directory are outside v1 scope.

## Version history
* **2026/8/4** Proposed.
