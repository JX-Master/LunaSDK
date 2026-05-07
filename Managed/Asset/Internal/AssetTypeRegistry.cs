using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;

namespace Luna.Asset.Internal;

internal static class AssetTypeRegistry
{
    private static readonly Dictionary<string, Registration> s_registrations = new(StringComparer.Ordinal);
    private static readonly object s_lock = new();
    private static readonly UIntPtr s_failureCode = RuntimeNative.ErrorGetCodeByName("BasicError", "failure");

    [ThreadStatic]
    private static Exception? t_pendingException;

    internal static void Register(AssetTypeDesc desc)
    {
        var registration = new Registration(desc);
        lock (s_lock)
        {
            if (s_registrations.TryGetValue(desc.Name, out var previous))
            {
                previous.Dispose();
            }
            s_registrations[desc.Name] = registration;
        }

        AssetNative.RegisterAssetType(
            desc.Name,
            desc.OnLoadAsset is null ? null : Registration.LoadAssetThunk,
            desc.OnLoadAssetDefaultData is null ? null : Registration.LoadAssetDefaultDataThunk,
            desc.OnSaveAsset is null ? null : Registration.SaveAssetThunk,
            desc.OnSetAssetData is null ? null : Registration.SetAssetDataThunk,
            desc.OnGetReferredAssets is null ? null : Registration.GetReferredAssetsThunk,
            registration.Userdata);
    }

    internal static void ThrowPendingExceptionIfAny()
    {
        var exception = t_pendingException;
        t_pendingException = null;
        if (exception is not null)
        {
            throw exception;
        }
    }

    private static UIntPtr CaptureException(Exception ex)
    {
        t_pendingException ??= ex;
        return s_failureCode;
    }

    private sealed class Registration : IDisposable
    {
        private readonly GCHandle _gcHandle;
        internal readonly AssetTypeDesc Desc;

        internal static readonly AssetNative.OnLoadAsset LoadAssetThunk = OnLoadAsset;
        internal static readonly AssetNative.OnLoadAssetDefaultData LoadAssetDefaultDataThunk = OnLoadAssetDefaultData;
        internal static readonly AssetNative.OnSaveAsset SaveAssetThunk = OnSaveAsset;
        internal static readonly AssetNative.OnSetAssetData SetAssetDataThunk = OnSetAssetData;
        internal static readonly AssetNative.OnGetReferredAssets GetReferredAssetsThunk = OnGetReferredAssets;

        internal Registration(AssetTypeDesc desc)
        {
            Desc = desc;
            _gcHandle = GCHandle.Alloc(this);
        }

        internal IntPtr Userdata => GCHandle.ToIntPtr(_gcHandle);

        public void Dispose()
        {
            if (_gcHandle.IsAllocated)
            {
                _gcHandle.Free();
            }
        }

        private static Registration GetRegistration(IntPtr userdata)
        {
            return (Registration)GCHandle.FromIntPtr(userdata).Target!;
        }

        private static UIntPtr OnLoadAsset(NativeAssetHandle asset, IntPtr path, out IntPtr outData, IntPtr userdata)
        {
            outData = IntPtr.Zero;
            try
            {
                var registration = GetRegistration(userdata);
                var managed = registration.Desc.OnLoadAsset!(asset.ToManaged(), Marshal.PtrToStringUTF8(path) ?? string.Empty);
                outData = managed?.GetNativeHandle() ?? IntPtr.Zero;
                return UIntPtr.Zero;
            }
            catch (Exception ex)
            {
                return CaptureException(ex);
            }
        }

        private static UIntPtr OnLoadAssetDefaultData(NativeAssetHandle asset, out IntPtr outData, IntPtr userdata)
        {
            outData = IntPtr.Zero;
            try
            {
                var registration = GetRegistration(userdata);
                var managed = registration.Desc.OnLoadAssetDefaultData!(asset.ToManaged());
                outData = managed?.GetNativeHandle() ?? IntPtr.Zero;
                return UIntPtr.Zero;
            }
            catch (Exception ex)
            {
                return CaptureException(ex);
            }
        }

        private static UIntPtr OnSaveAsset(NativeAssetHandle asset, IntPtr path, IntPtr data, IntPtr userdata)
        {
            try
            {
                var registration = GetRegistration(userdata);
                using var managedData = data == IntPtr.Zero ? null : new NativeAssetObject(data, retain: true);
                registration.Desc.OnSaveAsset!(asset.ToManaged(), Marshal.PtrToStringUTF8(path) ?? string.Empty, managedData!);
                return UIntPtr.Zero;
            }
            catch (Exception ex)
            {
                return CaptureException(ex);
            }
        }

        private static UIntPtr OnSetAssetData(NativeAssetHandle asset, IntPtr data, IntPtr userdata)
        {
            try
            {
                var registration = GetRegistration(userdata);
                using var managedData = data == IntPtr.Zero ? null : new NativeAssetObject(data, retain: true);
                registration.Desc.OnSetAssetData!(asset.ToManaged(), managedData);
                return UIntPtr.Zero;
            }
            catch (Exception ex)
            {
                return CaptureException(ex);
            }
        }

        private static void OnGetReferredAssets(NativeAssetHandle asset, IntPtr outAssets, ulong capacity, out ulong outCount, IntPtr userdata)
        {
            outCount = 0;
            try
            {
                var registration = GetRegistration(userdata);
                var assets = registration.Desc.OnGetReferredAssets!(asset.ToManaged()) ?? Array.Empty<AssetHandle>();
                outCount = (ulong)assets.Length;
                if (outAssets == IntPtr.Zero || capacity < outCount)
                {
                    return;
                }

                var native = new NativeAssetHandle[assets.Length];
                for (var i = 0; i < assets.Length; ++i)
                {
                    native[i] = NativeAssetHandle.FromManaged(assets[i]);
                }
                Marshal.StructureToPtr(native[0], outAssets, false);
                var stride = Marshal.SizeOf<NativeAssetHandle>();
                for (var i = 1; i < native.Length; ++i)
                {
                    Marshal.StructureToPtr(native[i], outAssets + (i * stride), false);
                }
            }
            catch (Exception ex)
            {
                t_pendingException ??= ex;
            }
        }
    }
}
