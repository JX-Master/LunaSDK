**This is an experimental project and is in actively development. The APIs, implementations and docs may subject to rapid changes. Please don't use this project in production environments.**

# LunaSDK

![LunaSDK LOGO](https://www.lunasdk.net/logo.png)

[![windows-build](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-windows-main.yml/badge.svg?branch=main)](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-windows-main.yml)
[![macos-build](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-macos-main.yml/badge.svg?branch=main)](https://github.com/JX-Master/LunaSDK/actions/workflows/ci-macos-main.yml)

LunaSDK is a C++ software development framework for real-time rendering applications like video games, interactive multimedia programs, data visualization programs and so on.

Specifications:

* Self-implemented fundamental libraries, including platform abstraction layer, container library, math library and more. No dependency on C++ STL.
* Full dynamic type reflection, including full support for generic types.
* Serialization and deserialization based on reflection, including JSON and XML support.
* Job system and thread pool for asynchronous computing.
* Render Hardware Interface (RHI) targeting Direct3D 12, Vulkan and Metal.
* Window management API.
* Shader compiling APIs that compile HLSL shaders to DXIL, SPIR-V and Metal.
* Low-latency Audio Hardware Interface (AHI).
* Human Interface Device API providing APIs to access input / output devices.
* Asset system and virtual file system for managing assets at runtime.
* File loader for image files, font files and .OBJ files.
* GPU-driven vector graphics rendering.
* Well documented and CI tested.

Designed target platforms:

* Windows (Direct3D 12+/Vulkan 1.0+)
* macOS (Metal 2+)
* Linux (Vulkan) (Not implemented yet.)
* Android (Vulkan) (Not implemented yet.)
* iOS (Metal) (Not implemented yet.)

## System Requirements

### Windows

* Windows 10 version 1703 (OS build 15063) or later.

The following requirements must be satisfied to run LunaSDK with Vulkan rendering backend:

* Vulkan runtime must be present on the system, and must be supported by your GPU and driver.
  * Vulkan runtime is shipped as part of system components on most modern operations systems, including Windows, Linux and Android.
  * To check whether your GPU and driver supports Vulkan, consult [this database](https://vulkan.gpuinfo.org/).
  * Installation of [Vulkan SDK](https://vulkan.lunarg.com/) is **not required** to build LunaSDK with Vulkan rendering backend.

* `VK_KHR_maintenance1 ` extension support, which is mandatory in Vulkan 1.1+.
* `VK_KHR_swapchain` extension support, which should be supported on all platforms with display screens.

### macOS

* macOS 10.15 (Catalina) and later.

## Docs
Docs are placed at `./LunaSDK-Docs` directory. Use [Obsidian](https://obsidian.md/) to open and read the docs.

## Feedback & Discussion
* `Issues` page is available for bug report and feature request (not guaranteed to be satisfied).
* `Discussions` page is available for discussions.
* QQ group: 665068249 (Chinese only).

## Contact Author
* Email: jxmaster@yeah.net
* Discord: jxmaster.me
* Zhihu: https://www.zhihu.com/people/jx-master
* Bilibili: https://space.bilibili.com/9919368

## Third Party SDKs and References
LunaSDK uses the following third party SDKs:
1. [Dear ImGui](https://github.com/ocornut/imgui) for GUI rendering (MIT License).
1. [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) for rendering gizmos using ImGui (MIT License).
1. [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler) for compiling HLSL shaders (University of Illinois Open Source License).
1. [GLFW](https://github.com/glfw/glfw) for window management on Windows, macOS and Linux (zlib License).
1. [STB](https://github.com/nothings/stb) for image file reading/writing and ttf font file processing (public domain).
1. [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for allocating device memory on Vulkan backend (MIT License).
1. [D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) for allocating device memory on D3D12 backend (MIT License).
1. [miniaudio](https://miniaud.io/index.html) for cross-platform low-level platform audio interface (public domain).

All SDKs are either embedded in the project, or can be fetched automatically by xmake scripts, the user does not need to install them manually.

The following SDKs are not directly used LunaSDK, but part of their design and implementation is referred when developing similar functionalities in LunaSDK:

1. [jsondiffpatch.net](https://github.com/wbish/jsondiffpatch.net) when developing the variant differential library (MIT License).
2. [DirectXMath](https://github.com/microsoft/DirectXMath) when developing the math library (MIT License).

## Alternatives

Here are some alternative real-time rendering engines/frameworks that have similar design goals to LunaSDK.

* [Sakura Engine](https://github.com/SakuraEngine/SakuraEngine) developed by SaeruHikari and other contributors (MIT License).
* [Horizon Engine](https://github.com/harukumo/HorizonEngine) developed by harukumo.
* [Piccolo Engine](https://github.com/BoomingTech/Piccolo) developed by Booming Tech and GAMES104 community contributors (MIT License).
* [CatDogEngine](https://github.com/CatDogEngine/CatDogEngine) developed by T-rvw, Hinageshi01 and other contributors (GPL-2.0 License).

## License
LunaSDK is licensed under the zlib/libpng license, see [LICENSE.txt](./LICENSE.txt) for more information.

## Mascot

![](https://www.lunasdk.net/luna-robot.png)

Say Hi to our mascot, a LunaSDK LOGO with two ellipses that look like eyes of one spherical robot.

