using System;
using System.Runtime.InteropServices;
using System.Text;
using Luna.Runtime;
using Luna.ShaderCompiler.Internal;

namespace Luna.ShaderCompiler;

public static class ShaderCompilerModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the ShaderCompiler module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ShaderCompilerNative.InitModule()));
    }

    public static ShaderCompileResult Compile(ShaderCompileParameters parameters)
    {
        ArgumentNullException.ThrowIfNull(parameters);
        ArgumentException.ThrowIfNullOrEmpty(parameters.Source);
        ArgumentException.ThrowIfNullOrEmpty(parameters.EntryPoint);

        var sourceBytes = Encoding.UTF8.GetBytes(parameters.Source);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ShaderCompilerNative.Compile(
            parameters.Source,
            (ulong)sourceBytes.Length,
            parameters.SourceName,
            parameters.SourceFilePath,
            parameters.EntryPoint,
            (uint)parameters.TargetFormat,
            (uint)parameters.ShaderType,
            parameters.ShaderModelMajor,
            parameters.ShaderModelMinor,
            (uint)parameters.OptimizationLevel,
            parameters.Debug ? 1 : 0,
            parameters.SkipValidation ? 1 : 0,
            (uint)parameters.MatrixPackMode,
            (uint)parameters.MetalPlatform,
            out var nativeResult)));
        try
        {
            if (nativeResult.DataSize > int.MaxValue)
            {
                throw new InvalidOperationException("Compiled shader data is too large to copy into a managed byte array.");
            }
            var data = new byte[(int)nativeResult.DataSize];
            if (nativeResult.DataSize > 0)
            {
                Marshal.Copy(nativeResult.Data, data, 0, data.Length);
            }
            var entryPoint = Marshal.PtrToStringUTF8(nativeResult.EntryPoint) ?? string.Empty;
            return new ShaderCompileResult(
                data,
                (ShaderCompilerTargetFormat)nativeResult.Format,
                entryPoint,
                nativeResult.MetalNumThreadsX,
                nativeResult.MetalNumThreadsY,
                nativeResult.MetalNumThreadsZ);
        }
        finally
        {
            ShaderCompilerNative.FreeCompileResult(ref nativeResult);
        }
    }
}
