# LunaSetup

Run `setup.sh` on macOS or `setup.bat` on Windows to install the platform SDK
bundle and the source SDKs required by LunaSDK. Both scripts use this .NET tool.
LunaBuild itself does not download dependencies.

`SourceSdks.json` pins each source SDK's version, archive URL, and SHA-256.
Setup downloads these archives to `build/LunaSetup`, verifies their hashes, and
extracts them into versioned directories under `SDKs`. A partial download or
failed extraction does not replace an installed source SDK. Cached archives are
verified before reuse. Missing installed files cause that source SDK to be
restored from its verified archive.

Source SDK checks run even when the platform SDK bundle is already installed.
`--force` redownloads and replaces both the platform bundle and source SDKs.
Existing versioned platform archives are reused when repairing an installation,
including archives kept directly in `SDKs` by older setup scripts.

## Zip dependencies

- libzip is installed in `SDKs/libzip-1.11.4` from its [official release](https://libzip.org/download/).
- zlib is installed in `SDKs/zlib-1.3.2` from its [official release archive](https://zlib.net/fossils/).

The repository stores only the recipes, source selection lists, and LunaBuild
configuration templates. Upstream library sources and licenses stay in `SDKs`,
which is ignored by Git. Distributors must include the licenses from the
downloaded SDKs when distributing the resulting libraries.

Setup copies `SourceSdks/libzip/*.h.in` to the libzip SDK as configuration
headers and generates `lib/zip_err_str.c` from the downloaded `zip.h` and
`zipint.h`. This follows the upstream error-table generator without requiring
CMake. Unchanged generated files retain their timestamps when setup is rerun.

The MSVC configuration enables the UCRT's secure memory/string functions and
[`_snprintf_s`/`_snwprintf_s`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-s-snprintf-s-l-snwprintf-s-snwprintf-s-l).
Without these capability definitions, libzip's
compatibility macros expand inside UCRT function declarations and cause syntax
errors in Windows SDK headers. After updating a configuration template, rerun
`setup.bat` (or `setup.sh`) to refresh installed headers, including cached SDKs;
`--force` and clearing the SDK cache are not required.

The `libzip` and `zlib` targets in the repository's `ThirdParty.Target.cs` compile
the downloaded C sources as static libraries through LunaBuild, including in
shared-module builds. Store and Deflate are enabled. Optional codecs and AES
backends are disabled, and Luna::Zip rejects encrypted data operations. Only
zlib's core compression sources are built; the gzip file API is excluded.

To upgrade a dependency, update its recipe and SHA-256, compare the selected
source list and libzip configuration templates with upstream, run setup, then
build and run `ZipTest` and `Tests/ZipTest/Interop.py`.
