#!/usr/bin/env python3
"""Generates Luna GUI icon data from the Phosphor Core SVG assets."""

import argparse
import json
import math
import pathlib
import re
import struct
import xml.etree.ElementTree as ET


TOKEN_PATTERN = re.compile(r"[AaCcHhLlMmQqSsTtVvZz]|[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?")
PARAMETER_COUNTS = {
    "A": 7,
    "C": 6,
    "H": 1,
    "L": 2,
    "M": 2,
    "Q": 4,
    "S": 4,
    "T": 2,
    "V": 1,
    "Z": 0,
}

COMMAND_MOVE_TO = 1.0
COMMAND_LINE_TO = 2.0
COMMAND_CURVE_TO = 3.0
COMMAND_CUBIC_TO = 12.0
WEIGHTS = ("regular", "bold", "fill", "duotone")
INVALID_INDEX = 0xFFFFFFFF
MAGIC = b"LGUIICON"
FORMAT_VERSION = 1


def _vector_angle(ux, uy, vx, vy):
    dot = ux * vx + uy * vy
    det = ux * vy - uy * vx
    return math.atan2(det, dot)


def _arc_to_cubics(start, end, rx, ry, rotation, large_arc, sweep):
    x1, y1 = start
    x2, y2 = end
    rx = abs(rx)
    ry = abs(ry)
    if rx <= 1.0e-8 or ry <= 1.0e-8 or (abs(x1 - x2) <= 1.0e-8 and abs(y1 - y2) <= 1.0e-8):
        return []

    phi = math.radians(rotation % 360.0)
    cos_phi = math.cos(phi)
    sin_phi = math.sin(phi)
    dx = (x1 - x2) * 0.5
    dy = (y1 - y2) * 0.5
    x1p = cos_phi * dx + sin_phi * dy
    y1p = -sin_phi * dx + cos_phi * dy

    scale = x1p * x1p / (rx * rx) + y1p * y1p / (ry * ry)
    if scale > 1.0:
        scale = math.sqrt(scale)
        rx *= scale
        ry *= scale

    numerator = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p
    denominator = rx * rx * y1p * y1p + ry * ry * x1p * x1p
    coefficient = 0.0
    if denominator > 1.0e-12:
        coefficient = math.sqrt(max(numerator / denominator, 0.0))
        if bool(large_arc) == bool(sweep):
            coefficient = -coefficient
    cxp = coefficient * rx * y1p / ry
    cyp = -coefficient * ry * x1p / rx
    cx = cos_phi * cxp - sin_phi * cyp + (x1 + x2) * 0.5
    cy = sin_phi * cxp + cos_phi * cyp + (y1 + y2) * 0.5

    ux = (x1p - cxp) / rx
    uy = (y1p - cyp) / ry
    vx = (-x1p - cxp) / rx
    vy = (-y1p - cyp) / ry
    start_angle = _vector_angle(1.0, 0.0, ux, uy)
    delta = _vector_angle(ux, uy, vx, vy)
    if not sweep and delta > 0.0:
        delta -= math.tau
    elif sweep and delta < 0.0:
        delta += math.tau

    segment_count = max(1, int(math.ceil(abs(delta) / (math.pi * 0.5))))
    segment_delta = delta / segment_count

    def map_point(x, y):
        return (
            cx + rx * cos_phi * x - ry * sin_phi * y,
            cy + rx * sin_phi * x + ry * cos_phi * y,
        )

    cubics = []
    angle = start_angle
    for _ in range(segment_count):
        next_angle = angle + segment_delta
        alpha = 4.0 / 3.0 * math.tan(segment_delta * 0.25)
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        cos_b = math.cos(next_angle)
        sin_b = math.sin(next_angle)
        control1 = map_point(cos_a - alpha * sin_a, sin_a + alpha * cos_a)
        control2 = map_point(cos_b + alpha * sin_b, sin_b - alpha * cos_b)
        target = map_point(cos_b, sin_b)
        cubics.append((control1, control2, target))
        angle = next_angle
    return cubics


def parse_path(path_data):
    tokens = TOKEN_PATTERN.findall(path_data)
    output = []
    index = 0
    command = None
    current = (0.0, 0.0)
    subpath_start = current
    last_cubic_control = None
    last_quadratic_control = None

    def emit_move(point):
        output.extend((COMMAND_MOVE_TO, point[0], point[1]))

    def emit_line(point):
        output.extend((COMMAND_LINE_TO, point[0], point[1]))

    def emit_quadratic(control, point):
        output.extend((COMMAND_CURVE_TO, control[0], control[1], point[0], point[1]))

    def emit_cubic(control1, control2, point):
        output.extend((COMMAND_CUBIC_TO, control1[0], control1[1], control2[0], control2[1], point[0], point[1]))

    while index < len(tokens):
        if tokens[index].isalpha():
            command = tokens[index]
            index += 1
        if command is None:
            raise ValueError("Path data begins without an SVG command.")
        upper = command.upper()
        relative = command.islower()
        if upper == "Z":
            if abs(current[0] - subpath_start[0]) > 1.0e-6 or abs(current[1] - subpath_start[1]) > 1.0e-6:
                emit_line(subpath_start)
            current = subpath_start
            last_cubic_control = None
            last_quadratic_control = None
            command = None
            continue

        parameter_count = PARAMETER_COUNTS[upper]
        first_group = True
        consumed_group = False
        while index < len(tokens) and not tokens[index].isalpha():
            if index + parameter_count > len(tokens):
                raise ValueError(f"Incomplete SVG {command} command.")
            values = [float(value) for value in tokens[index:index + parameter_count]]
            index += parameter_count
            consumed_group = True

            def point(x, y):
                if relative:
                    return current[0] + x, current[1] + y
                return x, y

            if upper == "M":
                target = point(values[0], values[1])
                if first_group:
                    emit_move(target)
                    subpath_start = target
                else:
                    emit_line(target)
                current = target
                last_cubic_control = None
                last_quadratic_control = None
            elif upper == "L":
                current = point(values[0], values[1])
                emit_line(current)
                last_cubic_control = None
                last_quadratic_control = None
            elif upper == "H":
                current = (current[0] + values[0], current[1]) if relative else (values[0], current[1])
                emit_line(current)
                last_cubic_control = None
                last_quadratic_control = None
            elif upper == "V":
                current = (current[0], current[1] + values[0]) if relative else (current[0], values[0])
                emit_line(current)
                last_cubic_control = None
                last_quadratic_control = None
            elif upper == "C":
                control1 = point(values[0], values[1])
                control2 = point(values[2], values[3])
                target = point(values[4], values[5])
                emit_cubic(control1, control2, target)
                current = target
                last_cubic_control = control2
                last_quadratic_control = None
            elif upper == "S":
                control1 = current if last_cubic_control is None else (
                    current[0] * 2.0 - last_cubic_control[0],
                    current[1] * 2.0 - last_cubic_control[1],
                )
                control2 = point(values[0], values[1])
                target = point(values[2], values[3])
                emit_cubic(control1, control2, target)
                current = target
                last_cubic_control = control2
                last_quadratic_control = None
            elif upper == "Q":
                control = point(values[0], values[1])
                target = point(values[2], values[3])
                emit_quadratic(control, target)
                current = target
                last_quadratic_control = control
                last_cubic_control = None
            elif upper == "T":
                control = current if last_quadratic_control is None else (
                    current[0] * 2.0 - last_quadratic_control[0],
                    current[1] * 2.0 - last_quadratic_control[1],
                )
                target = point(values[0], values[1])
                emit_quadratic(control, target)
                current = target
                last_quadratic_control = control
                last_cubic_control = None
            elif upper == "A":
                target = point(values[5], values[6])
                cubics = _arc_to_cubics(current, target, values[0], values[1], values[2], values[3] != 0.0, values[4] != 0.0)
                if cubics:
                    for control1, control2, cubic_target in cubics:
                        emit_cubic(control1, control2, cubic_target)
                else:
                    emit_line(target)
                current = target
                last_cubic_control = None
                last_quadratic_control = None
            first_group = False
            if index < len(tokens) and tokens[index].isalpha():
                break
        if not consumed_group:
            raise ValueError(f"SVG command {command} has no parameters.")
    return output


def source_path(source_root, icon_name, weight):
    filename = f"{icon_name}.svg" if weight == "regular" else f"{icon_name}-{weight}.svg"
    return source_root / "assets" / weight / filename


def load_variant(path):
    root = ET.parse(path).getroot()
    if root.attrib.get("viewBox") != "0 0 256 256":
        raise ValueError(f"Unexpected viewBox in {path}.")
    layers = []
    for child in list(root):
        if child.tag.rsplit("}", 1)[-1] != "path":
            raise ValueError(f"Only path elements are supported: {path}.")
        if "transform" in child.attrib or "fill-rule" in child.attrib or "clip-rule" in child.attrib:
            raise ValueError(f"Unsupported path attribute in {path}.")
        opacity = float(child.attrib.get("opacity", "1"))
        commands = parse_path(child.attrib["d"])
        if layers and abs(layers[-1][0] - opacity) <= 1.0e-6:
            layers[-1][1].extend(commands)
        else:
            layers.append([opacity, commands])
    if not layers:
        raise ValueError(f"No paths found in {path}.")
    return layers


def write_names(path, icons, revision):
    lines = [
        "/*!",
        "* This file is a portion of LunaSDK.",
        "* For conditions of distribution and use, see the disclaimer",
        "* and license in LICENSE.txt.",
        "*",
        "* This file is generated by Tools/GUIIconGen/Generate.py. Do not edit.",
        f"* Phosphor Core revision: {revision}",
        "*/",
    ]
    lines.extend(f"{name.replace('-', '_')}," for name in icons)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_binary(path, floats, layers, variants, revision):
    revision_bytes = revision.encode("ascii")[:40].ljust(40, b"\0")
    with path.open("wb") as stream:
        stream.write(struct.pack("<8sIIII40s", MAGIC, FORMAT_VERSION, len(floats), len(layers), len(variants), revision_bytes))
        for first_float, num_floats, opacity in layers:
            stream.write(struct.pack("<IIf", first_float, num_floats, opacity))
        for first_layer, num_layers in variants:
            stream.write(struct.pack("<II", first_layer, num_layers))
        stream.write(struct.pack(f"<{len(floats)}f", *floats))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=pathlib.Path)
    parser.add_argument("--manifest", required=True, type=pathlib.Path)
    parser.add_argument("--names-output", required=True, type=pathlib.Path)
    parser.add_argument("--data-output", required=True, type=pathlib.Path)
    parser.add_argument("--revision", required=True)
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    icons = manifest["icons"]
    optional_weights = manifest["weights"]
    if len(icons) != len(set(icons)):
        raise ValueError("The icon manifest contains duplicate icon names.")

    floats = []
    layers = []
    variants = [(INVALID_INDEX, 0) for _ in range(len(icons) * len(WEIGHTS))]
    for icon_index, icon_name in enumerate(icons):
        requested_weights = ["regular"]
        requested_weights.extend(weight for weight in WEIGHTS[1:] if icon_name in optional_weights.get(weight, []))
        for weight in requested_weights:
            svg_path = source_path(args.source, icon_name, weight)
            if not svg_path.is_file():
                raise FileNotFoundError(svg_path)
            source_layers = load_variant(svg_path)
            first_layer = len(layers)
            for opacity, commands in source_layers:
                first_float = len(floats)
                floats.extend(commands)
                layers.append((first_float, len(commands), opacity))
            variants[icon_index * len(WEIGHTS) + WEIGHTS.index(weight)] = (first_layer, len(source_layers))

    args.names_output.parent.mkdir(parents=True, exist_ok=True)
    args.data_output.parent.mkdir(parents=True, exist_ok=True)
    write_names(args.names_output, icons, args.revision)
    write_binary(args.data_output, floats, layers, variants, args.revision)
    print(json.dumps({
        "icons": len(icons),
        "variants": sum(1 for first_layer, _ in variants if first_layer != INVALID_INDEX),
        "layers": len(layers),
        "floats": len(floats),
        "bytes": args.data_output.stat().st_size,
        "revision": args.revision,
    }, indent=2))


if __name__ == "__main__":
    main()
