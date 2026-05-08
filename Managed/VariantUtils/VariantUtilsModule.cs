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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.InitModule()));
    }

    public static Variant ReadJson(string source)
    {
        ArgumentException.ThrowIfNullOrEmpty(source);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.ReadJson(source, ulong.MaxValue, out var variant)));
        return Variant.FromNative(variant);
    }

    public static Variant ReadJson(IStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.ReadJsonStream(stream.GetNativeHandle(), out var variant)));
        return Variant.FromNative(variant);
    }

    public static string WriteJson(Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.WriteJson(variant.Handle, indent ? 1 : 0, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNative.FreeString(text);
        }
    }

    public static void WriteJson(IStream stream, Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.WriteJsonStream(stream.GetNativeHandle(), variant.Handle, indent ? 1 : 0)));
    }

    public static Variant NewXmlElement(string name)
    {
        ArgumentException.ThrowIfNullOrEmpty(name);
        return Variant.FromNative(VariantUtilsNative.NewXmlElement(name));
    }

    public static string GetXmlName(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.GetXmlName(xmlElement.Handle, out var name)));
        try
        {
            return Marshal.PtrToStringUTF8(name) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNative.FreeString(name);
        }
    }

    public static void SetXmlName(Variant xmlElement, string name)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentException.ThrowIfNullOrEmpty(name);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.SetXmlName(xmlElement.Handle, name)));
    }

    public static Variant GetXmlAttributes(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        return Variant.FromNative(VariantUtilsNative.GetXmlAttributes(xmlElement.Handle));
    }

    public static void SetXmlAttributes(Variant xmlElement, Variant attributes)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentNullException.ThrowIfNull(attributes);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.SetXmlAttributes(xmlElement.Handle, attributes.Handle)));
    }

    public static Variant GetXmlContent(Variant xmlElement)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        return Variant.FromNative(VariantUtilsNative.GetXmlContent(xmlElement.Handle));
    }

    public static void SetXmlContent(Variant xmlElement, Variant content)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentNullException.ThrowIfNull(content);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.SetXmlContent(xmlElement.Handle, content.Handle)));
    }

    public static Variant? FindFirstXmlChildElement(Variant xmlElement, string name, ulong startIndex, out ulong index)
    {
        ArgumentNullException.ThrowIfNull(xmlElement);
        ArgumentException.ThrowIfNullOrEmpty(name);
        var child = VariantUtilsNative.FindFirstXmlChildElement(xmlElement.Handle, name, startIndex, out index);
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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.ReadXml(bytes, (ulong)bytes.Length, out var variant)));
        return Variant.FromNative(variant);
    }

    public static Variant ReadXml(IStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.ReadXmlStream(stream.GetNativeHandle(), out var variant)));
        return Variant.FromNative(variant);
    }

    public static string WriteXml(Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.WriteXml(variant.Handle, indent ? 1 : 0, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNative.FreeString(text);
        }
    }

    public static void WriteXml(IStream stream, Variant variant, bool indent = true)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(variant);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.WriteXmlStream(stream.GetNativeHandle(), variant.Handle, indent ? 1 : 0)));
    }

    public static Variant Diff(Variant before, Variant after)
    {
        ArgumentNullException.ThrowIfNull(before);
        ArgumentNullException.ThrowIfNull(after);
        return Variant.FromNative(VariantUtilsNative.Diff(before.Handle, after.Handle));
    }

    public static void Patch(Variant before, Variant delta)
    {
        ArgumentNullException.ThrowIfNull(before);
        ArgumentNullException.ThrowIfNull(delta);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.Patch(before.Handle, delta.Handle)));
    }

    public static void Revert(Variant after, Variant delta)
    {
        ArgumentNullException.ThrowIfNull(after);
        ArgumentNullException.ThrowIfNull(delta);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.Revert(after.Handle, delta.Handle)));
    }

    public static void AddDiffPrefix(Variant delta, params Variant[] prefixNodes)
    {
        ArgumentNullException.ThrowIfNull(delta);
        prefixNodes ??= Array.Empty<Variant>();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNative.AddDiffPrefix(
            delta.Handle,
            Variant.ToNativeArray(prefixNodes),
            (ulong)prefixNodes.Length)));
    }
}
