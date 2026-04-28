namespace CPPSL.Core.Frontend;

public interface ICppslFrontend
{
    CppslFrontendResult Parse(CppslFrontendOptions options);
}
