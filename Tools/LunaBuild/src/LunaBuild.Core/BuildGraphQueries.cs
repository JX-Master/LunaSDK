namespace LunaBuild.Core;

public static class BuildGraphQueries
{
    public static string? FindRunnableOutput(BuildWorkspace workspace, BuildGraph graph, string targetName)
    {
        return IdeProjectModel.FindRunnableOutput(workspace, graph, targetName);
    }
}
