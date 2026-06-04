## Building
### Command usage
- Call `setup.sh` on macOS or `setup.bat` on Windows first if `SDKs` folder is not present or is not complete (which may take several minutes and may require promotion since it needs to download zips from internet).
- Use cross-platform uniform entry `dotnet run --project LunaBuild.csproj -- ...` to perform building tasks.
- Use `dotnet run --project LunaBuild.csproj -- build --target XXX` to build target `XXX`, if `--target XXX` is not specified, all targets will be built.
- Use `dotnet run --project LunaBuild.csproj -- run --target XXX` to launch target `XXX` in terminal. `all` is not supported for this action.
- Consult `Tools/LunaBuild/README.md` for other supported actions.

### Notice
- Do not run multiple `dotnet run` commands concurrently.
- If you need to use building artifacts, make sure the building process is finished.

## Coding
- Read docs FIRST and CAREFULLY! Docs are located in `LunaSDK-Docs`.
- All codes written in LunaSDK must conform to [Coding Convention](LunaSDK-Docs/Manual/Coding%20Convention.md).
- All public headers in LunaSDK must be well documented using Doxygen syntax.
- Use English to write codes and docs in LunaSDK, unless the user explicitly asked you to use another language.