-- Legacy/reference xmake script.
-- LunaBuild *.Target.cs rules are authoritative for main-path and CI builds.
-- Keep this file only as migration reference while old branches are being ported.

if is_plat("windows", "macosx", "linux") then
    includes("Studio")
    includes("LunaDoc")
end
includes("MultiPlatformSample")
