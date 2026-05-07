if is_plat("windows", "macosx", "linux") then
    includes("RuntimeTest")
    includes("VariantUtilsTest")
    includes("WindowTest")
    includes("RHITests")
    includes("VGTest")
    includes("FontArrangeTest")
    includes("ImGuiTest")
    includes("JobSystemTest")
    includes("ECSTest")
    includes("AHITest")
    if has_config("managed") then
        includes("RuntimeCSharpTest")
        includes("ImageCSharpTest")
        includes("VFSCSharpTest")
        includes("FontCSharpTest")
        includes("AssetCSharpTest")
        includes("WindowCSharpTest")
        includes("RHICSharpTest")
    end
end
