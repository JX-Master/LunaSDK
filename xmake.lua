set_project("Luna")

add_moduledirs("Tools/xmake/modules")

rule("luna.shader")
    set_extensions(".hlsl")
    on_load(function (target) 
        local headerdir = path.join(target:autogendir(), "shaders")
        if not os.isdir(headerdir) then 
            os.mkdir(headerdir)
        end 
        target:add("includedirs", headerdir)
        
        local cpp_rule = target:rule("c++.build"):clone()
        cpp_rule:add("deps", "luna.shader", {order = true})
        target:rule_add(cpp_rule)
    end)
    on_build_file(function (target, sourcefile, opt)
        import("compile_shader")
        import("utils.progress")
        import("core.project.depend")

        -- get object file
        local headerdir = path.absolute(path.join(target:autogendir(), "shaders"))
        local targetfile = path.join(headerdir, path.basename(sourcefile) .. ".hpp")
        local configs = target:fileconfig(sourcefile) or {}

        -- need build this object?
        local dependfile = target:dependfile(targetfile)
        local dependinfo = target:is_rebuilt() and {} or (depend.load(dependfile) or {})
        if not depend.is_changed(dependinfo, {lastmtime = os.mtime(targetfile), values = configs}) then
            return
        end

        -- trace progress info
        progress.show(opt.progress, "${color.build.object}compiling.shader %s", sourcefile)

        -- build this object.
        configs.output = targetfile
        configs.cpp_output = true
        compile_shader.compile_shader(sourcefile, configs)

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

option("contract_assertion")
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
    set_description("Whether to build tests for Luna SDK")
option_end()

option("memory_profiler")
    set_default(true)
    set_showmenu(true)
    set_description("Whether to forcly enable memory profiler for Luna SDK. The memory profiler will still be enabled in Debug and Profile mode.")
    add_defines("LUNA_ENABLE_MEMORY_PROFILER")
option_end()

function get_default_rhi_api()
    local default_rhi_api = false
    if is_os("windows") then
        default_rhi_api = "D3D12"
    elseif is_os("macosx", "ios") then
        default_rhi_api = "Metal"
    elseif is_os("linux", "android") then
        default_rhi_api = "Vulkan"
    end
    if default_rhi_api == false then
        raise("No Graphics API is present for the current platform!")
    end
    return default_rhi_api
end

option("rhi_api")
    set_default(get_default_rhi_api())
    set_showmenu(true)
    if is_os("windows") then
        set_values("D3D12", "Vulkan")
    elseif is_os("macosx", "ios") then
        set_values("Metal")
    elseif is_os("linux", "android") then
        set_values("Vulkan")
    end
    set_description("The Graphics API to use for Luna SDK")
option_end()

if is_config("rhi_api", "Vulkan") then 
    add_requires("volk", {configs = {header_only = true}})
    add_requires("vulkan-memory-allocator")
elseif is_config("rhi_api", "D3D12") then 
    add_requireconfs("d3d12-memory-allocator", {toolchains = "msvc"}) -- currently d3d12-memory-allocator does not support clang-cl.
    add_requires("d3d12-memory-allocator")
end

function add_luna_sdk_options()
    add_options("shared", "contract_assertion", "thread_safe_assertion", "memory_profiler", "rhi_api")
    -- Contract assertion is always enabled in debug mode.
    if has_config("contract_assertion") or is_mode("debug") then
        add_defines("LUNA_ENABLE_CONTRACT_ASSERTION")
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
    set_kind("binary")
end

add_includedirs("Modules")
set_languages("c99", "cxx17")

if is_os("windows") then 
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
end

includes("Modules")
includes("Programs")

if has_config("build_tests") then
    includes("Tests")
end
