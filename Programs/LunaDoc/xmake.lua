target("LunaDoc")
    set_luna_sdk_program()
    add_files("**.cpp")
    add_deps("Runtime", "VariantUtils")
target_end()