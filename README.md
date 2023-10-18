**This is an experimental project and is in actively development. The APIs, implementations and docs may subject to rapid changes. Please don't use this project in production environments.**

# Luna SDK

![Luna SDK LOGO](https://www.lunasdk.org/logo.png)

Luna SDK is a C++ software development framework for real-time rendering applications like video games, interactive multimedia programs, data visualization programs and so on.

Key features:

* Everything is built from scratch, including os abstraction layer, container library, math library and much more.
* Modern graphics API targeting Direct3D 12, Vulkan and Metal, including window management.
* Full dynamic type reflection, including full support for generic types.
* Serialization support based on reflection.
* Built-in asset system and virtual file system for managing assets in runtime.
* Well documented and CI tested.

Designed target platforms:

* Windows (Direct3D 12+/Vulkan 1.0+)
* macOS (Metal 2+)
* Linux (Vulkan) (Not implemented yet.)
* Android (Vulkan) (Not implemented yet.)
* iOS (Metal) (Not implemented yet.)

## Building

[![windows-build](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-windows-main.yml/badge.svg?branch=main)](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-windows-main.yml)
[![macos-build](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-macos-main.yml/badge.svg?branch=main)](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-macos-main.yml)

### Prerequisites

* C++ toolchain on your platform:
    * Visual Studio 2019 or later on Windows (C++ desktop development & C++ game development workload).
    * XCode and command line tools on macOS.
    * clang or gcc on Linux (not implemented yet).
* xmake building system, check [here](https://xmake.io/#/guide/installation) for installation instructions.
* For Visual Studio Code users, install `XMake`(tboox) and `C/C++`(Microsoft) extensions on Visual Studio Code to improve development experience.

### Visual Studio
1. Clone or download this project.
1. Double click `setup.bat` to perform project setup.
1. Double click `gen_vs2019.bat` or execute the following commands:

    ``` xmake project  -y -k vsxmake2019 -m "debug;profile;release" Solution ```

    if you user other Visual Studio versions, change `vsxmake2019` to your version, like `vsxmake2021`.

1. Open solution file in `/Solution/vsxmake2019/Luna.sln`
1. Build solution in Visual Studio.

### Visual Studio Code
1. Clone or download this project.
1. Run `setup.bat` (on Windows) or `setup.sh` (on macOS) to perform project setup.
1. Open Code editor on the project root directory, then choose xmake toolchain in Code editor.
1. Configure the building option by executing `xmake f {options}`. Possible options include:
   1. `-p` for target platform, including `windows` and `macosx`. This can be set automatically for most of the time.
   1. `-a` for architecture, including `x64` and `arm64`. 
   1. `-m` for mode, including `debug`, `profile`and `release`.
   1. `--rhi_debug=y` if you want to enable the debug layer of the rendering backend (D3D12 debug layer or Vulkan validation layer).
   1. `--rhi_api=XXX` for choosing the rendering backend, including `D3D12` (default on Windows), `Vulkan` (default on Linux) and `Metal` (default on macOS). 
1. Open terminal and execute `xmake build` for all projects, or `xmake build {target}` for a specific target, like `Studio`. You may also use `Run and Debug` tab to build project if you install the `XMake` extension.

### XCode
1. Clone or download this project.
1. Run `setup.sh` to perform project setup.
1. Run `gen_xcode.sh` on terminal:
    ```
    chmod +x ./gen_xcode.sh
    ./gen_xcode.sh
    ```
1. Since the current version of xmake does not support running custom post-build scripts in XCode, you may need to copy images, shaders and other files to `build/macosx/{arch}/release/Debug` directory if the program failed to find them.
1. Open `Luna.xcodeproj` on the root directory of Luna SDK.
1. Build products in XCode. 

## System Requirements

he following requirements must be satisfied to run Luna SDK with Direct3D 12 rendering backend:

* Windows 10 operating system, 64-bit.

The following requirements must be satisfied to run Luna SDK with Vulkan rendering backend:

* Vulkan runtime must be present on the system, and must be supported by your GPU and driver.
  * Vulkan runtime is shipped as part of system components on most modern operations systems, including Windows, Linux and Android.
  * To check whether your GPU and driver supports Vulkan, consult [this database](https://vulkan.gpuinfo.org/).
  * Installation of [Vulkan SDK](https://vulkan.lunarg.com/) is **not required** to build Luna SDK with Vulkan rendering backend.

* `VK_KHR_maintenance1 ` extension support, which is mandatory in Vulkan 1.1+.
* `VK_KHR_swapchain` extension support, which should be supported on all platforms with display screens.

The following requirements must be satisfied to run Luna SDK with Metal rendering backend:

* macOS 10.15 (Catalina) and later.

## Docs
See [Luna SDK Docs](https://www.lunasdk.org).

## Feedback & Discussion
* `Issues` page is available for bug report and feature request (not guaranteed to be satisfied).
* `Discussions` page is available for discussions.
* QQ group: 665068249 (Chinese only).

## Contact Author
* Email: jxmaster@yeah.net
* Twitter: @JXMaster
* Zhihu: https://www.zhihu.com/people/jx-master
* Bilibili: https://space.bilibili.com/9919368

## Third Party SDKs and References
Luna SDK uses the following third party SDKs:
1. [Dear ImGui](https://github.com/ocornut/imgui) for GUI rendering (MIT License).
1. [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) for rendering gizmos using ImGui (MIT License).
1. [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler) for compiling HLSL shaders (University of Illinois Open Source License).
1. [GLFW](https://github.com/glfw/glfw) for window management on Windows, macOS and Linux (zlib License).
1. [STB](https://github.com/nothings/stb) for image file reading/writing and ttf font file processing (public domain).
1. [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for allocating device memory on Vulkan backend (MIT License).
1. [D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) for allocating device memory on D3D12 backend (MIT License).
1. [miniaudio](https://miniaud.io/index.html) for cross-platform low-level platform audio interface (public domain).

All SDKs are either embedded in the project, or can be fetched automatically by xmake scripts, the user does not need to install them manually.

The following SDKs are not directly used Luna SDK, but part of their design and implementation is referred when developing similar functionalities in Luna SDK:

1. [jsondiffpatch.net](https://github.com/wbish/jsondiffpatch.net) when developing the variant differential library (MIT License).
2. [DirectXMath](https://github.com/microsoft/DirectXMath) when developing the math library (MIT License).

## Alternatives

Here are some alternative real-time rendering engines/frameworks that have similar design goals to Luna SDK.

* [Sakura Engine](https://github.com/SakuraEngine/SakuraEngine) developed by SaeruHikari and other contributors (MIT License).
* [Horizon Engine](https://github.com/harukumo/HorizonEngine) developed by harukumo.
* [Piccolo Engine](https://github.com/BoomingTech/Piccolo) developed by Booming Tech and GAMES104 community contributors (MIT License).
* [CatDogEngine](https://github.com/CatDogEngine/CatDogEngine) developed by T-rvw, Hinageshi01 and other contributors (GPL-2.0 License).

## License
Luna SDK is licensed under the zlib/libpng license, see [LICENSE.txt](./LICENSE.txt) for more information.

## Mascot

![](https://www.lunasdk.org/luna-robot.png)

Say Hi to our mascot, a Luna SDK LOGO with two ellipses that look like eyes of one spherical robot.

