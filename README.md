**This is an experimental project and is in actively development. The APIs, implementations and docs may subject to rapid changes. Please don't use this project in production environments.**

# Luna SDK

![Luna SDK LOGO](https://www.lunasdk.org/logo.png)

Luna SDK is a C++ software development framework for real-time rendering applications like video games, interactive multimedia programs, data visualization programs and so on.

Key features:

* Modern graphics API targeting Direct3D 12, Vulkan and Metal, including window management.
* Full dynamic type reflection, including full support for generic types.
* Serialization support based on reflection.
* Fast entity-component-system scene representation.
* Built-in asset system and virtual file system for locating files.

Designed target platforms:

* Windows (Direct3D 12/Vulkan)
* macOS (Metal) (Not implemented yet.)
* Linux (Vulkan) (Not implemented yet.)
* Android (Vulkan) (Not implemented yet.)
* iOS (Metal) (Not implemented yet.)

## Building

### Prerequisites
* C++ toolchain on your platform:
    * Visual Studio 2019 or later on Windows (C++ desktop development & C++ game development workload).
    * XCode on macOS (not implemented yet).
    * clang or gcc on Linux (not implemented yet).
* xmake building system, check [here](https://xmake.io/#/guide/installation) for installation instructions.
* For Visual Studio Code users, install `XMake`(tboox) and `C/C++`(Microsoft) extensions on Visual Studio Code to improve development experience.
* [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) should be installed manually if you want to build Luna SDK with Vulkan backend. This is a requirement for Linux users. 

### Visual Studio
1. Clone or download this project.
1. Double click `gen_vs2019.bat` or execute the following commands:

    ``` xmake project  -y -k vsxmake2019 -m "debug;profile;release" Solution ```

    if you user other Visual Studio versions, change `vsxmake2019` to your version, like `vsxmake2021`.

1. Open solution file in `/Solution/vsxmake2019/Luna.sln`
1. Build solution in Visual Studio.

### Visual Studio Code
1. Clone or download this project.
1. Open Code editor on the project root directory, then choose xmake toolchain in Code editor.
1. Configure the building option by executing `xmake f {options}`. Possible options include:
   1. `-p` for target platform, including `windows` and `macosx`. This can be set automatically for most of the time.
   1. `-a` for architecture, including `x64` and `arm64`. 
   1. `-m` for mode, including `debug`, `profile`and `release`.
   1. `--rhi_debug=y` if you want to enable the debug layer of the rendering backend (D3D12 debug layer or Vulkan validation layer).
   1. `--rhi_api=XXX` for choosing the rendering backend, including `D3D12` (default on Windows), `Vulkan` (default on Linux) and `Metal` (default on macOS). 
1. Open terminal and execute `xmake build` for all projects, or `xmake build {target}` for a specific target, like `Studio`. You may also use `Run and Debug` tab to build project if you install the `XMake` extension.

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
1. [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler) for compiling HLSL shaders (University of Illinois Open Source License).
1. [GLFW](https://github.com/glfw/glfw) for window management on Windows, macOS and Linux (zlib License).
1. [STB](https://github.com/nothings/stb) for image file reading/writing and ttf font file processing (public domain).
1. [zlib](https://github.com/madler/zlib) for compression/decompression (zlib License).
1. [minizip-ng](https://github.com/zlib-ng/minizip-ng) for zip archive file reading/writing (zlib License).

All SDKs are either embedded in the project, or can be fetched automatically by xmake scripts, the user does not need to install them manually.

The following SDKs are not directly used Luna SDK, but part of their design and implementation is referred when developing similar functionalities in Luna SDK:

1. [jsondiffpatch.net](https://github.com/wbish/jsondiffpatch.net) when developing the variant differential library (MIT License).
2. [DirectXMath](https://github.com/microsoft/DirectXMath) when developing the math library (MIT License).

## License
Luna SDK is licensed under the zlib/libpng license, see [LICENSE.txt](./LICENSE.txt) for more information.