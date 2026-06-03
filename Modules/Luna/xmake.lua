-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

includes("Runtime")
includes("Frontend")
includes("VariantUtils")
includes("HID")
includes("Window")
includes("VFS")
includes("RHI")
includes("RHIUtility")
includes("Font")
includes("VG")
includes("GUI")
includes("GUIWindow")
includes("ImGui")
includes("JobSystem")
includes("Asset")
includes("Image")
includes("ObjLoader")
includes("RG")
includes("AHI")
includes("ECS")
includes("MakeSystem")
includes("Lua")

includes("Experimental") -- For modules that is not ready for production use.
