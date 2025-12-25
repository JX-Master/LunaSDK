## Prerequisites
### Common
* xmake building system, check [here](https://xmake.io/#/guide/installation) for installation instructions.
### Windows
* Visual Studio 2019 or later (Choose C++ desktop development & C++ game development workload).
### macOS
* XCode and Command Line Tool.
### Android
* Android Studio with Android SDK of your choice (Level 31 and newer is tested).
* Install NDK (29.0.14206865 or newer) in `Tools -> SDK Manager` in Android Studio.
### iOS/iPadOS
* Install iOS/iPadOS SDK in XCode.

## Building

1. Clone or download this project.
2. Run `setup.bat` (on Windows) or `setup.sh` (on macOS) to download third party SDKs that are not managed by xmake. You may need to invoke `chmod +x ./setup.sh` firstly on macOS if a "permission denied" error occurs.
3. Configure the building option by executing `xmake f {options}`. Possible options include:
	1. `-p` for target platform, supported values are `windows`,  `macosx` and `android`. This can be set automatically if you build project for the current platform.
	2.  `-a` for architecture, supported values are `x64` on Windows, `x86_64` and `arm64` on macOS, `arm64-v8a` and `x86_64` for Android.
	3.  `-m` for mode, supported values are:
		1. `debug`: keep debug info and disable code optimization.
		2. `profile`: discard debug info, enable full code optimization, keep profiling codes.
		3. `release`: discard debug info, enable full code optimization, remov profiling codes.
	4. `--ndk={NDK_PATH}` to specifiy NDK path if you want to build for Android platform.
	5. `--shared=y` for building all libraries as shared libraries.
	6. `--rhi_debug=y` if you want to enable the debug layer of the rendering backend (D3D12 debug layer or Vulkan validation layer).
	7. `--rhi_api=XXX` for choosing the rendering backend, supported values are: 
		1. Windows: `D3D12` (default),  `Vulkan`.
		2. macOS: `Metal` (default). 
	8. `--memory_profiler=y` for enabling memory profiler.
	9. `--thread_safe_assertion=y` for enabling thread safe assertions.
	10. `--build_tests=y` for building unit tests.
	11. `--api_validation=y` for enabling api validation macros (`lucheck` and variations).
	xmake will download and install dependency packages on first configure command.
4. Build the project by executing `xmake build`, or build one target by executing `xmake build {target}`.
5. Run one target by executing `xmake run {target}`.

### Packaging for Android devices

One Android Studio project is required for packaging `.apk` files for Android devices. When creating projects, make sure that:
1. `NativeActivity` is used as the base class of your application activity class.
2. `org.tboox.gradle-xmake-plugin` plugin is specified in `build.gradle` of your project and module, and root xmake script file is specified. See [xmake-gradle GitHub page](https://github.com/xmake-io/xmake-gradle) for details.

## Debugging in IDEs

### Visual Studio Code (Windows/macOS) (suggested)

If you use Visual Studio Code for developing, you can install `xmake` extension for a better developing experience. Based on your preference, you may choose to use Microsoft C/C++ extension or clangd extension for code highlighting and debugging.

#### Set up Microsoft C/C++ extension

If you use Microsoft C/C++ extension, you should create one `c_cpp_properties.json` file in your `.vscode` directory with the following include path:
```json
"${workspaceFolder}/Modules"
```
The whole file may looks like this:
```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/Modules"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "windowsSdkVersion": "10.0.26100.0",
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}
```
With xmake extension installed, you can now build and debug targets in xmake panel.

#### Set Up clangd extension

If you use clangd extension, you need to generate `compile_commands.json` file in `.vscode` to let clangd understand the project. With xmake extension installed, you can use Ctrl/Command+Shift+P to open command palette, then type `XMake: UpdateIntelliSence`, which generates `compile_commands.json` automatically for you. You should also add the following settings in your `.vscode/settings.json`:
```json
{
	"clangd.arguments": [
		"--compile-commands-dir=${workspaceFolder}/.vscode"
	]
}
```

#### Debugging with LLDB DAP

LLDB (LLVM debugger) only provides a command-line based debugging interface, which is inconvenient for those who are used to debugging in IDE. To have a visual debugging experience, we suggest the user to install LLDB DAP extension for vscode. Note that the `lldb-dap` program should be installed manually, follow the instructions on LLDB DAP extension page to install it.

After you setup LLDB DAP, you can create a `launch.json` file in `.vscode` to start debugging programs. Here is one example that debugs `Studio` program of LunaSDK:

```json
{  
	"type": "lldb-dap",  
	"request": "launch",  
	"name": "Debug Studio",  
	"program": "${workspaceRoot}/build/macosx/arm64/debug/Studio",  
	"args": [],  
	"env": [],
	"cwd": "${workspaceRoot}/build/macosx/arm64/debug"
}
```

### Visual Studio
1. Clone or download this project.
2. Double click `setup.bat` to perform project setup.
3. Double click `gen_vs2019.bat` or execute the following commands:
    ``` xmake project  -y -k vsxmake2019 -m "debug;profile;release" Solution ```
    if you user other Visual Studio versions, change `vsxmake2019` to your version, like `vsxmake2021`.
4. Open solution file in `/Solution/vsxmake2019/Luna.sln`
5. Build solution in Visual Studio.

### XCode
1. Clone or download this project.
2. Run `setup.sh` to perform project setup.
3. Run `gen_xcode.sh` on terminal:
    ```
    chmod +x ./gen_xcode.sh
    ./gen_xcode.sh
    ```
4. Open `/Solution/Luna.xcodeproj`.
5. Build products in XCode. 