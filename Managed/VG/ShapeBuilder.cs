using System;
using Luna.Runtime;
using Luna.VG.Internal;

namespace Luna.VG;

public static class ShapeBuilder
{
    public static void AddRectangleFilled(IShapeBuffer shapeBuffer, float minX, float minY, float maxX, float maxY)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddRectangleFilled(buffer, minX, minY, maxX, maxY));
    }

    public static void AddRectangleBordered(IShapeBuffer shapeBuffer, float minX, float minY, float maxX, float maxY, float borderWidth, float borderOffset = 0.0f)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddRectangleBordered(buffer, minX, minY, maxX, maxY, borderWidth, borderOffset));
    }

    public static void AddRoundedRectangleFilled(IShapeBuffer shapeBuffer, float minX, float minY, float maxX, float maxY, float radius)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddRoundedRectangleFilled(buffer, minX, minY, maxX, maxY, radius));
    }

    public static void AddRoundedRectangleBordered(IShapeBuffer shapeBuffer, float minX, float minY, float maxX, float maxY, float radius, float borderWidth, float borderOffset = 0.0f)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddRoundedRectangleBordered(buffer, minX, minY, maxX, maxY, radius, borderWidth, borderOffset));
    }

    public static void AddTriangleFilled(IShapeBuffer shapeBuffer, float x1, float y1, float x2, float y2, float x3, float y3)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddTriangleFilled(buffer, x1, y1, x2, y2, x3, y3));
    }

    public static void AddTriangleBordered(IShapeBuffer shapeBuffer, float x1, float y1, float x2, float y2, float x3, float y3, float borderWidth, float borderOffset = 0.0f)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddTriangleBordered(buffer, x1, y1, x2, y2, x3, y3, borderWidth, borderOffset));
    }

    public static void AddCircleFilled(IShapeBuffer shapeBuffer, float centerX, float centerY, float radius)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddCircleFilled(buffer, centerX, centerY, radius));
    }

    public static void AddCircleBordered(IShapeBuffer shapeBuffer, float centerX, float centerY, float radius, float borderWidth, float borderOffset = 0.0f)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddCircleBordered(buffer, centerX, centerY, radius, borderWidth, borderOffset));
    }

    public static void AddAxisAlignedEllipseFilled(IShapeBuffer shapeBuffer, float centerX, float centerY, float radiusX, float radiusY)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddAxisAlignedEllipseFilled(buffer, centerX, centerY, radiusX, radiusY));
    }

    public static void AddAxisAlignedEllipseBordered(IShapeBuffer shapeBuffer, float centerX, float centerY, float radiusX, float radiusY, float borderWidth, float borderOffset = 0.0f)
    {
        AddCore(shapeBuffer, (buffer) => VgNative.ShapeBuilderAddAxisAlignedEllipseBordered(buffer, centerX, centerY, radiusX, radiusY, borderWidth, borderOffset));
    }

    private static void AddCore(IShapeBuffer shapeBuffer, System.Func<nint, nuint> action)
    {
        ArgumentNullException.ThrowIfNull(shapeBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(action(NativeShapeBuffer.GetNativeShapeBufferPointer(shapeBuffer))));
    }
}
