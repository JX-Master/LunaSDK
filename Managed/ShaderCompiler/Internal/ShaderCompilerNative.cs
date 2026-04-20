using System;
using System.Runtime.InteropServices;

namespace Luna.ShaderCompiler.Internal;

internal static class ShaderCompilerNative
{
    private const string LibraryName = "LunaShaderCompilerC";

    [DllImport(LibraryName, EntryPoint = "luna_shader_compiler_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_shader_compiler_compile")]
    internal static extern UIntPtr Compile(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string source,
        ulong sourceSize,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string sourceName,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string sourceFilePath,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string entryPoint,
        uint targetFormat,
        uint shaderType,
        uint shaderModelMajor,
        uint shaderModelMinor,
        uint optimizationLevel,
        int debug,
        int skipValidation,
        uint matrixPackMode,
        uint metalPlatform,
        out NativeShaderCompileResult outResult);

    [DllImport(LibraryName, EntryPoint = "luna_shader_compiler_free_compile_result")]
    internal static extern void FreeCompileResult(ref NativeShaderCompileResult result);
}
