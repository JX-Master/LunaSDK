-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

target("DemoApp")
    set_luna_sdk_program()
    add_rules("luna.shader")
    add_files("**.cpp")
    add_luna_shader("DemoAppVS.cxx", {type = "vertex", entry_point = "vs_main"})
    add_luna_shader("DemoAppPS.cxx", {type = "pixel", entry_point = "ps_main"})
    add_deps("Runtime", "Window", "RHI", "RHIUtility", "Image")
    before_build(function(target)
        os.cp("$(scriptdir)/luna.png", target:targetdir() .. "/luna.png")
    end)
    after_install(function (target)
        os.cp(target:targetdir() .. "/luna.png", target:installdir() .. "/bin/luna.png")
    end)
target_end()
