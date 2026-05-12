using System.Text.Json;

namespace LunaBuild.Core.MakeSystem;

internal sealed record MakeSystemCacheFile(
    int Version,
    Dictionary<string, MakeSystemCacheRecord> Records);

internal sealed record MakeSystemCacheRecord(
    string ActionPayload,
    long Timestamp,
    string[] Outputs,
    string[] Depfiles,
    string[] ImplicitDependencies);

internal sealed class MakeSystemCache : IDisposable
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
    };

    private readonly string _path;
    private readonly FileStream _lockFile;
    private readonly Dictionary<string, MakeSystemCacheRecord> _records;
    private readonly object _recordsLock = new();

    private MakeSystemCache(string path, FileStream lockFile, Dictionary<string, MakeSystemCacheRecord> records)
    {
        _path = path;
        _lockFile = lockFile;
        _records = records;
    }

    public static MakeSystemCache Load(BuildWorkspace workspace)
    {
        var path = Path.Combine(workspace.BuildDirectory, "MakeSystem", "cache.json");
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        var lockFile = AcquireLock(path + ".lock");
        if(!File.Exists(path))
        {
            return new MakeSystemCache(path, lockFile, new Dictionary<string, MakeSystemCacheRecord>(StringComparer.Ordinal));
        }

        var cache = JsonSerializer.Deserialize<MakeSystemCacheFile>(File.ReadAllText(path), JsonOptions);
        if(cache is null || cache.Version != 1)
        {
            return new MakeSystemCache(path, lockFile, new Dictionary<string, MakeSystemCacheRecord>(StringComparer.Ordinal));
        }
        return new MakeSystemCache(path, lockFile, new Dictionary<string, MakeSystemCacheRecord>(cache.Records, StringComparer.Ordinal));
    }

    public bool TryGet(string nodeId, out MakeSystemCacheRecord record)
    {
        lock(_recordsLock)
        {
            return _records.TryGetValue(nodeId, out record!);
        }
    }

    public void Set(string nodeId, MakeSystemCacheRecord record)
    {
        lock(_recordsLock)
        {
            _records[nodeId] = record;
        }
    }

    public void PruneTo(IEnumerable<string> nodeIds)
    {
        lock(_recordsLock)
        {
            var live = nodeIds.ToHashSet(StringComparer.Ordinal);
            foreach(var nodeId in _records.Keys.ToArray())
            {
                if(!live.Contains(nodeId))
                {
                    _records.Remove(nodeId);
                }
            }
        }
    }

    public void Save()
    {
        var directory = Path.GetDirectoryName(_path);
        if(!string.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        Dictionary<string, MakeSystemCacheRecord> records;
        lock(_recordsLock)
        {
            records = new Dictionary<string, MakeSystemCacheRecord>(_records, StringComparer.Ordinal);
        }

        var file = new MakeSystemCacheFile(Version: 1, Records: records);
        var tempPath = _path + ".tmp";
        File.WriteAllText(tempPath, JsonSerializer.Serialize(file, JsonOptions));
        File.Move(tempPath, _path, overwrite: true);
    }

    public void Dispose()
    {
        _lockFile.Dispose();
    }

    private static FileStream AcquireLock(string path)
    {
        var deadline = DateTime.UtcNow + TimeSpan.FromSeconds(30);
        while(true)
        {
            try
            {
                return new FileStream(path, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None);
            }
            catch(IOException) when(DateTime.UtcNow < deadline)
            {
                Thread.Sleep(100);
            }
        }
    }
}
