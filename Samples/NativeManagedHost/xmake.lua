local function version_less(a, b)
    local function parts(version)
        local result = {}
        for part in version:gmatch("%d+") do
            table.insert(result, tonumber(part))
        end
        return result
    end

    local av = parts(path.filename(a))
    local bv = parts(path.filename(b))
    local count = math.max(#av, #bv)
    for i = 1, count do
        local ai = av[i] or 0
        local bi = bv[i] or 0
        if ai ~= bi then
            return ai < bi
        end
    end
    return path.filename(a) < path.filename(b)
end

local function get_dotnet_rid()
    if is_plat("windows") then
        if is_arch("x64", "x86_64") then
            return "win-x64"
        elseif is_arch("arm64") then
            return "win-arm64"
        end
    elseif is_plat("macosx") then
        if is_arch("arm64") then
            return "osx-arm64"
        else
            return "osx-x64"
        end
    elseif is_plat("linux") then
        if is_arch("arm64") then
            return "linux-arm64"
        else
            return "linux-x64"
        end
    end
end

local function get_dotnet_host_native_dir()
    local rid = get_dotnet_rid()
    if not rid then
        return nil
    end

    local roots = {}
    if os.getenv("DOTNET_ROOT") then
        table.insert(roots, os.getenv("DOTNET_ROOT"))
    end
    if is_plat("windows") then
        table.insert(roots, path.join(os.getenv("ProgramFiles") or "C:\\Program Files", "dotnet"))
    else
        table.insert(roots, "/usr/local/share/dotnet")
        table.insert(roots, "/opt/homebrew/share/dotnet")
        table.insert(roots, path.join(os.getenv("HOME") or "", ".dotnet"))
    end

    for _, root in ipairs(roots) do
        local pack_dir = path.join(root, "packs", "Microsoft.NETCore.App.Host." .. rid)
        local versions = os.dirs(path.join(pack_dir, "*"))
        if #versions > 0 then
            table.sort(versions, version_less)
            return path.join(versions[#versions], "runtimes", rid, "native")
        end
    end
end

target("NativeManagedHost")
    set_luna_sdk_sample()
    add_files("Source/*.cpp")
    add_deps("Window")

    local native_dir = get_dotnet_host_native_dir()
    if not native_dir then
        raise("Cannot find .NET host native pack. Install the .NET SDK or set DOTNET_ROOT.")
    end
    add_includedirs(native_dir)
    add_linkdirs(native_dir)
    add_links("nethost")
    if is_plat("macosx", "linux") then
        add_rpathdirs(native_dir)
    end
target_end()
