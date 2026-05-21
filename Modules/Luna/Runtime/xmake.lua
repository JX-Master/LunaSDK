-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

luna_sdk_module_target("Runtime")
    add_headerfiles("*.hpp", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Impl/**.hpp)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Impl/**.inl)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Math/**.hpp)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("(Math/**.inl)", {prefixdir = "Luna/Runtime"})
    add_headerfiles("Source/**.hpp", {install = false})
    add_files("Source/*.cpp")
    if is_plat("windows") then
        add_headerfiles("(Platform/Windows/**.hpp)", {prefixdir = "Luna/Runtime"})
        add_headerfiles("Source/Platform/Windows/*.hpp", {install = false})
        add_files("Source/Platform/Windows/*.cpp")
    elseif is_plat("linux", "macosx", "android", "iphoneos") then
        add_headerfiles("Source/Platform/POSIX/*.hpp", {install = false})
        add_headerfiles("Source/Platform/POSIX/FiberContextBase.h", {install = false})
        add_files("Source/Platform/POSIX/*.cpp")
        if is_arch("arm64", "aarch64", "arm64-v8a") then
            add_headerfiles("Source/Platform/POSIX/FiberContext_arm64.h", {install = false})
            add_files("Source/Platform/POSIX/FiberContext_arm64.S")
        elseif is_arch("x86_64", "x64") then
            add_headerfiles("Source/Platform/POSIX/FiberContext_x86_64.h", {install = false})
            add_files("Source/Platform/POSIX/FiberContext_x86_64.S")
        elseif is_arch("i386", "x86") then
            add_headerfiles("Source/Platform/POSIX/FiberContext_x86.h", {install = false})
            add_files("Source/Platform/POSIX/FiberContext_x86.S")
        elseif is_arch("arm", "armv7", "armv7k", "armeabi-v7a") then
            add_headerfiles("Source/Platform/POSIX/FiberContext_arm.h", {install = false})
            add_files("Source/Platform/POSIX/FiberContext_arm.S")
        end
        if is_plat("linux", "macosx", "iphoneos") then
            add_syslinks("pthread")
        end
    end
target_end()