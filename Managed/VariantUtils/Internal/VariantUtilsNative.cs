using System;
using System.Runtime.InteropServices;

namespace Luna.VariantUtils.Internal;

internal static class VariantUtilsNative
{
    private const string LibraryName = "LunaVariantUtilsC";

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_free_string")]
    internal static extern void FreeString(IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_read_json")]
    internal static extern UIntPtr ReadJson([MarshalAs(UnmanagedType.LPUTF8Str)] string source, ulong sourceSize, out NativeVariantHandle outVariant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_read_json_stream")]
    internal static extern UIntPtr ReadJsonStream(IntPtr streamObject, out NativeVariantHandle outVariant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_write_json")]
    internal static extern UIntPtr WriteJson(NativeVariantHandle variant, int indent, out IntPtr outString);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_write_json_stream")]
    internal static extern UIntPtr WriteJsonStream(IntPtr streamObject, NativeVariantHandle variant, int indent);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_new_xml_element")]
    internal static extern NativeVariantHandle NewXmlElement([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_get_xml_name")]
    internal static extern UIntPtr GetXmlName(NativeVariantHandle xmlElement, out IntPtr outName);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_set_xml_name")]
    internal static extern UIntPtr SetXmlName(NativeVariantHandle xmlElement, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_get_xml_attributes")]
    internal static extern NativeVariantHandle GetXmlAttributes(NativeVariantHandle xmlElement);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_set_xml_attributes")]
    internal static extern UIntPtr SetXmlAttributes(NativeVariantHandle xmlElement, NativeVariantHandle attributes);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_get_xml_content")]
    internal static extern NativeVariantHandle GetXmlContent(NativeVariantHandle xmlElement);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_set_xml_content")]
    internal static extern UIntPtr SetXmlContent(NativeVariantHandle xmlElement, NativeVariantHandle content);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_find_first_xml_child_element")]
    internal static extern NativeVariantHandle FindFirstXmlChildElement(NativeVariantHandle xmlElement, [MarshalAs(UnmanagedType.LPUTF8Str)] string name, ulong startIndex, out ulong outIndex);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_read_xml")]
    internal static extern UIntPtr ReadXml([In] byte[] source, ulong sourceSize, out NativeVariantHandle outVariant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_read_xml_stream")]
    internal static extern UIntPtr ReadXmlStream(IntPtr streamObject, out NativeVariantHandle outVariant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_write_xml")]
    internal static extern UIntPtr WriteXml(NativeVariantHandle variant, int indent, out IntPtr outString);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_write_xml_stream")]
    internal static extern UIntPtr WriteXmlStream(IntPtr streamObject, NativeVariantHandle variant, int indent);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_diff")]
    internal static extern NativeVariantHandle Diff(NativeVariantHandle before, NativeVariantHandle after);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_patch")]
    internal static extern UIntPtr Patch(NativeVariantHandle before, NativeVariantHandle delta);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_revert")]
    internal static extern UIntPtr Revert(NativeVariantHandle after, NativeVariantHandle delta);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_add_diff_prefix")]
    internal static extern UIntPtr AddDiffPrefix(NativeVariantHandle delta, [In] NativeVariantHandle[] prefixNodes, ulong count);
}
