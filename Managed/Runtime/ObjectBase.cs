using System;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public abstract class ObjectBase : IObject
{
    private readonly NativeObjectHandle _handle;

    protected ObjectBase(IntPtr nativeObject, bool retain)
    {
        if (nativeObject == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeObject));
        }
        if (retain)
        {
            RuntimeNativeGenerated.ObjectRetain(nativeObject);
        }
        _handle = new NativeObjectHandle(nativeObject);
    }

    public bool IsDisposed => _handle.IsClosed || _handle.IsInvalid;

    public Type Type => RuntimeTypes.GetObjectType(this);

    public IntPtr GetNativeHandle()
    {
        EnsureNotDisposed();
        return _handle.DangerousGetHandle();
    }

    public bool IsA(Type type)
    {
        return RuntimeTypes.ObjectIsType(this, type);
    }

    protected IntPtr QueryInterface(Guid interfaceId)
    {
        return RuntimeNativeGenerated.ObjectQueryInterface(GetNativeHandle(), in interfaceId);
    }

    public void Dispose()
    {
        _handle.Dispose();
        GC.SuppressFinalize(this);
    }

    internal protected void EnsureNotDisposed()
    {
        if (IsDisposed)
        {
            throw new ObjectDisposedException(GetType().Name);
        }
    }
}
