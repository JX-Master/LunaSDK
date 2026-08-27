## Prerequisites

### Common

* .NET 9 SDK, check [here](https://dotnet.microsoft.com/download) for installation instructions.
* LunaBuild is the authoritative build system for LunaSDK.

### Windows

* Visual Studio 2022 or later.
* Install the C++ desktop development workload.
* Windows 10 SDK or later.

### macOS

* Apple Silicon Mac. LunaSDK supports macOS arm64 only.
* Xcode and Command Line Tools.
* Metal toolchain:

```sh
xcodebuild -downloadComponent MetalToolchain
```

### Android

* Android Studio or Android SDK command-line tools.
* Android SDK platform matching the application `compileSdk`.
* Android NDK matching the application `ndkVersion`.
* JDK 17 or the JBR bundled with Android Studio.

LunaBuild builds the native targets. The Android Gradle project is used only for APK packaging and does not compile native code.

### iOS/iPadOS

Install the iOS/iPadOS platform in Xcode. LunaBuild can compile iOS targets, assemble `.app` bundles, optionally sign them, and create an `.ipa` when the package output ends in `.ipa`. Device signing requires a suitable identity and, when applicable, a provisioning profile. Simulator builds use `--apple-sdk iphonesimulator`.

## Setup

Clone or download this project, then download LunaSDK's prebuilt third-party SDK bundle.

On Windows:

```bat
setup.bat
```

On macOS:

```sh
chmod +x ./setup.sh
./setup.sh
```

The setup script downloads the platform SDK archive into `SDKs`. LunaBuild does not manage packages and does not download third-party libraries during build. All external SDK paths are declared by LunaSDK target rules.

## Building

LunaBuild is invoked through the .NET project:

```sh
dotnet run --project LunaBuild.csproj -- <command> [options]
```

Build the default engine targets for the host platform:

```sh
dotnet run --project LunaBuild.csproj -- build --all
```

Build one target:

```sh
dotnet run --project LunaBuild.csproj -- build --target ObjLoader
```

Build all tests:

```sh
dotnet run --project LunaBuild.csproj -- build --category Tests
```

Build all tools:

```sh
dotnet run --project LunaBuild.csproj -- build --category Tools
```

Force rebuild:

```sh
dotnet run --project LunaBuild.csproj -- build --all --force
```

## Running

`run` builds the selected runnable target first. A normal `Executable` is launched directly from its output directory:

```sh
dotnet run --project LunaBuild.csproj -- run --target RuntimeTest
```

The target name can also be written positionally:

```sh
dotnet run --project LunaBuild.csproj -- run RuntimeTest
```

Pass program arguments after a second `--` separator:

```sh
dotnet run --project LunaBuild.csproj -- run RuntimeTest -- --list
```

For a macOS `Application`, `run` first assembles the target's `.app` bundle and launches it through LaunchServices. An iOS `Application` can be run only when it targets `iphonesimulator` and a Simulator is booted. Running Android applications is not implemented; use `package` and install the resulting APK with Android tools.

### Common Options

* `--target <name>` builds one target and its dependencies.
* `--all` builds all default targets.
* `--category <name>` filters all-target operations by `Engine`, `Tests`, or `Tools`. This option can be repeated or passed as a comma-separated list.
* `--mode <name>` selects `Debug`, `Profile`, or `Release`. Default: `Debug`.
* `--platform <name>` accepts `Windows`, `MacOS`, `Linux`, `Android`, or `IOS`. Default: host platform. The current C++ build executors implement Windows, macOS, Android, and iOS; Linux C++ compilation and linking are not implemented yet.
* `--arch <name>` selects architecture. LunaSDK supports macOS arm64 only.
* Other common architecture values include Windows `x64` and Android ABI names such as `arm64-v8a`.
* `--rhi <name>` selects `D3D12`, `Vulkan`, or `Metal`. Default: platform default.
* `--shared` builds shared libraries.
* `--static` builds static libraries.
* `--property <name=value>` sets one project-defined build property.
* `--api-validation` enables public API validation checks. Debug builds enable this by default. This is declared by LunaSDK's project rules.
* `--contract-assertion` is an alias of `--api-validation` for users migrating from older option names.
* `--thread-safe-assertion` enables thread-safety assertion checks. This is declared by LunaSDK's project rules.
* `--memory-profiler` enables Luna runtime memory profiler instrumentation. This is declared by LunaSDK's project rules.
* `--rhi-debug` enables RHI backend debug layers and validation helpers. This is declared by LunaSDK's project rules.

### Windows Examples

Build Debug D3D12 shared libraries:

```powershell
dotnet run --project LunaBuild.csproj -- build --all --platform Windows --arch x64 --mode Debug --rhi D3D12 --shared
```

Build Debug Vulkan shared libraries:

```powershell
dotnet run --project LunaBuild.csproj -- build --all --platform Windows --arch x64 --mode Debug --rhi Vulkan --shared
```

Build Release D3D12 static libraries:

```powershell
dotnet run --project LunaBuild.csproj -- build --all --platform Windows --arch x64 --mode Release --rhi D3D12 --static
```

### macOS Examples

Build Debug Metal shared libraries for Apple Silicon:

```sh
dotnet run --project LunaBuild.csproj -- build --all --platform MacOS --arch arm64 --mode Debug --rhi Metal --shared
```

Build Release Metal static libraries for Apple Silicon:

```sh
dotnet run --project LunaBuild.csproj -- build --all --platform MacOS --arch arm64 --mode Release --rhi Metal --static
```

### Android Examples

Build the native shared library for `MultiPlatformSample`:

```sh
dotnet run --project LunaBuild.csproj -- build --target MultiPlatformSample --platform Android --arch arm64-v8a --mode Debug --rhi Vulkan
```

Package `MultiPlatformSample` into an APK:

```sh
dotnet run --project LunaBuild.csproj -- package MultiPlatformSample --platform Android --arch arm64-v8a --mode Debug --rhi Vulkan --output build/LunaBuild/AndroidPackages
```

The package command builds the selected `Application` target first, copies the produced native `.so` files into the target's `AndroidProject/app/src/main/jniLibs/<abi>` directory, then invokes the Gradle wrapper for `assembleDebug` or `assembleRelease`. LunaBuild redirects Gradle and Android user-state directories under `build/LunaBuild` so local package runs do not depend on writable user profile cache directories.

### macOS Application Example

Build, assemble, and ad-hoc sign the `MultiPlatformSample` application bundle:

```sh
dotnet run --project LunaBuild.csproj -- package MultiPlatformSample --platform MacOS --arch arm64 --mode Debug --rhi Metal
```

The target must declare an Info.plist template with `AppleInfoPlist(...)`. Runtime files are placed in the bundle's `Resources` directory, and shared Luna module libraries are placed in `Frameworks`.

### iOS/iPadOS Examples

Build and package the iOS package smoke test without code signing:

```sh
dotnet run --project LunaBuild.csproj -- package IOSPackageSmoke --platform IOS --arch arm64 --ios-codesign-identity none --output build/LunaBuild/IOSPackageSmoke.app
```

Use an `.ipa` output path to create an IPA. For a simulator build on Apple Silicon, add `--apple-sdk iphonesimulator --arch arm64`; the current Apple executor accepts arm64. An iOS `Application` target must declare its Info.plist template; signing options can be supplied with `--ios-bundle-identifier`, `--ios-codesign-identity`, and `--ios-provisioning-profile`.

## Cleaning

Clean generated files for the selected graph:

```sh
dotnet run --project LunaBuild.csproj -- clean --all
```

Clean one target:

```sh
dotnet run --project LunaBuild.csproj -- clean --target ObjLoader
```

Remove the whole LunaBuild output directory:

```sh
dotnet run --project LunaBuild.csproj -- clean --full
```

## Installing Artifacts

Install build outputs into an artifact directory:

```sh
dotnet run --project LunaBuild.csproj -- install --all --mode Debug --output ./install/debug
```

The install command copies LunaBuild graph outputs and target metadata outputs such as libraries, binaries, runtime files, public headers, and generated headers.

## Debugging in IDEs

IDE projects generated by LunaBuild are editing and command-entry projects. They do not translate LunaBuild into native MSBuild, Xcode, or VSCode build logic. Build, rebuild, clean, and launch tasks call LunaBuild.

### Visual Studio Code

Generate VSCode tasks, launch configurations, settings, and compile commands:

```sh
dotnet run --project LunaBuild.csproj -- generate --format vscode --all
dotnet run --project LunaBuild.csproj -- generate --format compile_commands --all
```

The generated `.vscode/tasks.json` entries are prefixed with `LunaBuild:`. LunaBuild only replaces entries with this prefix and preserves user-defined tasks.

For clangd, point it at the generated compile commands directory:

```json
{
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build/LunaBuild"
    ]
}
```

### Visual Studio 2022

Generate the Visual Studio solution:

```powershell
dotnet run --project LunaBuild.csproj -- generate --format vs2022 --all --platform Windows --arch x64
```

Open the generated solution under `build/LunaBuild/VS2022`. The projects use NMake-style commands that call LunaBuild for build, rebuild, and clean.

### Xcode

Generate the Xcode project:

```sh
dotnet run --project LunaBuild.csproj -- generate --format xcode --all --platform MacOS --arch arm64
```

Open the generated project under `build/LunaBuild/Xcode`. Xcode schemes call LunaBuild through the generated `lunabuild-xcode.sh` helper inside that output directory.
