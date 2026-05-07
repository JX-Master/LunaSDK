> ⚠️ Warning: This is an experimental project and is in actively development. The APIs, implementations and docs may subject to rapid change. Please don't use this project in production environments.

# LunaSDK

![LunaSDK LOGO](./LunaSDK-Docs/Res/logo.png)

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
* CPPSL shader compilation pipeline targeting DXIL, SPIR-V and Metal.
* Low-latency Audio Hardware Interface (AHI).
* Human Interface Device API providing APIs to access input / output devices.
* Asset system and virtual file system for managing assets at runtime.
* File loader for image files, font files and .OBJ files.
* GPU-driven vector graphics rendering.
* Well documented and CI tested.

Designed target platforms:

* Windows (Direct3D 12+/Vulkan 1.0+)
* macOS (Metal 3.2+)
* Linux (Vulkan) (Not implemented yet.)
* Android (Vulkan 1.0+)
* iOS (Metal 3.2+)

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

* macOS 15 (Sequoia) and later.

### Android

* Android 11 (SDK Level 31) and later (older versions MAY run LunaSDK, but is not tested).

### iOS

* iOS 18 and later.

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
1. [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler) for compiling CPPSL-generated HLSL to DXIL (University of Illinois Open Source License).
1. [STB](https://github.com/nothings/stb) for image file reading/writing and ttf font file processing (public domain).
1. [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for allocating device memory on Vulkan backend (MIT License).
1. [Volk](https://github.com/zeux/volk.git) for dynamically loading Vulkan interfaces (MIT License).
1. [D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) for allocating device memory on D3D12 backend (MIT License).
1. [miniaudio](https://miniaud.io/index.html) for cross-platform low-level platform audio interface (public domain).
1. [Khronos glslang](https://github.com/KhronosGroup/glslang) for compiling CPPSL-generated GLSL to SPIR-V (Apache-2.0 License).
1. [Lua](https://www.lua.org/about.html) for Lua scripting environment (MIT License).
1. [.NET](https://dotnet.microsoft.com) for implementing CPPSL (Cpp Shader Language) compiler (MIT License).
1. [LLVM](https://llvm.org) for implementing CPPSL (Cpp Shader Language) compiler (Apache-2.0 License).

All SDKs are either embedded in the project, or can be fetched automatically by xmake scripts, the user does not need to install them manually.

The following SDKs are not directly used LunaSDK, but part of their design and implementation is referred when developing similar functionalities in LunaSDK:

1. [jsondiffpatch.net](https://github.com/wbish/jsondiffpatch.net) when developing the variant differential library (MIT License).
1. [DirectXMath](https://github.com/microsoft/DirectXMath) when developing the math library (MIT License).
1. [Marl](https://github.com/google/marl) when implementing fibers on POSIX system (Apache-2.0 License).

## Alternatives

Here are some alternative real-time rendering engines/frameworks that have similar design goals to LunaSDK.

* [Sakura Engine](https://github.com/SakuraEngine/SakuraEngine) developed by SaeruHikari and other contributors (MIT License).
* [Horizon Engine](https://github.com/harukumo/HorizonEngine) developed by harukumo.
* [Piccolo Engine](https://github.com/BoomingTech/Piccolo) developed by Booming Tech and GAMES104 community contributors (MIT License).
* [CatDogEngine](https://github.com/CatDogEngine/CatDogEngine) developed by T-rvw, Hinageshi01 and other contributors (GPL-2.0 License).

## License
LunaSDK is licensed under the zlib/libpng license, see [LICENSE.txt](./LICENSE.txt) for more information.

## Mascot

![](./LunaSDK-Docs/Res/luna-robot.png)

Say Hi to our mascot, a LunaSDK LOGO with two ellipses that look like eyes of one spherical robot.
