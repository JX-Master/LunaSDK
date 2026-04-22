local native_dir = os.scriptdir()
local llvm_sdk = path.absolute(path.join(native_dir, "..", "..", "..", "SDKs", "llvm-21.1.1-release-macosx-arm64"))

target("cppsl-native-extractor")
    set_default(false)
    set_group("Tools/CPPSL")
    set_kind("binary")
    set_languages("cxx20")
    set_targetdir(path.join(native_dir, "bin"))
    add_files("src/main.cpp")
    add_includedirs(path.join(llvm_sdk, "include"))
    add_linkdirs(path.join(llvm_sdk, "lib"))
    add_links("clang-cpp")
    add_rpathdirs(path.join(llvm_sdk, "lib"))
    if is_plat("macosx") then
        add_frameworks("CoreServices", "CoreFoundation")
    end
