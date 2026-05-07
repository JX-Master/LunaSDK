namespace CPPSL.Core.Artifacts;

public enum CppslOutputTarget
{
    Hlsl,
    Glsl,
    Msl,
    Reflection
}

public static class CppslOutputTargets
{
    public static readonly IReadOnlyList<CppslOutputTarget> Default = new[]
    {
        CppslOutputTarget.Hlsl,
        CppslOutputTarget.Glsl,
        CppslOutputTarget.Msl,
        CppslOutputTarget.Reflection
    };

    public static IReadOnlyList<CppslOutputTarget> Normalize(IReadOnlyList<CppslOutputTarget>? targets)
    {
        if (targets is null || targets.Count == 0)
        {
            return Default;
        }

        return targets.Distinct().ToArray();
    }

    public static string GetFileExtension(this CppslOutputTarget target)
    {
        return target switch
        {
            CppslOutputTarget.Hlsl => ".hlsl",
            CppslOutputTarget.Glsl => ".glsl",
            CppslOutputTarget.Msl => ".metal",
            CppslOutputTarget.Reflection => ".reflection.json",
            _ => throw new ArgumentOutOfRangeException(nameof(target), target, null)
        };
    }

    public static string GetDisplayName(this CppslOutputTarget target)
    {
        return target switch
        {
            CppslOutputTarget.Hlsl => "HLSL",
            CppslOutputTarget.Glsl => "GLSL",
            CppslOutputTarget.Msl => "MSL",
            CppslOutputTarget.Reflection => "Reflection",
            _ => target.ToString()
        };
    }
}
