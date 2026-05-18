namespace LunaBuild.Core.Targets;

public sealed class LuaTargetRules : TargetRules
{
    public LuaTargetRules()
        : base(
            name: "Lua",
            targetDirectory: "Modules/Luna/Lua",
            rulesPath: "Modules/Luna/Lua/Lua.Target.cs")
    {
        Headers("*.hpp", "Source/lua-5.5.0/src/*.h", "Source/lua-5.5.0/src/*.hpp");
        Sources("Source/*.cpp", "Source/lua-5.5.0/src/*.c");
        ExcludeSources("Source/lua-5.5.0/src/lua.c", "Source/lua-5.5.0/src/luac.c");
        IncludeDirectories("Source/lua-5.5.0/src");
        DependsOn("Runtime");
    }
}
