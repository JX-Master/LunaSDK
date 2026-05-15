local native_dir = os.scriptdir()
local repo_root = path.absolute(path.join(native_dir, "..", "..", ".."))
local llvm_sdk = nil
if os.host() == "macosx" and os.arch() == "arm64" then
    llvm_sdk = path.join(repo_root, "SDKs", "llvm-21.1.1", "macosx", "arm64")
elseif os.host() == "windows" and os.arch() == "x64" then
    llvm_sdk = path.join(repo_root, "SDKs", "llvm-21.1.1", "windows", "x64")
end

target("csharp-binding-generator")
    set_default(false)
    set_group("Tools/CSharpBindingGen")
    set_kind("binary")
    set_languages("cxx20")
    set_targetdir(path.join(native_dir, "bin"))
    add_files("src/main.cpp")
    if llvm_sdk == nil then
        assert(false, "C# binding generator requires an LLVM SDK for this host/architecture.")
    end
    add_includedirs(path.join(llvm_sdk, "include"))
    add_linkdirs(path.join(llvm_sdk, "lib"))
    if os.host() == "macosx" then
        add_cxxflags("-fno-rtti")
        add_links("clang-cpp")
        add_rpathdirs(path.join(llvm_sdk, "lib"))
        add_rpathdirs("@loader_path/../../../../llvm-21.1.1/macosx/arm64/lib")
        add_ldflags("-Wl,-headerpad_max_install_names")
        add_frameworks("CoreServices", "CoreFoundation")
    elseif os.host() == "windows" then
        set_runtimes("MD")
        add_defines("NDEBUG", "_ITERATOR_DEBUG_LEVEL=0")
        add_undefines("_DEBUG")
        for _, lib in ipairs(os.files(path.join(llvm_sdk, "lib", "clang*.lib"))) do
            add_links(path.basename(lib))
        end
        for _, lib in ipairs(os.files(path.join(llvm_sdk, "lib", "LLVM*.lib"))) do
            add_links(path.basename(lib))
        end
        add_syslinks("advapi32", "bcrypt", "dbghelp", "ntdll", "ole32", "shell32", "user32", "version", "ws2_32")
    end
target_end()
