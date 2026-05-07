using System;
using Luna.Runtime;

namespace Luna.Asset.Internal;

internal sealed class NativeAssetObject : ObjectBase
{
    internal NativeAssetObject(IntPtr nativeObject, bool retain)
        : base(nativeObject, retain)
    {
    }
}
