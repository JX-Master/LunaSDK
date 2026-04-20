using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class Runtime
{
    public static void Init()
    {
        if (RuntimeNative.Init() == 0)
        {
            throw new ErrorException(UIntPtr.Zero, UIntPtr.Zero, "Luna runtime initialization failed.");
        }
    }

    public static bool IsInitialized => RuntimeNative.IsInitialized() != 0;

    public static void Close()
    {
        RuntimeNative.Close();
    }
}
