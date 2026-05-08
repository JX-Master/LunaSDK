using System;
using System.Runtime.InteropServices;

namespace Luna.VariantUtils.Internal;

internal static class VariantNative
{
    private const string LibraryName = "LunaVariantUtilsC";

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_free_string")]
    internal static extern void FreeString(IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_free_buffer")]
    internal static extern void FreeBuffer(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create")]
    internal static extern NativeVariantHandle Create(byte type);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_i64")]
    internal static extern NativeVariantHandle CreateI64(long value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_u64")]
    internal static extern NativeVariantHandle CreateU64(ulong value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_f64")]
    internal static extern NativeVariantHandle CreateF64(double value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_string")]
    internal static extern NativeVariantHandle CreateString([MarshalAs(UnmanagedType.LPUTF8Str)] string value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_boolean")]
    internal static extern NativeVariantHandle CreateBoolean(int value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_create_blob")]
    internal static extern NativeVariantHandle CreateBlob([In] byte[]? data, ulong size, ulong alignment);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_destroy")]
    internal static extern void Destroy(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_clone")]
    internal static extern NativeVariantHandle Clone(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_equals")]
    internal static extern int Equals(NativeVariantHandle lhs, NativeVariantHandle rhs);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_type")]
    internal static extern byte GetType(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_number_type")]
    internal static extern byte GetNumberType(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_valid")]
    internal static extern int Valid(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_size")]
    internal static extern ulong GetSize(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_contains")]
    internal static extern int Contains(NativeVariantHandle variant, [MarshalAs(UnmanagedType.LPUTF8Str)] string key);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_array_item")]
    internal static extern NativeVariantHandle GetArrayItem(NativeVariantHandle variant, ulong index);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_object_item")]
    internal static extern NativeVariantHandle GetObjectItem(NativeVariantHandle variant, [MarshalAs(UnmanagedType.LPUTF8Str)] string key);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_object_key")]
    internal static extern UIntPtr GetObjectKey(NativeVariantHandle variant, ulong index, out IntPtr outKey);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_set_array_item")]
    internal static extern UIntPtr SetArrayItem(NativeVariantHandle variant, ulong index, NativeVariantHandle value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_insert_array_item")]
    internal static extern UIntPtr InsertArrayItem(NativeVariantHandle variant, ulong index, NativeVariantHandle value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_push_back")]
    internal static extern UIntPtr PushBack(NativeVariantHandle variant, NativeVariantHandle value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_erase_array_item")]
    internal static extern UIntPtr EraseArrayItem(NativeVariantHandle variant, ulong index);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_pop_back")]
    internal static extern UIntPtr PopBack(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_set_object_item")]
    internal static extern UIntPtr SetObjectItem(NativeVariantHandle variant, [MarshalAs(UnmanagedType.LPUTF8Str)] string key, NativeVariantHandle value);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_erase_object_item")]
    internal static extern UIntPtr EraseObjectItem(NativeVariantHandle variant, [MarshalAs(UnmanagedType.LPUTF8Str)] string key);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_string")]
    internal static extern UIntPtr GetString(NativeVariantHandle variant, out IntPtr outString);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_i64")]
    internal static extern long GetI64(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_u64")]
    internal static extern ulong GetU64(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_f64")]
    internal static extern double GetF64(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_boolean")]
    internal static extern int GetBoolean(NativeVariantHandle variant);

    [DllImport(LibraryName, EntryPoint = "luna_variant_utils_variant_get_blob")]
    internal static extern UIntPtr GetBlob(NativeVariantHandle variant, out IntPtr outData, out ulong outSize, out ulong outAlignment);
}
