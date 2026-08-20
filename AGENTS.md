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
- Use tools to generate GUID when needed, do not guess one. Always check for duplication after generating GUID and make sure every GUID is unique.
- Prefer using existing features provided by LunaSDK rather than reinventing the wheel, if using such features will introduce unnecessary module dependency, ask the user.

## ADR (Architecture Decision Record)
- Always ask for the user whether to draft a new ADR if new features are introduced or big feature changes are made. Bugfixes generally do not require a new ADR.
- ADRs are placed in `LunaSDK-Docs/Manual/ADRs`. Read [ADR template](LunaSDK-Docs/ADRs/ADR-0001%20ADR%20template.md) first to know how to write a new ADR.
- If a new ADR is required, write ADR and refer to it before writing real codes.

## Documentation
- Always ask the user whether to draft new docs if new modules or features are implemented.
- New docs are placed in LunaSDK-Docs/Drafts. Resource files referred by docs are placed in LunaSDK-Docs/Res. Docs are in Obsidian-specific markdown format.
- One module can have multiple doc pages (markdown files), one page per feature. Prevent writing one big page for all features of one module. See docs of RHI module as an example.
- The documentation should include the following contents, in order:
    1. The designed functionality of the feature, like what this feature does, what problem this feature solves, and how it provides the function.
    2. The concepts used by the feature. Concepts are used to describe parts that compose the feature, this may map to concrete types, functions, etc, and may also refer to higher level concepts.
    3. The programming guide of every component of the feature, from feature initialization to shutdown.
    4. When needed, provide code examples that describe the feature.
- The documentation should not mention ADR pages, they are kept in separate sites.