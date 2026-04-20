using System;

namespace Luna.Runtime;

public interface IObject : IDisposable
{
    bool IsDisposed { get; }

    Type Type { get; }

    IntPtr GetNativeHandle();

    bool IsA(Type type);
}
