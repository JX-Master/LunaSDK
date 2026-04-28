set_project("Luna")

add_moduledirs("Tools/xmake/modules")

rule("luna.shader")
    set_extensions(".hlsl", ".cxx")
    add_orders("luna.shader", "c++.build")
    on_load(function (target) 
        local headerdir = path.join(target:autogendir(), "shaders")
        if not os.isdir(headerdir) then 
            os.mkdir(headerdir)
        end 
        target:add("includedirs", headerdir)
        target:add("deps", "cppsl-native-extractor", {order = true})
        target:add("deps", "CPPSL", {order = true})
        
        local cpp_rule = target:rule("c++.build"):clone()
        cpp_rule:add("deps", "luna.shader", {order = true})
        target:rule_add(cpp_rule)
    end)
    before_build(function (target)
        local function cppslc_path()
            return path.join(os.projectdir(), "Tools", "CPPSL", "src", "CPPSL.Cli", "bin", "Debug", "net9.0", is_host("windows") and "cppslc.exe" or "cppslc")
        end

        local function native_extractor_path()
            return path.join(os.projectdir(), "Tools", "CPPSL", "native", "bin", is_host("windows") and "cppsl-native-extractor.exe" or "cppsl-native-extractor")
        end

        local function build_cppsl_cli()
            local cli_project = path.join(os.projectdir(), "Tools", "CPPSL", "src", "CPPSL.Cli", "CPPSL.Cli.csproj")
            os.runv("dotnet", {
                "build",
                cli_project,
                "-m:1",
                "/nr:false",
                "--nologo",
                "-p:UseSharedCompilation=false"
            })
        end

        local function acquire_tools_lock()
            local lock_dir = path.join(os.projectdir(), "build", ".cppsl-tools-build.lock")
            for _ = 1, 6000 do
                if os.isfile(cppslc_path()) and os.isfile(native_extractor_path()) then
                    return nil
                end

                local ok = try {
                    function ()
                        if is_host("windows") then
                            os.runv("cmd", {"/c", "mkdir", lock_dir})
                        else
                            os.runv("mkdir", {lock_dir})
                        end
                        return true
                    end
                }
                if ok then
                    return function ()
                        os.rm(lock_dir)
                    end
                end
                os.sleep(100)
            end
            os.raise("timed out while waiting for CPPSL tool build lock: " .. lock_dir)
        end

        local function ensure_tools()
            if os.isfile(cppslc_path()) and os.isfile(native_extractor_path()) then
                return
            end

            local release_lock = acquire_tools_lock()
            if release_lock == nil then
                return
            end
            local ok, errors = try {
                function ()
                    if not os.isfile(native_extractor_path()) then
                        os.runv("xmake", {"-b", "cppsl-native-extractor"})
                    end
                    if not os.isfile(cppslc_path()) then
                        build_cppsl_cli()
                    end
                    return true
                end
            }
            release_lock()
            if not ok then
                os.raise(errors)
            end

            if not os.isfile(cppslc_path()) then
                os.raise("CPPSL CLI build did not produce: " .. cppslc_path())
            end
            if not os.isfile(native_extractor_path()) then
                os.raise("CPPSL native extractor build did not produce: " .. native_extractor_path())
            end
        end

        for _, sourcefile in ipairs(target:sourcefiles()) do
            if path.extension(sourcefile) == ".cxx" then
                ensure_tools()
                break
            end
        end
    end)
    on_build_file(function (target, sourcefile, opt)
        import("utils.progress")
        import("core.project.depend")

        -- get object file
        local headerdir = path.absolute(path.join(target:autogendir(), "shaders"))
        local targetfile = path.join(headerdir, path.basename(sourcefile) .. ".hpp")
        local configs = target:fileconfig(sourcefile) or {}

        -- need build this object?
        local dependfile = target:dependfile(targetfile)
        local dependinfo = target:is_rebuilt() and {} or (depend.load(dependfile) or {})
        if not depend.is_changed(dependinfo, {lastmtime = os.mtime(targetfile), values = configs, files = {sourcefile}}) then
            return
        end

        -- trace progress info
        progress.show(opt.progress, "${color.build.object}compiling.shader %s", sourcefile)

        -- build this object.
        configs.output = targetfile
        configs.cpp_output = true
        if path.extension(sourcefile) == ".cxx" then
            import("compile_cppsl")
            compile_cppsl.compile_cppsl(sourcefile, configs)
        else
            import("compile_shader")
            compile_shader.compile_shader(sourcefile, configs)
        end

        -- update files and values to the dependent file
        dependinfo.files = {sourcefile}
        dependinfo.values = configs
        depend.save(dependinfo, dependfile)
    end)
rule_end()

function add_luna_shader(file, config)
    if is_config("rhi_api", "D3D12") then
        config.target_format = "dxil"
    elseif is_config("rhi_api", "Vulkan") then
        config.target_format = "spir_v"
    elseif is_config("rhi_api", "Metal") then
        config.target_format = "msl"
    end
    add_files(file, config)
end

add_rules("mode.debug", "mode.profile", "mode.release")
add_defines("LUNA_MANUAL_CONFIG_DEBUG_LEVEL")
if is_mode("debug") then
    add_defines("LUNA_DEBUG_LEVEL=2")
elseif is_mode("profile") then
    add_defines("LUNA_DEBUG_LEVEL=1")
else 
    add_defines("LUNA_DEBUG_LEVEL=0")
end

option("shared")
    set_default(true)
    set_showmenu(true)
    set_description("Build All SDK Modules as Shared Library.")
    add_defines("LUNA_BUILD_SHARED_LIB")
option_end()

option("api_validation")
    set_default(false)
    set_showmenu(true)
    set_description("Enables contract assertions. This is always enabled in debug build.")
option_end()

option("thread_safe_assertion")
    set_default(false)
    set_showmenu(true)
    set_description("Enables thread safe assertions. This is always enabled in debug build.")
    add_defines("LUNA_ENABLE_THREAD_SAFE_ASSERTION")
option_end()

option("build_tests")
    set_default(true)
    set_showmenu(true)
    set_description("Whether to build tests for LunaSDK")
option_end()

option("memory_profiler")
    set_default(false)
    set_showmenu(true)
    set_description("Whether to forcly enable memory profiler for LunaSDK.")
    add_defines("LUNA_ENABLE_MEMORY_PROFILER")
option_end()

function get_default_rhi_api()
    local default_rhi_api = nil
    if is_plat("windows") then
        default_rhi_api = "D3D12"
    elseif is_plat("macosx", "iphoneos") then
        default_rhi_api = "Metal"
    elseif is_plat("linux", "android") then
        default_rhi_api = "Vulkan"
    end
    return default_rhi_api
end

function set_default_rhi_api()
    local default_rhi = get_default_rhi_api();
    if default_rhi then
        set_default(default_rhi)
    end
end

option("rhi_api")
    set_default_rhi_api()
    set_showmenu(true)
    if is_os("windows") then
        set_values("D3D12", "Vulkan")
    elseif is_os("macosx", "ios") then
        set_values("Metal")
    elseif is_os("linux", "android") then
        set_values("Vulkan")
    end
    set_description("The Graphics API to use for LunaSDK")
option_end()

if is_config("rhi_api", "Vulkan") then 
    add_requires("volk", {configs = {header_only = true}})
    add_requires("vulkan-memory-allocator")
elseif is_config("rhi_api", "D3D12") then 
    add_requires("d3d12-memory-allocator", {configs = {toolchains = "msvc"}}) -- currently d3d12-memory-allocator does not support clang-cl.
end

function add_luna_sdk_options()
    add_options("shared", "api_validation", "thread_safe_assertion", "memory_profiler", "rhi_api")
    -- API validation is always enabled in debug mode.
    if has_config("api_validation") or is_mode("debug") then
        add_defines("LUNA_ENABLE_API_VALIDATION")
    end
end

function luna_sdk_module_target(target_name)
    target(target_name)
    add_luna_sdk_options()
    set_group("Modules")
    if has_config("shared") then
        set_kind("shared")
    else
        set_kind("static")
    end
    set_basename("Luna" .. target_name)
    set_exceptions("none")
end

function set_luna_sdk_test()
    add_luna_sdk_options()
    set_group("Tests")
end

function set_luna_sdk_program()
    add_luna_sdk_options()
    set_group("Programs")
    if is_plat("android") then
        -- Android uses Java Activity as main entry point, the user program is provided as 
        -- shared library for JNI invoking.
        set_kind("shared")
    else
        set_kind("binary")
    end
end

add_includedirs("Modules")
set_languages("c17", "cxx20")

if is_os("windows") then 
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
end

includes("Tools/CPPSL/native")

target("CPPSL")
    set_default(false)
    set_group("Tools/CPPSL")
    set_kind("phony")
    add_deps("cppsl-native-extractor")
    on_build(function ()
        local cppslc = path.join(os.projectdir(), "Tools", "CPPSL", "src", "CPPSL.Cli", "bin", "Debug", "net9.0", is_host("windows") and "cppslc.exe" or "cppslc")
        local native_extractor = path.join(os.projectdir(), "Tools", "CPPSL", "native", "bin", is_host("windows") and "cppsl-native-extractor.exe" or "cppsl-native-extractor")
        local cli_project = path.join(os.projectdir(), "Tools", "CPPSL", "src", "CPPSL.Cli", "CPPSL.Cli.csproj")
        os.runv("dotnet", {
            "build",
            cli_project,
            "-m:1",
            "/nr:false",
            "--nologo",
            "-p:UseSharedCompilation=false"
        })
        if not os.isfile(cppslc) then
            os.raise("CPPSL CLI build did not produce: " .. cppslc)
        end
        if not os.isfile(native_extractor) then
            os.raise("CPPSL native extractor build did not produce: " .. native_extractor)
        end
    end)

includes("Modules")
includes("Programs")

if has_config("build_tests") then
    includes("Tests")
end
