using Luna.Runtime;

namespace Luna.VFS;

public static class Errors
{
    public static ErrorCategory Category => RuntimeErrors.GetCategoryByName("VFSError");

    public static ErrorCode DriverNotFound => RuntimeErrors.GetCodeByName("VFSError", "driver_not_found");
}
