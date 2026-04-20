using Luna.Runtime;

namespace Luna.RHI;

public static class Errors
{
    public static ErrorCategory Category => RuntimeErrors.GetCategoryByName("RHIError");

    public static ErrorCode DeviceHung => GetCode("device_hung");

    public static ErrorCode DeviceReset => GetCode("device_reset");

    public static ErrorCode DeviceRemoved => GetCode("device_removed");

    public static ErrorCode DriverInternalError => GetCode("driver_internal_error");

    public static ErrorCode FrameStatisticsDisjoint => GetCode("frame_statistics_disjoint");

    public static ErrorCode SwapChainOutOfDate => GetCode("swap_chain_out_of_date");

    public static ErrorCode ColorSpaceNotSupported => GetCode("color_space_not_supported");

    private static ErrorCode GetCode(string name)
    {
        return RuntimeErrors.GetCodeByName("RHIError", name);
    }
}
