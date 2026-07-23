# GUI Icon Generator

`Generate.py` converts a curated set of Phosphor SVG assets into the scalar VG command stream embedded by the
`Luna.GUI` module. The generated application binary does not contain an SVG parser or one C++ function per icon.

The checked-in core pack currently uses `@phosphor-icons/core` revision
`2b75f3ad12b420c9504ef05df8d2564a28f8500e` (package version 2.1.1). Its selected icon names and optional weights are
listed in `Modules/Luna/GUI/Res/PhosphorIcons.json`. The upstream MIT license is copied to
`Modules/Luna/GUI/Res/PhosphorIcons.LICENSE`.

Generate the checked-in files from a local checkout of the upstream repository:

```sh
python3 Tools/GUIIconGen/Generate.py \
    --source /path/to/phosphor-icons-core \
    --manifest Modules/Luna/GUI/Res/PhosphorIcons.json \
    --names-output Modules/Luna/GUI/IconNames.inl \
    --data-output Modules/Luna/GUI/Res/PhosphorCore.bin \
    --revision 2b75f3ad12b420c9504ef05df8d2564a28f8500e
```

The generator accepts path-only SVG assets with a `0 0 256 256` view box. It supports absolute and relative
`M/L/H/V/Q/T/C/S/A/Z` path commands and converts elliptical arcs to cubic Bézier segments. Unsupported SVG elements
or attributes fail generation instead of being silently ignored.

`IconNames.inl` is the public enum member list. `PhosphorCore.bin` contains a versioned header, icon variant and layer
tables, and one contiguous scalar VG command stream. Both files are deterministic for a given manifest and source
revision.
