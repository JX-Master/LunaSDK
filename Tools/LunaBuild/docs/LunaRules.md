# LunaRules Debug Format v1

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
luna_make 1
```

### Options

```text
option <json-name> <json-value>
```

Options are metadata copied from `BuildOptions`.

### Nodes

```text
node <json-id> <kind> <json-path-or-null>
```

`kind` is one of:

- `file`
- `phony`
- `virtual`

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
luna_make 1
option "mode" "Debug"
target "target://ObjLoader"

node "file://Modules/Luna/ObjLoader/Source/ObjLoader.cpp" file "Modules/Luna/ObjLoader/Source/ObjLoader.cpp"
node "file://build/LunaBuild/Windows/x64/Debug/obj/ObjLoader/ObjLoader.obj" file "build/LunaBuild/Windows/x64/Debug/obj/ObjLoader/ObjLoader.obj"
node "target://ObjLoader" virtual "Modules/Luna/ObjLoader"

action "file://build/LunaBuild/Windows/x64/Debug/obj/ObjLoader/ObjLoader.obj" "cpp.compile" "kind=cpp.compile\ntarget=ObjLoader"
dep "file://build/LunaBuild/Windows/x64/Debug/obj/ObjLoader/ObjLoader.obj" "file://Modules/Luna/ObjLoader/Source/ObjLoader.cpp"
dep "target://ObjLoader" "file://build/LunaBuild/Windows/x64/Debug/obj/ObjLoader/ObjLoader.obj"
```
