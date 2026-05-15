using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Luna.ImGui.Internal;
using Luna.Runtime;
using Luna.Runtime.Internal;
using Luna.RHI;

namespace Luna.ImGui;

public static class ImGuiApi
{
    public static void NewFrame()
    {
        ImGuiNative.NewFrame();
    }

    public static void ShowDemoWindow()
    {
        ImGuiNative.ShowDemoWindow();
    }

    public static bool Begin(string name)
    {
        ArgumentException.ThrowIfNullOrEmpty(name);
        return ImGuiNative.Begin(name) != 0;
    }

    public static void End()
    {
        ImGuiNative.End();
    }

    public static void Text(string text)
    {
        ArgumentNullException.ThrowIfNull(text);
        ImGuiNative.Text(text);
    }

    public static void Image(ITexture texture, Vector2 imageSize, Vector2? uv0 = null, Vector2? uv1 = null)
    {
        ArgumentNullException.ThrowIfNull(texture);
        var nativeImageSize = NativeFloat2.FromPublic(imageSize);
        var nativeUv0 = NativeFloat2.FromPublic(uv0 ?? Vector2.Zero);
        var nativeUv1 = NativeFloat2.FromPublic(uv1 ?? Vector2.One);
        ImGuiNative.ImageTexture(texture.GetNativeHandle(), in nativeImageSize, in nativeUv0, in nativeUv1);
    }

    public static void Image(ISampledImage sampledImage, Vector2 imageSize, Vector2? uv0 = null, Vector2? uv1 = null)
    {
        ArgumentNullException.ThrowIfNull(sampledImage);
        var nativeImageSize = NativeFloat2.FromPublic(imageSize);
        var nativeUv0 = NativeFloat2.FromPublic(uv0 ?? Vector2.Zero);
        var nativeUv1 = NativeFloat2.FromPublic(uv1 ?? Vector2.One);
        ImGuiNative.ImageSampledImage(NativeSampledImage.GetNativeSampledImagePointer(sampledImage), in nativeImageSize, in nativeUv0, in nativeUv1);
    }

    public static bool ImageButton(string id, ITexture texture, Vector2 imageSize, Vector2? uv0 = null, Vector2? uv1 = null, Vector4? backgroundColor = null, Vector4? tintColor = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(id);
        ArgumentNullException.ThrowIfNull(texture);
        var nativeImageSize = NativeFloat2.FromPublic(imageSize);
        var nativeUv0 = NativeFloat2.FromPublic(uv0 ?? Vector2.Zero);
        var nativeUv1 = NativeFloat2.FromPublic(uv1 ?? Vector2.One);
        var nativeBackgroundColor = NativeFloat4.FromPublic(backgroundColor ?? Vector4.Zero);
        var nativeTintColor = NativeFloat4.FromPublic(tintColor ?? Vector4.One);
        return ImGuiNative.ImageButtonTexture(id, texture.GetNativeHandle(), in nativeImageSize, in nativeUv0, in nativeUv1, in nativeBackgroundColor, in nativeTintColor) != 0;
    }

    public static bool ImageButton(string id, ISampledImage sampledImage, Vector2 imageSize, Vector2? uv0 = null, Vector2? uv1 = null, Vector4? backgroundColor = null, Vector4? tintColor = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(id);
        ArgumentNullException.ThrowIfNull(sampledImage);
        var nativeImageSize = NativeFloat2.FromPublic(imageSize);
        var nativeUv0 = NativeFloat2.FromPublic(uv0 ?? Vector2.Zero);
        var nativeUv1 = NativeFloat2.FromPublic(uv1 ?? Vector2.One);
        var nativeBackgroundColor = NativeFloat4.FromPublic(backgroundColor ?? Vector4.Zero);
        var nativeTintColor = NativeFloat4.FromPublic(tintColor ?? Vector4.One);
        return ImGuiNative.ImageButtonSampledImage(id, NativeSampledImage.GetNativeSampledImagePointer(sampledImage), in nativeImageSize, in nativeUv0, in nativeUv1, in nativeBackgroundColor, in nativeTintColor) != 0;
    }

    public static bool InputText(string label, ref string value, InputTextFlags flags = InputTextFlags.None, InputTextCallback? callback = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(label);
        value ??= string.Empty;
        ValidateInputTextFlags(flags);
        using var context = callback is null ? null : new InputTextCallbackContext(callback);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.InputText(label, value, (uint)flags, context?.NativeCallback, context?.Userdata ?? IntPtr.Zero, out var changed, out var outValue)));
        value = CopyAndFreeString(outValue);
        context?.ThrowPendingExceptionIfAny();
        return changed != 0;
    }

    public static bool InputTextMultiline(string label, ref string value, Vector2? size = null, InputTextFlags flags = InputTextFlags.None, InputTextCallback? callback = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(label);
        value ??= string.Empty;
        ValidateInputTextFlags(flags);
        var nativeSize = NativeFloat2.FromPublic(size ?? Vector2.Zero);
        using var context = callback is null ? null : new InputTextCallbackContext(callback);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.InputTextMultiline(label, value, in nativeSize, (uint)flags, context?.NativeCallback, context?.Userdata ?? IntPtr.Zero, out var changed, out var outValue)));
        value = CopyAndFreeString(outValue);
        context?.ThrowPendingExceptionIfAny();
        return changed != 0;
    }

    public static bool InputTextWithHint(string label, string hint, ref string value, InputTextFlags flags = InputTextFlags.None, InputTextCallback? callback = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(label);
        ArgumentNullException.ThrowIfNull(hint);
        value ??= string.Empty;
        ValidateInputTextFlags(flags);
        using var context = callback is null ? null : new InputTextCallbackContext(callback);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.InputTextWithHint(label, hint, value, (uint)flags, context?.NativeCallback, context?.Userdata ?? IntPtr.Zero, out var changed, out var outValue)));
        value = CopyAndFreeString(outValue);
        context?.ThrowPendingExceptionIfAny();
        return changed != 0;
    }

    public static void Gizmo(ref Matrix4x4 worldMatrix, in Matrix4x4 view, in Matrix4x4 projection, RectF viewportRect, GizmoOperation operation, GizmoMode mode, float snap, bool enabled, bool orthographic, out Matrix4x4 deltaMatrix, out bool isMouseHover, out bool isMouseMoving)
    {
        var nativeWorld = NativeMatrix4x4.FromPublic(worldMatrix);
        var nativeView = NativeMatrix4x4.FromPublic(view);
        var nativeProjection = NativeMatrix4x4.FromPublic(projection);
        ImGuiNative.Gizmo(
            ref nativeWorld,
            in nativeView,
            in nativeProjection,
            in viewportRect,
            (uint)operation,
            (uint)mode,
            snap,
            enabled ? 1 : 0,
            orthographic ? 1 : 0,
            out var nativeDelta,
            out var hover,
            out var moving);
        worldMatrix = nativeWorld.ToPublic();
        deltaMatrix = nativeDelta.ToPublic();
        isMouseHover = hover != 0;
        isMouseMoving = moving != 0;
    }

    public static void Render()
    {
        ImGuiNative.Render();
    }

    private static string CopyAndFreeString(IntPtr value)
    {
        try
        {
            return value == IntPtr.Zero ? string.Empty : Marshal.PtrToStringUTF8(value) ?? string.Empty;
        }
        finally
        {
            if (value != IntPtr.Zero)
            {
                RuntimeNativeGenerated.FreeBuffer(value);
            }
        }
    }

    private static void ValidateInputTextFlags(InputTextFlags flags)
    {
        if ((flags & InputTextFlags.CallbackResize) != 0)
        {
            throw new ArgumentException("InputTextFlags.CallbackResize is managed internally and must not be set explicitly.", nameof(flags));
        }
    }

    private sealed class InputTextCallbackContext : IDisposable
    {
        internal static readonly ImGuiNative.InputTextCallback NativeThunk = Invoke;

        private readonly GCHandle _handle;
        private readonly InputTextCallback _callback;
        private Exception? _pendingException;

        public InputTextCallbackContext(InputTextCallback callback)
        {
            _callback = callback;
            _handle = GCHandle.Alloc(this);
        }

        public ImGuiNative.InputTextCallback NativeCallback => NativeThunk;

        public IntPtr Userdata => GCHandle.ToIntPtr(_handle);

        public void Dispose()
        {
            if (_handle.IsAllocated)
            {
                _handle.Free();
            }
        }

        public void ThrowPendingExceptionIfAny()
        {
            if (_pendingException is null)
            {
                return;
            }
            var exception = _pendingException;
            _pendingException = null;
            throw exception;
        }

        private static int Invoke(IntPtr data, IntPtr userdata)
        {
            var context = (InputTextCallbackContext)GCHandle.FromIntPtr(userdata).Target!;
            try
            {
                return context._callback(new InputTextCallbackData(data));
            }
            catch (Exception ex)
            {
                context._pendingException ??= ex;
                return 0;
            }
        }
    }
}
