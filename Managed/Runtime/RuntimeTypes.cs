using System;
using System.Runtime.InteropServices;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimeTypes
{
    public static Type FromNativeHandle(IntPtr handle)
    {
        return new Type(handle);
    }

    public static Type GetByGuid(Guid guid)
    {
        return new Type(RuntimeNativeGenerated.TypeGetByGuid(in guid));
    }

    public static Type GetObjectType(IObject obj)
    {
        ArgumentNullException.ThrowIfNull(obj);
        return new Type(RuntimeNativeGenerated.TypeGetObjectType(obj.GetNativeHandle()));
    }

    public static Type GetBase(Type type)
    {
        return type.IsValid ? new Type(RuntimeNativeGenerated.TypeGetBase(type.Handle)) : default;
    }

    public static Guid GetGuid(Type type)
    {
        if (!type.IsValid)
        {
            return default;
        }
        RuntimeNativeGenerated.TypeGetGuid(type.Handle, out var guid);
        return guid;
    }

    public static string GetName(Type type)
    {
        return type.IsValid ? PtrToString(RuntimeNativeGenerated.TypeGetName(type.Handle)) : string.Empty;
    }

    public static string GetAlias(Type type)
    {
        return type.IsValid ? PtrToString(RuntimeNativeGenerated.TypeGetAlias(type.Handle)) : string.Empty;
    }

    public static ulong GetSize(Type type)
    {
        return type.IsValid ? RuntimeNativeGenerated.TypeGetSize(type.Handle) : 0;
    }

    public static ulong GetAlignment(Type type)
    {
        return type.IsValid ? RuntimeNativeGenerated.TypeGetAlignment(type.Handle) : 0;
    }

    public static bool IsType(Type type, Type targetType)
    {
        return type.IsValid && targetType.IsValid && RuntimeNativeGenerated.TypeIsType(type.Handle, targetType.Handle) != 0;
    }

    public static bool ObjectIsType(IObject obj, Type targetType)
    {
        ArgumentNullException.ThrowIfNull(obj);
        return targetType.IsValid && RuntimeNativeGenerated.ObjectIsType(obj.GetNativeHandle(), targetType.Handle) != 0;
    }

    private static string PtrToString(IntPtr value)
    {
        return Marshal.PtrToStringUTF8(value) ?? string.Empty;
    }
}
