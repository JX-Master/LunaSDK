using Luna.Runtime;

namespace Luna.AHI;

public static class Errors
{
    public static ErrorCategory Category => RuntimeErrors.GetCategoryByName("AHIError");

    public static ErrorCode FormatNotSupported => RuntimeErrors.GetCodeByName("AHIError", "format_not_supported");

    public static ErrorCode DeviceTypeNotSupported => RuntimeErrors.GetCodeByName("AHIError", "device_type_not_supported");

    public static ErrorCode ShareModeNotSupported => RuntimeErrors.GetCodeByName("AHIError", "share_mode_not_supported");

    public static ErrorCode NoBackend => RuntimeErrors.GetCodeByName("AHIError", "no_backend");

    public static ErrorCode NoDevice => RuntimeErrors.GetCodeByName("AHIError", "no_device");

    public static ErrorCode ApiNotFound => RuntimeErrors.GetCodeByName("AHIError", "api_not_found");

    public static ErrorCode BadDeviceConfig => RuntimeErrors.GetCodeByName("AHIError", "bad_device_config");

    public static ErrorCode Loop => RuntimeErrors.GetCodeByName("AHIError", "loop");

    public static ErrorCode DeviceNotStarted => RuntimeErrors.GetCodeByName("AHIError", "device_not_started");

    public static ErrorCode DeviceNotStopped => RuntimeErrors.GetCodeByName("AHIError", "device_not_stopped");

    public static ErrorCode FailedToInitBackend => RuntimeErrors.GetCodeByName("AHIError", "failed_to_init_backend");

    public static ErrorCode FailedToOpenBackendDevice => RuntimeErrors.GetCodeByName("AHIError", "failed_to_open_backend_device");

    public static ErrorCode FailedToStartBackendDevice => RuntimeErrors.GetCodeByName("AHIError", "failed_to_start_backend_device");

    public static ErrorCode FailedToStopBackendDevice => RuntimeErrors.GetCodeByName("AHIError", "failed_to_stop_backend_device");
}
