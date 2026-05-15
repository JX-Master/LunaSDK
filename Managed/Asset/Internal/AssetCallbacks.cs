using System;
using System.Runtime.InteropServices;

namespace Luna.Asset.Internal;

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
