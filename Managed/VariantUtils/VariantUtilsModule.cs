using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;
using Luna.VariantUtils.Internal;

namespace Luna.VariantUtils;

public static class VariantUtilsModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the VariantUtils module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.InitModule()));
    }

    public static Variant ReadJson(string source)
    {
        ArgumentException.ThrowIfNullOrEmpty(source);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.ReadJson(source, ulong.MaxValue, out var variant)));
        return Variant.FromNative(variant);
    }

    public static Variant ReadJson(IStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.ReadJsonStream(stream.GetNativeHandle(), out var variant)));
        return Variant.FromNative(variant);
    }

    public static string WriteJson(Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.WriteJson(variant.Handle, indent ? 1 : 0, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNativeGenerated.FreeString(text);
        }
    }

    public static void WriteJson(IStream stream, Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.WriteJsonStream(stream.GetNativeHandle(), variant.Handle, indent ? 1 : 0)));
    }

    public static Variant NewXmlElement(string name)
    {
        ArgumentException.ThrowIfNullOrEmpty(name);
        return Variant.FromNative(VariantUtilsNativeGenerated.NewXmlElement(name));
    }

    public static string GetXmlName(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.GetXmlName(xmlElement.Handle, out var name)));
        try
        {
            return Marshal.PtrToStringUTF8(name) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNativeGenerated.FreeString(name);
        }
    }

    public static void SetXmlName(Variant xmlElement, string name)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentException.ThrowIfNullOrEmpty(name);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.SetXmlName(xmlElement.Handle, name)));
    }

    public static Variant GetXmlAttributes(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        return Variant.FromNative(VariantUtilsNativeGenerated.GetXmlAttributes(xmlElement.Handle));
    }

    public static void SetXmlAttributes(Variant xmlElement, Variant attributes)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentNullException.ThrowIfNull(attributes);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.SetXmlAttributes(xmlElement.Handle, attributes.Handle)));
    }

    public static Variant GetXmlContent(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        return Variant.FromNative(VariantUtilsNativeGenerated.GetXmlContent(xmlElement.Handle));
    }

    public static void SetXmlContent(Variant xmlElement, Variant content)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentNullException.ThrowIfNull(content);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.SetXmlContent(xmlElement.Handle, content.Handle)));
    }

    public static Variant? FindFirstXmlChildElement(Variant xmlElement, string name, ulong startIndex, out ulong index)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentException.ThrowIfNullOrEmpty(name);
        var child = VariantUtilsNativeGenerated.FindFirstXmlChildElement(xmlElement.Handle, name, startIndex, out index);
        var result = Variant.FromNative(child);
        if (!result.IsValid)
        {
            result.Dispose();
            return null;
        }
        return result;
    }

    public static Variant ReadXml(string source)
    {
        ArgumentException.ThrowIfNullOrEmpty(source);
        var bytes = System.Text.Encoding.UTF8.GetBytes(source);
        using var pinnedBytes = PinnedByteArray.Create(bytes);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.ReadXml(pinnedBytes.Pointer, (ulong)bytes.Length, out var variant)));
        return Variant.FromNative(variant);
    }

    public static Variant ReadXml(IStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.ReadXmlStream(stream.GetNativeHandle(), out var variant)));
        return Variant.FromNative(variant);
    }

    public static string WriteXml(Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.WriteXml(variant.Handle, indent ? 1 : 0, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNativeGenerated.FreeString(text);
        }
    }

    public static void WriteXml(IStream stream, Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.WriteXmlStream(stream.GetNativeHandle(), variant.Handle, indent ? 1 : 0)));
    }

    public static Variant Diff(Variant before, Variant after)
    {
        ArgumentNullException.ThrowIfNull(before);
        ArgumentNullException.ThrowIfNull(after);
        return Variant.FromNative(VariantUtilsNativeGenerated.Diff(before.Handle, after.Handle));
    }

    public static void Patch(Variant before, Variant delta)
    {
        ArgumentNullException.ThrowIfNull(before);
        ArgumentNullException.ThrowIfNull(delta);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.Patch(before.Handle, delta.Handle)));
    }

    public static void Revert(Variant after, Variant delta)
    {
        ArgumentNullException.ThrowIfNull(after);
        ArgumentNullException.ThrowIfNull(delta);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.Revert(after.Handle, delta.Handle)));
    }

    public static void AddDiffPrefix(Variant delta, params Variant[] prefixNodes)
    {
        ArgumentNullException.ThrowIfNull(delta);
        prefixNodes ??= Array.Empty<Variant>();
        var nativePrefixNodes = Variant.ToNativeArray(prefixNodes);
        using var pinnedPrefixNodes = PinnedVariantHandleArray.Create(nativePrefixNodes);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.AddDiffPrefix(
            delta.Handle,
            pinnedPrefixNodes.Pointer,
            (ulong)prefixNodes.Length)));
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle _handle;

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            _handle = handle;
            Pointer = pointer;
        }

        public IntPtr Pointer { get; }

        public static PinnedByteArray Create(byte[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (_handle.IsAllocated)
            {
                _handle.Free();
            }
        }
    }

    private readonly struct PinnedVariantHandleArray : IDisposable
    {
        private readonly GCHandle _handle;

        private PinnedVariantHandleArray(GCHandle handle, IntPtr pointer)
        {
            _handle = handle;
            Pointer = pointer;
        }

        public IntPtr Pointer { get; }

        public static PinnedVariantHandleArray Create(NativeVariantHandle[] handles)
        {
            if (handles.Length == 0)
            {
                return new PinnedVariantHandleArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(handles, GCHandleType.Pinned);
            return new PinnedVariantHandleArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (_handle.IsAllocated)
            {
                _handle.Free();
            }
        }
    }
}
