namespace LunaBuild.Core.MakeSystem;

internal readonly record struct FileState(bool Exists, long Timestamp);

// A lazy snapshot for one dependency-analysis pass. Execution and cache updates
// must read the live filesystem instead, because actions can create or replace files.
internal sealed class FileStateSnapshot
{
    private readonly BuildWorkspace _workspace;
    private readonly Dictionary<string, string> _resolvedPaths = new(StringComparer.Ordinal);
    private readonly Dictionary<string, FileState> _states = new(
        OperatingSystem.IsWindows() ? StringComparer.OrdinalIgnoreCase : StringComparer.Ordinal);

    public FileStateSnapshot(BuildWorkspace workspace)
    {
        _workspace = workspace;
    }

    internal int FilesReadCount => _states.Count;

    public FileState Get(string path)
    {
        if(!_resolvedPaths.TryGetValue(path, out var fullPath))
        {
            fullPath = _workspace.ResolveRepositoryPath(path);
            _resolvedPaths.Add(path, fullPath);
        }
        if(!_states.TryGetValue(fullPath, out var state))
        {
            var file = new FileInfo(fullPath);
            file.Refresh();
            state = file.Exists ? new FileState(true, file.LastWriteTimeUtc.Ticks) : default;
            _states.Add(fullPath, state);
        }
        return state;
    }
}
