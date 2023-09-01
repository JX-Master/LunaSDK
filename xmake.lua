set_project("Luna")

add_moduledirs("Tools/xmake/modules")
add_repositories("luna-repo SDKs")

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

function add_luna_sdk_options()
    add_options("shared", "contract_assertion", "thread_safe_assertion")
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

function add_luna_modules(...)
    local args = {...}
    for i, mod in ipairs(args) do
        add_deps(mod)
        if mod ~= "Runtime" then
            if is_plat("windows") then 
                add_ldflags("/INCLUDE:luna_static_register_module_" .. mod, {force = true, public = false})
            else
                add_ldflags("-u _luna_static_register_module_" .. mod, {force = true, public = false})
            end
        end
    end
end

add_requires("imgui", {configs = {shared = has_config("shared")}})

add_includedirs("Modules")
set_languages("c99", "cxx17")

if is_os("windows") then 
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
end

includes("Modules")
includes("Programs")

if has_config("build_tests") then
    includes("Tests")
end