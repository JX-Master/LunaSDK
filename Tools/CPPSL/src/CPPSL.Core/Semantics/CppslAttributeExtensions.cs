namespace CPPSL.Core.Semantics;

public static class CppslAttributeExtensions
{
    public static CppslAttribute? FindAttribute(this IEnumerable<CppslAttribute> attributes, string name)
    {
        return attributes.FirstOrDefault(attribute => attribute.Name == name);
    }

    public static int? FirstIntArgument(this CppslAttribute? attribute)
    {
        if (attribute is null || attribute.Arguments.Count == 0)
        {
            return null;
        }
        return int.TryParse(attribute.Arguments[0], out var value) ? value : null;
    }
}
