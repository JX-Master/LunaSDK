using Luna.Runtime;

namespace Luna.Asset;

public static class Errors
{
    public static ErrorCategory Category => RuntimeErrors.GetCategoryByName("AssetError");

    public static ErrorCode MetaFileNotFound => RuntimeErrors.GetCodeByName("AssetError", "meta_file_not_found");

    public static ErrorCode UnknownAssetType => RuntimeErrors.GetCodeByName("AssetError", "unknown_asset_type");

    public static ErrorCode AssetNotRegistered => RuntimeErrors.GetCodeByName("AssetError", "asset_not_registered");

    public static ErrorCode AssetAlreadyRegistered => RuntimeErrors.GetCodeByName("AssetError", "asset_already_registered");

    public static ErrorCode EmptyAssetPath => RuntimeErrors.GetCodeByName("AssetError", "empty_asset_path");

    public static ErrorCode AssetDataNotLoaded => RuntimeErrors.GetCodeByName("AssetError", "asset_data_not_loaded");

    public static ErrorCode AssetDataLoading => RuntimeErrors.GetCodeByName("AssetError", "asset_data_loading");
}
