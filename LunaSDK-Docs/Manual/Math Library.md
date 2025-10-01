LunaSDK comes with a math library that contains most math types and functions commonly-used in 3D applications. The math library contains the following parts:

```c++
#include <Luna/Runtime/Math/Math.hpp> // For basic math types and functions.
#include <Luna/Runtime/Math/Vector.hpp> // For vector types and functions.
#include <Luna/Runtime/Math/Matrix.hpp> // For matrix types and functions.
#include <Luna/Runtime/Math/Quaternion.hpp> // For Quaternion type and functions.
#include <Luna/Runtime/Math/Transform.hpp> // For transform operations.
```

The math library use SIMD instructions for vector and matrix calculations when possible. If SIMD instructions are not available on the target platform, we also have non-SIMD implementations for all math operations for compatibility. The SIMD instructions support of math library includes support of SSE/SSE2, SSE3, SSE4, AVX, AVX2, FMA3 and SVML instruction sets on x86/x64 processors, and NEON instruction set on arm/arm64 processors.

## Vectors

```c++
#include <Luna/Runtime/Math/Vector.hpp>
```

`Float2`, `Float3` and `Float4` represent 2D, 3D and 4D vectors. These three types are 16-bytes aligned for maximizing SIMD performance. LunaSDK also provides unaligned vector types, these types are `Vec2U<T>`, `Vec3U<T>` and `Vec4U<T>`. The unaligned types are used mainly for storing and transferring vectors, such types should be converted to aligned types before they can be used for calculations. LunaSDK also defines `Float2U`, `Int2U`, `UInt2U`, `Float3U`, `Int3U`, `UInt3U`, `Float4U`, `Int4U`, `UInt4U` as aliasing types of `Vec2U<T>`, `Vec3U<T>`, `Vec24U<T>` for convenience. Components of these vector types can be fetched by their `x`, `y`, `z` and `w` properties.

Aligned vector types can be compared(`==` and `!=`), added (`+`), subtracted (`-`), multiplied (`*`) and divided (`/`) like normal scalar types. These calculations are performed as performing the same calculation on each component element of the vector individually. When performing mathematical calculations between vector types and scalar types, the scaler number will be applied to all components of the vector.

LunaSDK defines a series of functions to perform basic vector calculations. All these functions provide overloaded versions for handling 2D, 3D and 4D vector types. The following table lists all vector functions.

| Function                     | Description                                                  |
| ---------------------------- | ------------------------------------------------------------ |
| `in_bounds(a, min, max)`     | Tests if `a` is in `[min, max]` bounds.                      |
| `length(a)`                  | Returns the length of vector `a`.                            |
| `length_squared(a)`          | Returns the squared length of vector `a`. This is faster than `length`. |
| `dot(a, b)`                  | Returns the dot product of vector `a` and vector `b`.        |
| `cross(a, b)`                | Returns the cross product of vector `a` and vector `b`.      |
| `normalize(a)`               | Returns the normalized vector of vector `a`.                 |
| `clamp(a, min, max)`         | Clamps vector `a` in `[min, max]` range.                     |
| `distance(a, b)`             | Returns the Euclidean distance from vector `a` to vector `b`. |
| `distance_squared(a, b)`     | Returns the squared Euclidean distance from vector `a` to vector `b`. This is faster than ``distance`. |
| `min(a, b)`                  | Returns one vector composed by the smaller component of each component in `a` and `b`. |
| `max(a, b)`                  | Returns one vector composed by the larger component of each component in `a` and `b`. |
| `lerp(a, b, t)`              | Performs [Linear interpolation](https://en.wikipedia.org/wiki/Linear_interpolation) between vector `a` and vector `b` according to one scalar factor `t`. |
| `smoothstep(a, b, t)`        | Performs [Smoothstep](https://en.wikipedia.org/wiki/Smoothstep) between vector `a` and vector `b` according to one scalar factor `t`. |
| `barycentric(a, b, c, x, y)` | Performs [Barycentric triangle interpolation](https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmvectorbarycentric) using three vector points `a`, `b`, `c` according to two scalar factors `x` and `y`. |
| `catmull_rom(a, b, c, d, t)` | Performs [Catmull-Rom spline](https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline) interpolation using four vector points `a`, `b`, `c`, `d` according to one scalar factor `t`. |
| `hermite(a, t1, b, t2, t)`   | Performs [Cubic Hermite spline](https://en.wikipedia.org/wiki/Cubic_Hermite_spline) interpolation using two vector points `a`, `b`, two vector tangents `t1`, `t2` according to one scalar factor `t`. |
| `reflect(i, n)`              | Computes the reflection vector of the incident vector `i` using the normal vector `n`. |
| `refract(i, n, r)`           | Computes the refraction vector of the incident vector `i` using the normal vector `n` and refraction index `r`. |

## Matrices

```c++
#include <Luna/Runtime/Math/Matrix.hpp>
```

`Float3x3` and `Float4x4` represent 3x3 and 4x4 32-bit floating-point matrices. These two types are 16-bytes aligned for maximizing SIMD performance. LunaSDK also provides unaligned matrix types, these types are `Float3x2U`, `Float3x3U`, `Float4x3U` and `Float4x4U`. The  unaligned types are used for storing and transferring matrices, and should be converted to aligned types (`Float3x2U` to `Float3x3`, `Float4x3U` to `Float4x4`) before they can be used for calculation. Rows in one matrix can be fetched by the `m` property of the matrix type, which is an array of `Float3` or `Float4` for `Float3x3` and `Float4x4`, or an two-dimensional `f32` array for any unaligned matrix type.

Aligned matrix types can be compared(`==` and `!=`), added (`+`), subtracted (`-`), multiplied (`*`) and divided (`/`) like normal scalar types. These calculations are performed as performing the same calculation on each component element of the matrix individually. When performing mathematical calculations between matrix types and scalar types, the scaler number will be applied to all components of the matrix.

LunaSDK defines a series of functions to perform basic matrix calculations. All these functions provide overloaded versions for handling different matrix types. The following table lists all matrix functions.

| Function         | Description                                                  |
| ---------------- | ------------------------------------------------------------ |
| `mul(a, b)`      | Performs matrix multiplication between `a` and `b`, where `a` and `b` can be vector or matrix types. |
| `determinant(m)` | Computes determinant of one matrix `m`.                      |
| `transpose(m)`   | Computes the transpose matrix of one matrix `m`.             |
| `inverse(m)`     | Computes the inversed matrix of one matrix `m`.              |

## Quaternions

```c++
#include <Luna/Runtime/Math/Quaternion.hpp>
```

`Quaternion` represents one Quaternion that can be used to represent a rotating operation in 3D space. Every `Quaternion` contains four `f32` components, and is 16-bytes aligned for maximizing SIMD performance. The user can convert one `Quaternion ` to `Float4U` for storing and transferring the Quaternion.

`Quaternion` can be compared(`==` and `!=`), added (`+`), subtracted (`-`), multiplied (`*`) and divided (`/`) like normal scalar types. The addition and subtraction behavior of one Quaternion is the same as those of `Float4`. The multiplication operation concatenates two Quaternions, while the division operation decomposes one Quaternion into two.

LunaSDK defines a series of functions to perform Quaternion calculations. The following table lists all Quaternion functions.

| Function            | Description                                                  |
| ------------------- | ------------------------------------------------------------ |
| `length(q)`         | Returns the length of one `Quaternion`. Same as `length` for `Float4`. |
| `length_squared(q)` | Returns the squared length of one `Quaternion`. Same as `length_squared` for `Float4`. |
| `normalize(q)`      | Normalizes one `Quaternion`. Same as `normalize` for `Float4`. |
| `conjugate(q)`      | Computes the conjugate of one `Quaternion`.                  |
| `inverse(q)`        | Computes the inverse of one `Quaternion`.                    |
| `dot(q1, q2)`       | Computes the dot product of two `Quaternion`s `q1` and `q2`. Same as `dot` for `Float4`. |
| `lerp(q1, q2, t)`   | Performs [linear interpolation](https://en.wikipedia.org/wiki/Linear_interpolation) on two `Quaternion`s `q1` and `q2` according to one scalar factor `t`. |
| `slerp(q1, q2, t)`  | Performs [spherical linear interpolation](https://en.wikipedia.org/wiki/Slerp) on two `Quaternion`s `q1` and `q2` according to one scalar factor `t`. |

## Transform

```c++
#include <Luna/Runtime/Math/Transform.hpp>
```

The transform header file does not include any new type. Instead, it defines a set of functions that can be useful for constructing affine matrices and projection matrices that are used in 2D and 3D transformations.

### Affine matrix operations

2D affine matrices and 3D affine matrices are presented by `Float3x3` and `Float4x4`. The +x axis of one affine matrix points to right, the +y axis of one affine matrix points to top, the +z axis of one affine matrix points to forward.

The following table lists all functions for operating affine matrices. All functions are declared in `AffineMatrix` namespace.

| Function                                | Description                                                  |
| --------------------------------------- | ------------------------------------------------------------ |
| `make(p, r, s)`                         | Constructs one 2D or 3D affine matrix from position vector `p`, rotation scalar or Quaternion `r` and scaling vector `s`. |
| `up(m)`                                 | Extracts the up vector from one 2D or 3D affine matrix.      |
| `down(m)`                               | Extracts the down vector from one 2D or 3D affine matrix.    |
| `left(m)`                               | Extracts the left vector from one 2D or 3D affine matrix.    |
| `right(m)`                              | Extracts the right vector from one 2D or 3D affine matrix.   |
| `forward`                               | Extracts the forward vector from one 3D affine matrix.       |
| `backward`                              | Extracts the backward vector from one 2D or 3D affine matrix. |
| `translation(m)`                        | Extracts the translation vector from one 2D or 3D affine matrix. |
| `rotation(m)`                           | Extracts the rotation scalar or Quaternion from one 2D or 3D affine matrix. |
| `euler_angles(m)`                       | Extracts the rotation vector that uses stores the rotation in Euler angles (pitch, yaw, roll) from one 3D affine matrix. |
| `scaling(m)`                            | Extracts the scaling vector from one 2D or 3D affine matrix. |
| `translation_matrix(m)`                 | Extracts the translation matrix from one 2D or 3D affine matrix. |
| `rotation_matrix(m)`                    | Extracts the rotation matrix from one 2D or 3D affine matrix. |
| `scaling_matrix(m)`                     | Extracts the scaling matrix from one 2D or 3D affine matrix. |
| `make_translation(t)`                   | Constructs one 2D or 3D translation matrix from position vector `p`. |
| `make_rotation(r)`                      | Constructs one 2D or 3D rotation matrix from rotation scalar or Quaternion `r`. |
| `make_rotation_x(r)`                    | Constructs one 3D rotation matrix that represents one rotation alone x axis. |
| `make_rotation_y(r)`                    | Constructs one 3D rotation matrix that represents one rotation alone y axis. |
| `make_rotation_z(r)`                    | Constructs one 3D rotation matrix that represents one rotation alone z axis. |
| `make_rotation_axis_angle(axis, angle)` | Constructs one 3D rotation matrix by specifying the rotation axis and rotation angle. |
| `make_rotation_euler_angles`            | Constructs one 3D rotation matrix from Euler angles (pitch, yaw, roll). |
| `make_scaling(s)`                       | Constructs one 2D or 3D scaling matrix from scaling vector `s`. |
| `make_look_at(eye, target, up)`         | Constructs one view matrix that targets the specified position. |
| `make_look_to(eye, dir, up)`            | Constructs one view matrix that targets the specified direction. |

### Projection matrix operations

The following table lists all functions for operating projection matrices. All functions are declared in `ProjectionMatrix` namespace.

| Function                                                     | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `make_perspective(width, height, near_z, far_z)`             | Constructs a perspective projection matrix using width and height of the frustum. |
| `make_perspective_fov(fov, aspect_ratio, near_z, far_z)`     | Constructs a perspective projection matrix using field-of-view and aspect_ratio of the frustum. |
| `make_perspective_off_center(left, right, bottom, top, near_z, far_z)` | Constructs a perspective projection matrix using offsets of the four sides of the frustum from the camera center. |
| `make_orthographic(width, height, near_z, far_z)`            | Constructs a orthographic projection matrix using width and height of the frustum. |
| `make_orthographic_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)` | Constructs a orthographic projection matrix using offsets of the four sides of the frustum from the camera center. |

