-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

if is_os("windows", "macosx") then
    luna_sdk_module_target("MakeSystem")
        add_headerfiles("*.hpp")
        add_files("Source/**.cpp")
        add_deps("Runtime", "VariantUtils", "JobSystem")
    target_end()
end