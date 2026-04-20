if is_plat("windows", "macosx", "linux") and has_config("managed") then
    includes("HelloWindow")
    includes("HelloRHI")
    includes("ManagedHostApp")
    includes("NativeManagedHost")
end
