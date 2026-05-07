using Luna.Runtime;

namespace Luna.Image;

public static class Errors
{
    public static ErrorCategory Category => RuntimeErrors.GetCategoryByName("ImageError");

    public static ErrorCode FileParseError => RuntimeErrors.GetCodeByName("ImageError", "file_parse_error");
}
