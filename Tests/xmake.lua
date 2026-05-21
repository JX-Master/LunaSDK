if is_plat("windows", "macosx", "linux") then
    includes("RuntimeTest")
    includes("VariantUtilsTest")
    includes("WindowTest")
    includes("RHITests")
    includes("VGTest")
    includes("FontArrangeTest")
    includes("ImGuiTest")
    includes("GUITest")
    includes("JobSystemTest")
    if is_os("windows", "macosx") then
        includes("MakeSystemTest")
    end
    includes("ECSTest")
    includes("AHITest")
end
