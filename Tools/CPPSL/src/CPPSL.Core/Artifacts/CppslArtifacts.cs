namespace CPPSL.Core.Artifacts;

public sealed record CppslArtifacts(
    string IrPath,
    IReadOnlyDictionary<CppslOutputTarget, string> Outputs)
{
    public string GetOutputPath(CppslOutputTarget target)
    {
        return Outputs[target];
    }
}
