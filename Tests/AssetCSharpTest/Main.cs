using System;
using System.IO;
using System.Linq;
using System.Text;
using Luna.Asset;
using Luna.Font;
using Luna.Runtime;
using Luna.VFS;
using AssetErrors = Luna.Asset.Errors;

Runtime.Init();

try
{
    VfsModule.Init();
    AssetModule.Init();
    FontModule.Init();

    if (RuntimeErrors.GetCategoryName(AssetErrors.Category) != "AssetError")
    {
        throw new InvalidOperationException("Asset error category lookup failed.");
    }
    if (RuntimeErrors.GetCodeName(AssetErrors.AssetNotRegistered) != "asset_not_registered")
    {
        throw new InvalidOperationException("Asset asset_not_registered lookup failed.");
    }

    var nativeRoot = Path.Combine(Path.GetTempPath(), $"LunaAssetCSharpTest-{System.Guid.NewGuid():N}");
    var mountPath = $"/managed-asset-{System.Guid.NewGuid():N}";
    Directory.CreateDirectory(nativeRoot);

    try
    {
        VfsModule.Mount(VfsModule.PlatformFilesystemDriver, nativeRoot, mountPath);
        try
        {
            var scratchHandle = AssetModule.CreateAssetHandle();
            if (AssetModule.GetAssetState(scratchHandle) != AssetState.Unregistered)
            {
                throw new InvalidOperationException("Fresh asset handle should start unregistered.");
            }
            AssetModule.RegisterAsset(scratchHandle, "ManagedScratchType");
            if (AssetModule.GetAssetState(scratchHandle) != AssetState.Unloaded)
            {
                throw new InvalidOperationException("Registered asset handle should be unloaded.");
            }

            var assetPath = $"{mountPath}/sample.asset";
            var assetBytes = Encoding.UTF8.GetBytes("managed asset payload");
            using (var file = VfsModule.OpenFile(assetPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
            {
                if (file.Write(assetBytes) != (ulong)assetBytes.Length)
                {
                    throw new InvalidOperationException("Failed to write asset payload.");
                }
                file.Flush();
            }

            var asset = AssetModule.NewAsset(assetPath, "ManagedAssetType");
            if (!asset.IsValid)
            {
                throw new InvalidOperationException("NewAsset should return a valid handle.");
            }
            if (AssetModule.GetAssetState(asset) != AssetState.Unloaded)
            {
                throw new InvalidOperationException("New asset should start unloaded.");
            }
            if (AssetModule.GetAssetPath(asset) != assetPath)
            {
                throw new InvalidOperationException("Asset path mismatch.");
            }
            if (AssetModule.GetAssetType(asset) != "ManagedAssetType")
            {
                throw new InvalidOperationException("Asset type mismatch.");
            }
            if (AssetModule.GetAssetName(asset) != "sample")
            {
                throw new InvalidOperationException("Asset name mismatch.");
            }

            var guid = AssetModule.GetAssetGuid(asset);
            if (guid == default)
            {
                throw new InvalidOperationException("Asset GUID should not be zero.");
            }

            var byPath = AssetModule.GetAssetByPath(assetPath);
            if (byPath != asset)
            {
                throw new InvalidOperationException("GetAssetByPath should return the existing asset handle.");
            }

            var files = AssetModule.GetAssetFiles(asset);
            if (!files.Contains("sample.asset") || !files.Contains("sample.asset.meta"))
            {
                throw new InvalidOperationException("Asset file listing should contain the data file and metadata file.");
            }

            var metaAttribute = VfsModule.GetFileAttribute($"{assetPath}.meta");
            if ((metaAttribute.Attributes & FileAttributeFlags.Directory) != 0)
            {
                throw new InvalidOperationException("Metadata path should be a file.");
            }

            AssetModule.SetAssetType(asset, "ChangedAssetType");
            if (AssetModule.GetAssetType(asset) != "ChangedAssetType")
            {
                throw new InvalidOperationException("SetAssetType should update in-memory type.");
            }
            AssetModule.LoadAssetMeta(asset);
            if (AssetModule.GetAssetType(asset) != "ManagedAssetType")
            {
                throw new InvalidOperationException("LoadAssetMeta should restore the saved metadata type.");
            }

            var manualAssetPath = $"{mountPath}/manual.asset";
            using (var file = VfsModule.OpenFile(manualAssetPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
            {
                file.Write(new byte[] { 1, 2, 3, 4 });
                file.Flush();
            }
            AssetModule.SetAssetPath(scratchHandle, manualAssetPath);
            AssetModule.SaveAssetMeta(scratchHandle);
            if (AssetModule.GetAssetPath(scratchHandle) != manualAssetPath)
            {
                throw new InvalidOperationException("SetAssetPath should update asset path.");
            }
            if (AssetModule.GetAssetType(scratchHandle) != "ManagedScratchType")
            {
                throw new InvalidOperationException("Registered scratch asset type mismatch.");
            }
            var reloadedScratch = AssetModule.GetAssetByPath(manualAssetPath);
            if (reloadedScratch != scratchHandle)
            {
                throw new InvalidOperationException("Scratch asset path mapping mismatch.");
            }

            var copiedPath = $"{mountPath}/copied.asset";
            var copiedAsset = AssetModule.CopyAsset(asset, copiedPath);
            if (AssetModule.GetAssetPath(copiedAsset) != copiedPath)
            {
                throw new InvalidOperationException("Copied asset path mismatch.");
            }
            if (!VfsModule.GetNativePath(copiedPath).EndsWith("copied.asset", StringComparison.Ordinal))
            {
                throw new InvalidOperationException("Copied asset native path translation mismatch.");
            }

            var movedPath = $"{mountPath}/moved.asset";
            AssetModule.MoveAsset(copiedAsset, movedPath);
            if (AssetModule.GetAssetPath(copiedAsset) != movedPath)
            {
                throw new InvalidOperationException("Moved asset path mismatch.");
            }
            try
            {
                VfsModule.GetFileAttribute(copiedPath);
                throw new InvalidOperationException("Old copied asset path should no longer exist after move.");
            }
            catch (ErrorException)
            {
            }

            AssetModule.DeleteAsset(copiedAsset);
            if (AssetModule.GetAssetState(copiedAsset) != AssetState.Unregistered)
            {
                throw new InvalidOperationException("Deleted asset should become unregistered.");
            }
            try
            {
                VfsModule.GetFileAttribute(movedPath);
                throw new InvalidOperationException("Moved asset file should be deleted.");
            }
            catch (ErrorException)
            {
            }

            var callbackTypeName = $"ManagedCallbackAssetType-{System.Guid.NewGuid():N}";
            var callbackAssetPath = $"{mountPath}/font.asset";
            using var defaultFont = FontModule.GetDefaultFont();
            var defaultFontBytes = defaultFont.GetData();
            using (var file = VfsModule.OpenFile(callbackAssetPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
            {
                file.Write(defaultFontBytes);
                file.Flush();
            }

            var saveCalled = false;
            var setCalled = false;
            AssetHandle[] referredAssets = [asset];
            AssetHandle callbackAsset = default;
            AssetModule.RegisterAssetType(new AssetTypeDesc
            {
                Name = callbackTypeName,
                OnLoadAsset = (loadedAsset, path) =>
                {
                    using var file = VfsModule.OpenFile(path, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting);
                    return FontModule.LoadTtfFontFile(file);
                },
                OnLoadAssetDefaultData = _ => FontModule.GetDefaultFont(),
                OnSaveAsset = (savedAsset, path, data) =>
                {
                    if (savedAsset != callbackAsset)
                    {
                        throw new InvalidOperationException("Save callback asset mismatch.");
                    }
                    if (string.IsNullOrEmpty(path) || data is null)
                    {
                        throw new InvalidOperationException("Save callback should receive a path and non-null data.");
                    }
                    saveCalled = true;
                },
                OnSetAssetData = (updatedAsset, data) =>
                {
                    if (updatedAsset != callbackAsset)
                    {
                        throw new InvalidOperationException("Set callback asset mismatch.");
                    }
                    setCalled = true;
                },
                OnGetReferredAssets = _ => referredAssets
            });

            callbackAsset = AssetModule.NewAsset(callbackAssetPath, callbackTypeName);
            AssetModule.LoadAsset(callbackAsset);
            if (AssetModule.GetAssetState(callbackAsset) != AssetState.Loaded)
            {
                throw new InvalidOperationException("Callback asset should be loaded after LoadAsset.");
            }
            if (AssetModule.GetAssetData(callbackAsset) is null)
            {
                throw new InvalidOperationException("Callback asset data should not be null after LoadAsset.");
            }
            var referred = AssetModule.GetReferredAssets(callbackAsset);
            if (referred.Length != 1 || referred[0] != asset)
            {
                throw new InvalidOperationException("Referred asset callback mismatch.");
            }

            AssetModule.SaveAsset(callbackAsset);
            if (!saveCalled)
            {
                throw new InvalidOperationException("Save callback should be invoked.");
            }

            AssetModule.LoadAssetDefaultData(callbackAsset, forceReload: true);
            if (AssetModule.GetAssetState(callbackAsset) != AssetState.Loaded)
            {
                throw new InvalidOperationException("Callback asset should remain loaded after LoadAssetDefaultData.");
            }
            AssetModule.SetAssetData(callbackAsset, null);
            if (!setCalled)
            {
                throw new InvalidOperationException("SetAssetData callback should be invoked.");
            }
        }
        finally
        {
            VfsModule.Unmount(mountPath);
        }
    }
    finally
    {
        if (Directory.Exists(nativeRoot))
        {
            Directory.Delete(nativeRoot, recursive: true);
        }
    }

    Console.WriteLine("AssetCSharpTest passed.");
}
finally
{
    Runtime.Close();
}
