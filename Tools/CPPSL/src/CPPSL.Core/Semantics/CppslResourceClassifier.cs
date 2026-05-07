namespace CPPSL.Core.Semantics;

internal static class CppslResourceClassifier
{
    public const string ConstantBuffer = "constant_buffer";
    public const string StructuredBuffer = "structured_buffer";
    public const string RwStructuredBuffer = "rw_structured_buffer";
    public const string Texture = "texture";
    public const string RwTexture = "rw_texture";
    public const string Sampler = "sampler";
    public const string AccelerationStructure = "acceleration_structure";

    public static string? ClassifyGlobal(string type, IReadOnlyList<CppslAttribute> attributes)
    {
        return ClassifyFromAttributes(attributes) ??
            ClassifyFromType(type, allowStructuredBufferTemplates: true);
    }

    public static string? ClassifyDescriptorSetField(string type, IReadOnlyList<CppslAttribute> attributes)
    {
        return ClassifyFromAttributes(attributes) ??
            ClassifyFromType(type, allowStructuredBufferTemplates: false);
    }

    public static bool IsResourceTypeOrAttribute(string type, IReadOnlyList<CppslAttribute> attributes)
    {
        return ClassifyFromAttributes(attributes) is not null ||
            ClassifyFromType(type, allowStructuredBufferTemplates: true) is not null;
    }

    public static bool IsTextureType(string type)
    {
        var normalized = type.Trim();
        return (normalized.StartsWith("Texture", StringComparison.Ordinal) ||
            normalized.StartsWith("DepthTexture", StringComparison.Ordinal)) &&
            normalized.Contains('<', StringComparison.Ordinal);
    }

    public static bool IsRwTextureType(string type)
    {
        var normalized = type.Trim();
        return normalized.StartsWith("RWTexture", StringComparison.Ordinal) &&
            normalized.Contains('<', StringComparison.Ordinal);
    }

    private static string? ClassifyFromAttributes(IReadOnlyList<CppslAttribute> attributes)
    {
        if (attributes.FindAttribute("cbuffer") is not null)
        {
            return ConstantBuffer;
        }
        if (attributes.FindAttribute("structured_buffer") is not null ||
            attributes.FindAttribute("sbuffer") is not null)
        {
            return StructuredBuffer;
        }
        if (attributes.FindAttribute("rwstructured_buffer") is not null ||
            attributes.FindAttribute("rw_structured_buffer") is not null ||
            attributes.FindAttribute("rwsbuffer") is not null)
        {
            return RwStructuredBuffer;
        }

        return null;
    }

    private static string? ClassifyFromType(string type, bool allowStructuredBufferTemplates)
    {
        var normalized = type.Trim();
        if (allowStructuredBufferTemplates)
        {
            if (normalized.StartsWith("ConstantBuffer<", StringComparison.Ordinal))
            {
                return ConstantBuffer;
            }
            if (normalized.StartsWith("StructuredBuffer<", StringComparison.Ordinal))
            {
                return StructuredBuffer;
            }
            if (normalized.StartsWith("RWStructuredBuffer<", StringComparison.Ordinal))
            {
                return RwStructuredBuffer;
            }
        }
        if (IsTextureType(normalized))
        {
            return Texture;
        }
        if (IsRwTextureType(normalized))
        {
            return RwTexture;
        }
        if (normalized == "SamplerState")
        {
            return Sampler;
        }
        if (normalized == "AccelerationStructure")
        {
            return AccelerationStructure;
        }

        return null;
    }
}
