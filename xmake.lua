set_project("Luna")

add_moduledirs("Tools/xmake/modules")
add_repositories("luna-repo SDKs")

add_rules("mode.debug", "mode.profile", "mode.release")

option("build_shared_lib")
    set_default(true)
    set_showmenu(true)
    set_description("Build All SDK Modules as Shared Library.")
    add_defines("LUNA_BUILD_SHARED_LIB")
option_end()

option("contract_assertion")
    set_default(false)
    set_showmenu(true)
    set_description("Enables contract assertions. This is always enabled in debug build.")
    add_defines("LUNA_ENABLE_CONTRACT_ASSERTION")
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
    add_options("build_shared_lib", "contract_assertion", "thread_safe_assertion")
end

function set_luna_sdk_module()
    add_luna_sdk_options()
    set_group("Modules")
    if has_config("build_shared_lib") then
        set_kind("shared")
    else
        set_kind("static")
    end
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

add_requires("imgui", {configs = {shared = has_config("build_shared_lib")}})
add_requires("zlib-ng", {configs = {shared = has_config("build_shared_lib")}})
add_requires("minizip-ng", {configs = {shared = has_config("build_shared_lib")}})

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