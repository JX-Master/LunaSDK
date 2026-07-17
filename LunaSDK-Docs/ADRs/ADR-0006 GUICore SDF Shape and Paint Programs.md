## Status
Approved.

## Last updated
2026/7/17

## Background
GUICore compiles GUI-level drawing commands while preserving element painter order, layer order, and clip state.
VG remains appropriate for text, images, icons, and arbitrary vector contours, but the editor design language also
needs analytic rectangles, rounded rectangles, circles, capsules, constructive geometry, gradients, borders,
highlights, and soft inset and outset effects.

The former dedicated rounded-rectangle shadow pipeline made shadows a rendering special case. The first SDF
prototype generalized that pipeline, but encoded shape and color data as `float4` records with a common opcode,
record length, arity, and format-version header. It also stored fill, stroke, outer-shadow, and inner-shadow state
in per-instance data. That representation duplicated information already implied by each opcode and prevented a
single shape from being reused cleanly by several concrete drawing passes.

The SDF backend must satisfy these constraints:

1. Preserve the existing GUICore command, element, layer, clip, and painter-order model.
2. Keep widget semantics and Style names out of GUICore and its shaders.
3. Use bounded and inspectable data rather than arbitrary shader callbacks.
4. Support Metal, D3D12, and Vulkan without bindless resources.
5. Allow several ordered color or effect passes to reuse one shape range.
6. Batch adjacent SDF instances without changing their logical order.
7. Keep VG as the backend for text, images, and complex monochrome vector shapes.

## Decision
Add a general signed-distance-field drawing backend to GUICore. The backend consumes two scalar float32 streams:

1. A **shape buffer** containing analytic primitives and prefix constructive-solid-geometry expressions.
2. A **color buffer** containing one concrete paint or effect instruction for each logical SDF pass.

Both GPU buffers use a four-byte structured-buffer stride, matching the scalar command storage model used by
`VG::IShapeBuffer`. The SDF command ABI remains private to GUICore and does not extend the VG contour ABI.

### Rendering boundary
The GUICore logical layer owns one ordered draw-command stream. One SDF draw references one validated shape offset
and one validated color offset. Several SDF draws may reference the same shape offset while selecting different
color offsets, for example:

```text
Shadow -> SolidColor -> Highlight
```

The rendering layer preserves this order and merges consecutive SDF instances that use the same shape/color page
pair into one instanced draw call. VG and SDF batches remain interleaved in exact painter order.

The application owns render-target transitions and render-pass boundaries. `IRenderer::prepare` validates and
uploads SDF data outside the render pass. `IRenderer::render` records prepared SDF and VG batches inside the
application-owned pass.

### Scalar instruction format
Shape and color buffers are `Vector<f32>` streams on the CPU and `StructuredBuffer<float>` resources on the GPU.
Every instruction begins with one integer-valued float32 opcode. Opcodes remain below `2^24`, so conversion between
the float representation and `u32` is exact.

There is no universal instruction length, arity, or format-version header. Fixed-length instructions derive their
layout from the opcode. An instruction that genuinely contains variable data stores an opcode-specific count, such
as `num_stops` or a future `num_points` field.

CPU handles retain an offset and allocation length for validation and page packing. Those lengths are not encoded
in the instruction or passed to the shader. Instructions and their variable payloads may not cross a program-page
boundary. The streams are transient rendering data rather than a persistent serialized ABI. If persistent caches
are introduced, their version belongs to resource-level metadata rather than every instruction.

### Shape instructions
The initial shape instruction set contains:

1. Axis-aligned rectangle.
2. Axis-aligned rounded rectangle with four independent circular corner radii.
3. Circle.
4. Axis-aligned ellipse.
5. Capsule defined by a line segment and radius.
6. Prefix union, intersection, difference, and exclusive-or operations.

Primitive parameter counts and operation arity are implied by the opcode. Shape coordinates use GUI logical
coordinates with the origin at the top left and Y increasing downward. A primitive returns a signed distance with
negative values inside the shape.

Binary operations preserve operand order and use prefix notation:

```text
union(A, B)        = min(A, B)
intersection(A, B) = max(A, B)
difference(A, B)   = max(A, -B)
xor(A, B)          = max(min(A, B), -max(A, B))
```

The CPU validator derives expression length, operand count, maximum stack depth, instruction count, and conservative
bounds. The initial limits remain 64 instructions and a maximum evaluation stack depth of 16.

### Color instructions
One color-buffer offset identifies one concrete paint or effect pass. The low eight opcode bits select the base
algorithm. Two additional bits select independent signed-distance clipping:

```text
bits 0..7: base color opcode
bit 8:     inner clip
bit 9:     outer clip
```

The initial base algorithms are:

1. Solid color.
2. Linear gradient.
3. Radial gradient.
4. Conic gradient.
5. Four-corner bilinear gradient.
6. Analytic shadow.

Colors are concrete non-premultiplied sRGB values resolved from Style data on the CPU. The color buffer stores no
Style names or Style indexes. Gradient instructions store `num_stops` followed by ordered fixed-stride stop data.
The initial implementation supports pad and repeat spread modes, midpoint-adjusted sRGB interpolation, and at most
16 stops.

### General distance clipping
For a signed distance `d`, the color instruction optionally stores `inner_distance`, `outer_distance`, or both
immediately after its encoded opcode. The shader constructs one clip distance:

```text
inner term = -d - inner_distance
outer term =  d - outer_distance
clip distance = maximum of the enabled terms
```

The four modes are:

1. `00b`: no SDF clipping; coverage is one throughout the draw mesh.
2. `01b`: inner clipping; points deeper than `inner_distance` inside the contour are rejected.
3. `10b`: outer clipping; points farther than `outer_distance` outside the contour are rejected.
4. `11b`: both limits apply, producing an independently controllable boundary band.

Legacy fill is the convenience configuration `outer clip, outer_distance = 0`. A centered stroke of width `w` is
`inner and outer clip, inner_distance = outer_distance = w / 2`. Different distances produce asymmetric inward and
outward strokes without multiplying the base color-opcode set.

The clip coverage uses `fwidth` antialiasing. The shader may discard fragments only after obtaining a zero
antialiased clip coverage, so edge samples are not removed prematurely.

### Unified analytic shadow
Outer and inner shadows use one Shadow base opcode. The instruction stores RGBA, offset, softness, and spread.
The shader compares the original shape mask with a shifted and softened mask to produce a two-sided shadow signal.
The generic clip flags then select the visible side:

1. An outer shadow uses inner clip with `inner_distance = 0`.
2. An inner shadow uses outer clip with `outer_distance = 0`.
3. No clip keeps both sides.
4. Both clips keep a bounded band around the contour.

Shadow color and all shadow parameters live in the color buffer. They are not part of `SDFDrawDesc` or the SDF
instance layout.

### Raster domain, bounds, and GUI clipping
Every SDF pass is rasterized through a finite rectangle mesh. That mesh is the shader invocation domain. NoClip and
InnerClip mean only that the shader does not apply an outward SDF limit; they do not create an infinite draw.

The renderer derives a conservative mesh rectangle from the resolved DrawCommand rectangle, shape bounds, color
algorithm, clip distances, and effect falloff. Outer clipping expands shape bounds by `outer_distance`. A shadow
mesh applies its offset and spread, then adds a finite softness cutoff and antialiasing margin. The existing element
and explicit GUI clip stack still constrains the result.

The responsibilities remain independent:

1. The rectangle mesh bounds shader invocation.
2. The shape buffer supplies signed distance.
3. Color clip bits restrict signed-distance coverage inside the mesh.
4. The color opcode computes paint or effect output.
5. The GUI clip rectangle implements parent and scrolling clips.

### Blending and color output
The SDF pipeline emits premultiplied-alpha colors and uses source-over blending. Each pass emits only its own paint
or effect; ordered layering composes the final surface. Invalid instructions fail closed on the CPU, and an unknown
shader opcode outputs transparent black.

### Buffer ownership and paging
The context owns transient scalar shape and color streams. Generated-command regeneration restores recorded streams
before draw callbacks run so repeated generation is deterministic. The renderer uploads scalar streams into
reusable read-only structured-buffer pages. Descriptor sets bind one shape page and one color page, and instances
store page-local scalar offsets.

The initial page remains 16 MiB. In scalar units this is four times as many entries as the former `float4` page.
Programs and their variable payloads are padded before the page edge when necessary.

### Migration
Migration proceeds atomically within the GUICore SDF feature branch:

1. Replace `Float4U` SDF builders, validation, handles, Context storage, and CPU evaluators with scalar streams.
2. Replace fill/stroke/shadow instance fields with color opcode clip flags and parameters.
3. Merge outer and inner shader paths into the unified Shadow opcode.
4. Keep transitional rectangle, rounded-rectangle, gradient-rectangle, and shadow commands, but translate each into
   one or more scalar SDF color passes in the renderer.
5. Migrate explicit SDF consumers and tests.
6. Retain VG for text, images, lines, and arbitrary vector paths.

The legacy primitive translation is temporary migration staging rather than a permanent compatibility layer.

## Impact
Expected benefits:

1. Shapes are reusable across ordered fill, border, highlight, and shadow passes.
2. Fill, symmetric stroke, asymmetric stroke, inner clipping, and outer clipping share one distance model.
3. One Shadow opcode covers inner, outer, two-sided, and bounded contour effects.
4. Scalar buffers match VG's established command-buffer storage and remove unused header fields.
5. Variable-length instructions pay for count fields only when required.
6. Adjacent logical passes may remain one physical instanced draw call.

Costs and risks:

1. Shader behavior depends on CPU validation because instructions no longer carry universal lengths or versions.
2. Fragment cost grows with shape depth, gradient-stop count, and Shadow's second shape evaluation.
3. Dynamic clip and opcode branches require cross-backend shader tests.
4. CSG min/max preserves contours but may not preserve exact Euclidean distance near seams.
5. Discard may not improve performance on every GPU architecture and must remain a measured optimization.
6. The initial color model does not implement the complete CSS painting and compositing specification.

Mitigations:

1. Validate every instruction, count, scalar parameter, and prefix expression before upload.
2. Keep strict instruction, stack, and color-stop limits.
3. Pack complete variable instructions within one page.
4. Add CPU reference evaluation and cross-backend shader compilation tests.
5. Track scalar counts, upload bytes, instance counts, page switches, and SDF/VG batch switches.
6. Treat shader discard as an implementation choice verified by profiling rather than part of the buffer ABI.

## Alternatives considered
### Keep the common float4 instruction header
* Status: rejected.
* Reason: fixed-length opcodes do not need record length, arity, and format version on every instruction. Scalar
  streams are more compact and match VG's existing structured-buffer convention.

### Keep fill, stroke, outer shadow, and inner shadow in per-instance state
* Status: rejected.
* Reason: those fields mix geometry references with concrete paint algorithms, duplicate color data, and make one
  shape-plus-one-color pass less self-contained.

### Create a separate opcode for every fill and stroke paint
* Status: rejected.
* Reason: two clip bits apply the same signed-distance restriction to every base paint or effect without multiplying
  the opcode set.

### Require every instruction to have one fixed allocation size
* Status: rejected.
* Reason: gradients and future point lists are naturally variable. Opcode-specific count fields preserve compactness
  without restoring a universal header.

### Treat NoClip or InnerClip as an infinite draw
* Status: rejected.
* Reason: the finite rectangle mesh already bounds shader invocation. The clip bits describe only SDF coverage inside
  that raster domain.

### Extend the VG contour command buffer
* Status: rejected.
* Reason: VG evaluates contour coverage rather than signed distance. Mixing GUI CSG and analytic effects into its
  contour ABI would join unrelated rendering models.

## Remarks
The scalar SDF stream is private transient rendering data. GUIAsset stores semantic widget and Style data rather
than raw SDF instructions.

## Version history
* **2026/7/17** Proposed and approved.
* **2026/7/17** Replaced the float4 program-header model with scalar instructions, generic distance clip bits, and a
  unified Shadow color opcode.
