# LunaRules Debug Format v2

LunaRules is a textual dump of the LunaBuild build graph. It is meant for
debugging, review, repro logs, and future protocol experiments. It is not the
primary connection between LunaBuild front-end and back-end.

The normal in-process pipeline is:

```text
LunaBuild front-end -> BuildGraph object -> C# MakeSystem backend
```

`BuildRuleFileWriter` serializes the same `BuildGraph` object only when a user
asks for `generate` output or a `build --output` debug dump.

## Non-Goals

- No glob patterns.
- No variables.
- No conditionals.
- No includes.
- No rule inference.
- No requirement that MakeSystem parses this file before every build.

## Encoding

- UTF-8 text.
- `LF` and `CRLF` are accepted by readers.
- Blank lines are ignored.
- Lines whose first non-whitespace character is `#` are comments.
- String values are JSON strings.

## Directives

### Header

```text
luna_make 2
```

Readers accept v1 for compatibility. New multi-project graphs are emitted as
v2.

### Options

```text
option <json-name> <json-value>
```

Options are metadata copied from `BuildOptions`. Project-defined build
properties are written as `property.<name>` options.

### Projects And Configurations

```text
project <json-name> <json-root> <json-build-directory> <is-host>
configuration <json-id> <json-project-name> <json-options>
```

Every project has a canonical source root and a distinct build directory.
Configuration IDs are stable hashes of the project configuration that owns the
nodes. `json-options` is a JSON string containing the serialized `BuildOptions`
record.

### Nodes

```text
node <json-id> <kind> <json-path-or-null>
node_config <json-id> <json-project-name> <json-configuration-id>
```

`kind` is one of:

- `file`
- `phony`
- `virtual`

`node_config` records which project and configuration own a node. It is emitted
for configured v2 nodes and omitted for compatibility-only nodes.

### Actions

```text
action <json-node-id> <json-action-kind> <json-payload>
```

The action payload is the canonical action description used by the C# MakeSystem
cache. It is not required to be a shell command.

### Edges

```text
dep <json-node-id> <json-dependency-id>
order <json-node-id> <json-dependency-id>
output <json-node-id> <json-output-id>
depfile <json-node-id> <json-depfile-id>
```

All declared file relationships are expressed through nodes and edges. There is
no separate `inputs` list.

### Targets

```text
target <json-node-id>
```

At least one target is expected in a complete dump.

## Example

```text
luna_make 2
option "mode" "Debug"
project "Host" "/src/host" "/src/host/build/LunaBuild" true
configuration "41a7d3194584" "Host" "{\"Platform\":\"MacOS\"}"
target "target://Host.ObjLoader"

node "file://Modules/Luna/ObjLoader/Source/ObjLoader.cpp" file "Modules/Luna/ObjLoader/Source/ObjLoader.cpp"
node "file://build/LunaBuild/MacOS/arm64/Debug/obj/ObjLoader/ObjLoader.o" file "build/LunaBuild/MacOS/arm64/Debug/obj/ObjLoader/ObjLoader.o"
node "target://Host.ObjLoader" virtual "Modules/Luna/ObjLoader"
node_config "target://Host.ObjLoader" "Host" "41a7d3194584"

action "file://build/LunaBuild/MacOS/arm64/Debug/obj/ObjLoader/ObjLoader.o" "cpp.compile" "kind=cpp.compile\ntarget=Host.ObjLoader"
dep "file://build/LunaBuild/MacOS/arm64/Debug/obj/ObjLoader/ObjLoader.o" "file://Modules/Luna/ObjLoader/Source/ObjLoader.cpp"
dep "target://Host.ObjLoader" "file://build/LunaBuild/MacOS/arm64/Debug/obj/ObjLoader/ObjLoader.o"
```
