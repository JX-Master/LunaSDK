using System;
using System.Runtime.InteropServices;
using Luna.Asset.Internal;
using Luna.Runtime;
using AssetGuid = Luna.Runtime.Guid;

namespace Luna.Asset;

public static class AssetModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the Asset module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.InitModule()));
    }

    public static AssetHandle GetAsset(AssetGuid guid)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAsset(in guid, out var asset)));
        return asset.ToManaged();
    }

    public static AssetHandle CreateAssetHandle()
    {
        return GetAsset(default);
    }

    public static void RegisterAssetType(AssetTypeDesc desc)
    {
        ArgumentNullException.ThrowIfNull(desc);
        ArgumentException.ThrowIfNullOrEmpty(desc.Name);
        AssetTypeRegistry.Register(desc);
    }

    public static void RegisterAsset(AssetHandle asset, string type)
    {
        ValidateAsset(asset);
        ArgumentException.ThrowIfNullOrEmpty(type);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.RegisterAsset(NativeAssetHandle.FromManaged(asset), type)));
    }

    public static AssetHandle NewAsset(string path, string type, bool saveMetaToFile = true)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentException.ThrowIfNullOrEmpty(type);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.NewAsset(path, type, saveMetaToFile ? 1 : 0, out var asset)));
        return asset.ToManaged();
    }

    public static void LoadAssetsMeta(string path, bool allowOverwrite = true)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.LoadAssetsMeta(path, allowOverwrite ? 1 : 0)));
    }

    public static void LoadAssetMeta(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.LoadAssetMeta(NativeAssetHandle.FromManaged(asset))));
    }

    public static void SaveAssetMeta(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.SaveAssetMeta(NativeAssetHandle.FromManaged(asset))));
    }

    public static AssetHandle GetAssetByPath(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetByPath(path, out var asset)));
        return asset.ToManaged();
    }

    public static AssetGuid GetAssetGuid(AssetHandle asset)
    {
        ValidateAsset(asset);
        AssetNative.GetAssetGuid(NativeAssetHandle.FromManaged(asset), out var guid);
        return guid;
    }

    public static string GetAssetPath(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetPath(NativeAssetHandle.FromManaged(asset), out var path)));
        return PtrToManagedAndFree(path);
    }

    public static void SetAssetPath(AssetHandle asset, string path)
    {
        ValidateAsset(asset);
        ArgumentException.ThrowIfNullOrEmpty(path);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.SetAssetPath(NativeAssetHandle.FromManaged(asset), path)));
    }

    public static string GetAssetName(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetName(NativeAssetHandle.FromManaged(asset), out var name)));
        return PtrToManagedAndFree(name);
    }

    public static string GetAssetType(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetType(NativeAssetHandle.FromManaged(asset), out var type)));
        return PtrToManagedAndFree(type);
    }

    public static void SetAssetType(AssetHandle asset, string type)
    {
        ValidateAsset(asset);
        ArgumentException.ThrowIfNullOrEmpty(type);
        AssetNative.SetAssetType(NativeAssetHandle.FromManaged(asset), type);
    }

    public static string[] GetAssetFiles(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetFilesCount(NativeAssetHandle.FromManaged(asset), out var count)));
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("Too many asset files to copy into a managed array.");
        }
        var result = new string[(int)count];
        for (ulong i = 0; i < count; ++i)
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.GetAssetFile(NativeAssetHandle.FromManaged(asset), i, out var name)));
            result[i] = PtrToManagedAndFree(name);
        }
        return result;
    }

    public static void DeleteAsset(AssetHandle asset)
    {
        ValidateAsset(asset);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.DeleteAsset(NativeAssetHandle.FromManaged(asset))));
    }

    public static void MoveAsset(AssetHandle asset, string newPath)
    {
        ValidateAsset(asset);
        ArgumentException.ThrowIfNullOrEmpty(newPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.MoveAsset(NativeAssetHandle.FromManaged(asset), newPath)));
    }

    public static AssetHandle CopyAsset(AssetHandle asset, string newPath, AssetGuid guid = default)
    {
        ValidateAsset(asset);
        ArgumentException.ThrowIfNullOrEmpty(newPath);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AssetNative.CopyAsset(NativeAssetHandle.FromManaged(asset), newPath, in guid, out var newAsset)));
        return newAsset.ToManaged();
    }

    public static IObject? GetAssetData(AssetHandle asset)
    {
        ValidateAsset(asset);
        var handle = AssetNative.GetAssetData(NativeAssetHandle.FromManaged(asset));
        return handle == IntPtr.Zero ? null : new NativeAssetObject(handle, retain: false);
    }

    public static void SetAssetData(AssetHandle asset, IObject? data)
    {
        ValidateAsset(asset);
        var nativeHandle = data?.GetNativeHandle() ?? IntPtr.Zero;
        var code = new ErrorCode(AssetNative.SetAssetData(NativeAssetHandle.FromManaged(asset), nativeHandle));
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        RuntimeErrors.ThrowIfFailed(code);
    }

    public static void LoadAsset(AssetHandle asset, bool forceReload = false)
    {
        ValidateAsset(asset);
        var code = new ErrorCode(AssetNative.LoadAsset(NativeAssetHandle.FromManaged(asset), forceReload ? 1 : 0));
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        RuntimeErrors.ThrowIfFailed(code);
    }

    public static void LoadAssetDefaultData(AssetHandle asset, bool forceReload = false)
    {
        ValidateAsset(asset);
        var code = new ErrorCode(AssetNative.LoadAssetDefaultData(NativeAssetHandle.FromManaged(asset), forceReload ? 1 : 0));
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        RuntimeErrors.ThrowIfFailed(code);
    }

    public static AssetState GetAssetState(AssetHandle asset)
    {
        return asset.IsValid ? (AssetState)AssetNative.GetAssetState(NativeAssetHandle.FromManaged(asset)) : AssetState.Unregistered;
    }

    public static void SaveAsset(AssetHandle asset)
    {
        ValidateAsset(asset);
        var code = new ErrorCode(AssetNative.SaveAsset(NativeAssetHandle.FromManaged(asset)));
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        RuntimeErrors.ThrowIfFailed(code);
    }

    public static AssetHandle[] GetReferredAssets(AssetHandle asset)
    {
        ValidateAsset(asset);
        AssetNative.GetReferredAssets(NativeAssetHandle.FromManaged(asset), Array.Empty<NativeAssetHandle>(), 0, out var count);
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("Too many referred assets to copy into a managed array.");
        }
        if (count == 0)
        {
            return Array.Empty<AssetHandle>();
        }
        var native = new NativeAssetHandle[(int)count];
        AssetNative.GetReferredAssets(NativeAssetHandle.FromManaged(asset), native, count, out count);
        AssetTypeRegistry.ThrowPendingExceptionIfAny();
        var result = new AssetHandle[(int)count];
        for (var i = 0; i < result.Length; ++i)
        {
            result[i] = native[i].ToManaged();
        }
        return result;
    }

    private static void ValidateAsset(AssetHandle asset)
    {
        if (!asset.IsValid)
        {
            throw new ArgumentException("Asset handle is invalid.", nameof(asset));
        }
    }

    private static string PtrToManagedAndFree(IntPtr value)
    {
        if (value == IntPtr.Zero)
        {
            return string.Empty;
        }
        try
        {
            return Marshal.PtrToStringUTF8(value) ?? string.Empty;
        }
        finally
        {
            AssetNative.FreeString(value);
        }
    }
}
