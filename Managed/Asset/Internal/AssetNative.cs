using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using AssetGuid = Luna.Runtime.Guid;

namespace Luna.Asset.Internal;

internal static class AssetNative
{
    private const string LibraryName = "LunaAssetC";

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate UIntPtr OnLoadAsset(NativeAssetHandle asset, IntPtr path, out IntPtr outData, IntPtr userdata);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate UIntPtr OnLoadAssetDefaultData(NativeAssetHandle asset, out IntPtr outData, IntPtr userdata);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate UIntPtr OnSaveAsset(NativeAssetHandle asset, IntPtr path, IntPtr data, IntPtr userdata);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate UIntPtr OnSetAssetData(NativeAssetHandle asset, IntPtr data, IntPtr userdata);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void OnGetReferredAssets(NativeAssetHandle asset, IntPtr outAssets, ulong capacity, out ulong outCount, IntPtr userdata);

    [DllImport(LibraryName, EntryPoint = "luna_asset_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_asset_free_string")]
    internal static extern void FreeString(IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_asset_register_asset_type")]
    internal static extern void RegisterAssetType(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string type,
        OnLoadAsset? onLoadAsset,
        OnLoadAssetDefaultData? onLoadAssetDefaultData,
        OnSaveAsset? onSaveAsset,
        OnSetAssetData? onSetAssetData,
        OnGetReferredAssets? onGetReferredAssets,
        IntPtr userdata);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset")]
    internal static extern UIntPtr GetAsset(
        in AssetGuid guid,
        out NativeAssetHandle outAsset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_register_asset")]
    internal static extern UIntPtr RegisterAsset(
        NativeAssetHandle asset,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string type);

    [DllImport(LibraryName, EntryPoint = "luna_asset_new_asset")]
    internal static extern UIntPtr NewAsset(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string type,
        int saveMetaToFile,
        out NativeAssetHandle outAsset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_load_assets_meta")]
    internal static extern UIntPtr LoadAssetsMeta(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        int allowOverwrite);

    [DllImport(LibraryName, EntryPoint = "luna_asset_load_asset_meta")]
    internal static extern UIntPtr LoadAssetMeta(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_save_asset_meta")]
    internal static extern UIntPtr SaveAssetMeta(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_by_path")]
    internal static extern UIntPtr GetAssetByPath(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path,
        out NativeAssetHandle outAsset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_guid")]
    internal static extern void GetAssetGuid(NativeAssetHandle asset, out AssetGuid outGuid);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_path")]
    internal static extern UIntPtr GetAssetPath(NativeAssetHandle asset, out IntPtr outPath);

    [DllImport(LibraryName, EntryPoint = "luna_asset_set_asset_path")]
    internal static extern UIntPtr SetAssetPath(
        NativeAssetHandle asset,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string path);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_name")]
    internal static extern UIntPtr GetAssetName(NativeAssetHandle asset, out IntPtr outName);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_type")]
    internal static extern UIntPtr GetAssetType(NativeAssetHandle asset, out IntPtr outType);

    [DllImport(LibraryName, EntryPoint = "luna_asset_set_asset_type")]
    internal static extern void SetAssetType(
        NativeAssetHandle asset,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string type);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_files_count")]
    internal static extern UIntPtr GetAssetFilesCount(NativeAssetHandle asset, out ulong outCount);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_file")]
    internal static extern UIntPtr GetAssetFile(NativeAssetHandle asset, ulong index, out IntPtr outName);

    [DllImport(LibraryName, EntryPoint = "luna_asset_delete_asset")]
    internal static extern UIntPtr DeleteAsset(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_move_asset")]
    internal static extern UIntPtr MoveAsset(
        NativeAssetHandle asset,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string newPath);

    [DllImport(LibraryName, EntryPoint = "luna_asset_copy_asset")]
    internal static extern UIntPtr CopyAsset(
        NativeAssetHandle asset,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string newPath,
        in AssetGuid guid,
        out NativeAssetHandle outAsset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_data")]
    internal static extern IntPtr GetAssetData(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_set_asset_data")]
    internal static extern UIntPtr SetAssetData(NativeAssetHandle asset, IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_asset_load_asset")]
    internal static extern UIntPtr LoadAsset(NativeAssetHandle asset, int forceReload);

    [DllImport(LibraryName, EntryPoint = "luna_asset_load_asset_default_data")]
    internal static extern UIntPtr LoadAssetDefaultData(NativeAssetHandle asset, int forceReload);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_asset_state")]
    internal static extern uint GetAssetState(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_save_asset")]
    internal static extern UIntPtr SaveAsset(NativeAssetHandle asset);

    [DllImport(LibraryName, EntryPoint = "luna_asset_get_referred_assets")]
    internal static extern void GetReferredAssets(
        NativeAssetHandle asset,
        [Out] NativeAssetHandle[] outAssets,
        ulong capacity,
        out ulong outCount);
}
