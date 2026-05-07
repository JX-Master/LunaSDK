using System;
using Luna.Runtime;

namespace Luna.Asset;

public sealed class AssetTypeDesc
{
    public required string Name { get; init; }

    public Func<AssetHandle, string, IObject?>? OnLoadAsset { get; init; }

    public Func<AssetHandle, IObject?>? OnLoadAssetDefaultData { get; init; }

    public Action<AssetHandle, string, IObject>? OnSaveAsset { get; init; }

    public Action<AssetHandle, IObject?>? OnSetAssetData { get; init; }

    public Func<AssetHandle, AssetHandle[]>? OnGetReferredAssets { get; init; }
}
