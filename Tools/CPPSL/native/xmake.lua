local native_dir = os.scriptdir()
local repo_root = path.absolute(path.join(native_dir, "..", "..", ".."))
local llvm_sdk = nil
if os.host() == "macosx" and os.arch() == "arm64" then
    llvm_sdk = path.join(repo_root, "SDKs", "llvm-21.1.1", "macosx", "arm64")
elseif os.host() == "windows" and os.arch() == "x64" then
    llvm_sdk = path.join(repo_root, "SDKs", "llvm-21.1.1", "windows", "x64")
end

target("cppsl-native-extractor")
    set_default(false)
    set_group("Tools/CPPSL")
    set_kind("binary")
    set_languages("cxx20")
    set_targetdir(path.join(native_dir, "bin"))
    add_files("src/main.cpp")
    if llvm_sdk == nil then
        assert(false, "CPPSL native extractor requires an LLVM SDK for this host/architecture.")
    end
    add_includedirs(path.join(llvm_sdk, "include"))
    add_linkdirs(path.join(llvm_sdk, "lib"))
    if os.host() == "macosx" then
        add_cxxflags("-fno-rtti")
        add_links("clang-cpp")
        add_rpathdirs(path.join(llvm_sdk, "lib"))
        add_frameworks("CoreServices", "CoreFoundation")
    elseif os.host() == "windows" then
        for _, lib in ipairs(os.files(path.join(llvm_sdk, "lib", "clang*.lib"))) do
            add_links(path.basename(lib))
        end
        for _, lib in ipairs(os.files(path.join(llvm_sdk, "lib", "LLVM*.lib"))) do
            add_links(path.basename(lib))
        end
        add_syslinks("advapi32", "bcrypt", "dbghelp", "ole32", "shell32", "user32", "version", "ws2_32")
    end
