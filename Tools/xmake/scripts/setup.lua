import("net.http")
import("utils.archive")

local sdk_packages = {
    macosx = {
        name = "macosx",
        root = "SDKs-macosx",
        url = "https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v5/SDKs-v5-macosx.zip"
    },
    windows = {
        name = "windows",
        root = "SDKs-windows",
        url = "https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v5/SDKs-v5-windows.zip"
    }
}

local function find_package()
    local host = os.host()
    local package = sdk_packages[host]
    if package == nil then
        assert(false, string.format("unsupported SDK host platform: %s", host))
    end
    return package
end

local function copy_sdk_payload(extracted_dir, package)
    local sdk_payload = path.join(extracted_dir, "SDKs")
    if not os.isdir(sdk_payload) then
        sdk_payload = path.join(extracted_dir, package.root)
    end
    if not os.isdir(sdk_payload) then
        sdk_payload = extracted_dir
    end

    os.mkdir("SDKs")
    for _, item in ipairs(os.dirs(path.join(sdk_payload, "*"))) do
        os.cp(item, "SDKs")
    end
    for _, item in ipairs(os.files(path.join(sdk_payload, "*"))) do
        os.cp(item, "SDKs")
    end
end

function main(...)
    local arguments = {...}
    local package = find_package()
    local tmp_dir = path.join("tmp", "setup-sdk-" .. package.name)
    local archive_file = path.join(tmp_dir, "SDKs-" .. package.name .. ".zip")
    local extracted_dir = path.join(tmp_dir, "extracted")

    print(string.format("setting up LunaSDK third-party SDKs for %s", package.name))
    print(string.format("SDK package URL: %s", package.url))

    if os.isdir(tmp_dir) then
        os.rmdir(tmp_dir)
    end
    os.mkdir(tmp_dir)
    http.download(package.url, archive_file)
    archive.extract(archive_file, extracted_dir)
    copy_sdk_payload(extracted_dir, package)
    os.rmdir(tmp_dir)
    if os.isdir("tmp") and #os.dirs(path.join("tmp", "*")) == 0 and #os.files(path.join("tmp", "*")) == 0 then
        os.rmdir("tmp")
    end
end
