-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

if is_plat("windows", "macosx", "linux") then
    includes("RuntimeTest")
    includes("VariantUtilsTest")
    includes("WindowTest")
    includes("RHITests")
    includes("VGTest")
    includes("FontArrangeTest")
    includes("ImGuiTest")
    includes("JobSystemTest")
    if is_os("windows", "macosx") then
        includes("MakeSystemTest")
    end
    includes("ECSTest")
    includes("AHITest")
end
