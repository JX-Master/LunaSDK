namespace LunaBuild.Core.MakeSystem;

public sealed class MakeSystemBackend
{
    private readonly Dictionary<string, IMakeActionExecutor> _executors = new(StringComparer.Ordinal);
    private readonly int _maxParallelism;

    public MakeSystemBackend(IEnumerable<IMakeActionExecutor> executors, int? maxParallelism = null)
    {
        foreach(var executor in executors)
        {
            // Executors may accept multiple kinds. The first matching executor
            // wins at execution time; this dictionary is only for exact-kind
            // executors registered through KnownActionExecutor.
            if(executor is KnownActionExecutor known)
            {
                _executors[known.ActionKind] = executor;
            }
        }
        ExtraExecutors = executors.Where(executor => executor is not KnownActionExecutor).ToArray();
        _maxParallelism = Math.Max(1, maxParallelism ?? Environment.ProcessorCount);
    }

    private IReadOnlyList<IMakeActionExecutor> ExtraExecutors { get; }

    public async Task<MakeSystemResult> BuildAsync(
        BuildWorkspace workspace,
        BuildGraph graph,
        IReadOnlyList<string>? requestedTargets = null,
        CancellationToken cancellationToken = default)
    {
        var validated = ValidateGraph(graph);
        var targets = requestedTargets is { Count: > 0 } ? requestedTargets : graph.Targets;
        if(targets.Count == 0)
        {
            return new MakeSystemResult(0, 0, UpToDate: true);
        }

        var orderedNodes = CollectNodes(validated, targets);
        using var cache = MakeSystemCache.Load(workspace);
        var buildInfos = orderedNodes.Select(node => new BuildInfo(node)).ToArray();
        var indexById = buildInfos.Select((info, index) => (info.Node.Id, index))
            .ToDictionary(pair => pair.Id, pair => pair.index, StringComparer.Ordinal);

        var sideOutputIds = new HashSet<string>(
            orderedNodes.SelectMany(node => node.Outputs.Concat(node.Depfiles)),
            StringComparer.Ordinal);

        for(var i = 0; i < buildInfos.Length; ++i)
        {
            var node = buildInfos[i].Node;
            var currentTimestamp = GetNodeTimestamp(workspace, cache, node);
            var maxDependencyTimestamp = 0L;
            var dependencyNeedsBuild = false;

            foreach(var dependencyId in node.Dependencies)
            {
                var dependency = validated.NodesById[dependencyId];
                maxDependencyTimestamp = Math.Max(maxDependencyTimestamp, GetNodeTimestamp(workspace, cache, dependency));
                if(indexById.TryGetValue(dependencyId, out var dependencyIndex) && buildInfos[dependencyIndex].NeedsBuild)
                {
                    dependencyNeedsBuild = true;
                }
            }
            if(cache.TryGet(node.Id, out var cachedRecord) && cachedRecord.ImplicitDependencies is not null)
            {
                foreach(var dependencyPath in cachedRecord.ImplicitDependencies)
                {
                    var fullPath = workspace.ResolveRepositoryPath(dependencyPath);
                    maxDependencyTimestamp = Math.Max(maxDependencyTimestamp, File.Exists(fullPath) ? File.GetLastWriteTimeUtc(fullPath).Ticks : 0);
                }
            }

            var missingFile = node.Kind == BuildGraphNodeKind.File && NodeOutputs(workspace, validated, node).Any(path => !File.Exists(path));
            var commandChanged = !cache.TryGet(node.Id, out var record) || record.ActionPayload != (node.Command ?? string.Empty);

            if(!string.IsNullOrWhiteSpace(node.Command))
            {
                buildInfos[i].NeedsBuild = missingFile || commandChanged || maxDependencyTimestamp > currentTimestamp || dependencyNeedsBuild;
            }
            else if(node.Kind == BuildGraphNodeKind.File && missingFile && !sideOutputIds.Contains(node.Id))
            {
                throw new MakeSystemException($"Input file node is missing and has no action: {node.Path}");
            }
            else if(node.Kind == BuildGraphNodeKind.Phony)
            {
                buildInfos[i].NeedsBuild = dependencyNeedsBuild;
            }
        }

        var actionsToRun = buildInfos.Count(info => info.NeedsBuild && !string.IsNullOrWhiteSpace(info.Node.Command));
        if(actionsToRun == 0)
        {
            cache.Save();
            return new MakeSystemResult(orderedNodes.Count, 0, UpToDate: true);
        }

        foreach(var info in buildInfos.Where(info => info.NeedsBuild))
        {
            var allDependencies = info.Node.Dependencies.Concat(info.Node.OrderOnlyDependencies).Distinct(StringComparer.Ordinal);
            foreach(var dependencyId in allDependencies)
            {
                if(indexById.TryGetValue(dependencyId, out var dependencyIndex) && buildInfos[dependencyIndex].NeedsBuild)
                {
                    ++info.RemainingDependencies;
                    buildInfos[dependencyIndex].Dependents.Add(info);
                }
            }
        }

        var finishedNodes = buildInfos.Count(info => !info.NeedsBuild);
        var executedActions = 0;
        while(finishedNodes < buildInfos.Length)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var ready = buildInfos
                .Where(info => info.NeedsBuild && !info.Scheduled && info.RemainingDependencies == 0)
                .Take(_maxParallelism)
                .ToArray();
            if(ready.Length == 0)
            {
                throw new MakeSystemException("Internal MakeSystem scheduler deadlock.");
            }

            foreach(var info in ready)
            {
                info.Scheduled = true;
            }

            await Task.WhenAll(ready.Select(info => ExecuteNodeAsync(workspace, graph, validated, cache, info, cancellationToken)));
            foreach(var info in ready)
            {
                info.Finished = true;
                ++finishedNodes;
                if(!string.IsNullOrWhiteSpace(info.Node.Command))
                {
                    ++executedActions;
                }
                foreach(var dependent in info.Dependents)
                {
                    --dependent.RemainingDependencies;
                }
            }
        }

        cache.Save();
        return new MakeSystemResult(orderedNodes.Count, executedActions, UpToDate: false);
    }

    private async Task ExecuteNodeAsync(
        BuildWorkspace workspace,
        BuildGraph graph,
        ValidatedGraph validated,
        MakeSystemCache cache,
        BuildInfo info,
        CancellationToken cancellationToken)
    {
        var node = info.Node;
        if(string.IsNullOrWhiteSpace(node.Command))
        {
            UpdateCache(workspace, validated, cache, node);
            return;
        }

        var actionKind = BuildActionKind.Extract(node.Command);
        var executor = ResolveExecutor(actionKind, node.Id);
        var context = new MakeActionContext(
            workspace,
            graph,
            node,
            actionKind,
            node.Command,
            node.Dependencies.Select(id => validated.NodesById[id]).ToArray(),
            node.Outputs.Select(id => validated.NodesById[id]).ToArray(),
            node.Depfiles.Select(id => validated.NodesById[id]).ToArray());

        await executor.ExecuteAsync(context, cancellationToken);
        foreach(var output in NodeOutputs(workspace, validated, node))
        {
            if(!File.Exists(output))
            {
                throw new MakeSystemException($"Action `{actionKind}` for node `{node.Id}` did not produce output `{output}`.");
            }
        }
        UpdateCache(workspace, validated, cache, node);
    }

    private IMakeActionExecutor ResolveExecutor(string actionKind, string nodeId)
    {
        if(_executors.TryGetValue(actionKind, out var executor))
        {
            return executor;
        }
        foreach(var extra in ExtraExecutors)
        {
            if(extra.CanExecute(actionKind))
            {
                return extra;
            }
        }
        throw new MissingMakeActionExecutorException(actionKind, nodeId);
    }

    private static ValidatedGraph ValidateGraph(BuildGraph graph)
    {
        if(graph.Version != 1)
        {
            throw new MakeSystemException($"Unsupported build graph version: {graph.Version}");
        }

        var nodesById = new Dictionary<string, BuildGraphNode>(StringComparer.Ordinal);
        var filePaths = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach(var node in graph.Nodes)
        {
            if(!nodesById.TryAdd(node.Id, node))
            {
                throw new MakeSystemException($"Duplicate build node id: {node.Id}");
            }

            if(node.Kind == BuildGraphNodeKind.File)
            {
                if(string.IsNullOrWhiteSpace(node.Path))
                {
                    throw new MakeSystemException($"File node has no path: {node.Id}");
                }
                if(!filePaths.Add(node.Path))
                {
                    throw new MakeSystemException($"Duplicate file node path: {node.Path}");
                }
            }
        }

        foreach(var target in graph.Targets)
        {
            RequireNode(nodesById, target, "target");
        }

        foreach(var node in graph.Nodes)
        {
            foreach(var dependency in node.Dependencies)
            {
                RequireNode(nodesById, dependency, $"dependency of {node.Id}");
            }
            foreach(var dependency in node.OrderOnlyDependencies)
            {
                RequireNode(nodesById, dependency, $"order-only dependency of {node.Id}");
            }
            foreach(var output in node.Outputs)
            {
                RequireNode(nodesById, output, $"output of {node.Id}");
            }
            foreach(var depfile in node.Depfiles)
            {
                RequireNode(nodesById, depfile, $"depfile of {node.Id}");
            }
        }

        return new ValidatedGraph(nodesById);
    }

    private static void RequireNode(Dictionary<string, BuildGraphNode> nodesById, string nodeId, string usage)
    {
        if(!nodesById.ContainsKey(nodeId))
        {
            throw new MakeSystemException($"Undefined build node referenced as {usage}: {nodeId}");
        }
    }

    private static IReadOnlyList<BuildGraphNode> CollectNodes(ValidatedGraph graph, IReadOnlyList<string> targets)
    {
        var ordered = new List<BuildGraphNode>();
        var visited = new HashSet<string>(StringComparer.Ordinal);
        var visiting = new HashSet<string>(StringComparer.Ordinal);
        foreach(var target in targets)
        {
            Visit(target, graph, visited, visiting, ordered);
        }
        return ordered;
    }

    private static void Visit(
        string nodeId,
        ValidatedGraph graph,
        HashSet<string> visited,
        HashSet<string> visiting,
        List<BuildGraphNode> ordered)
    {
        if(visiting.Contains(nodeId))
        {
            throw new MakeSystemException($"Circular dependency detected at node: {nodeId}");
        }
        if(!visited.Add(nodeId))
        {
            return;
        }

        visiting.Add(nodeId);
        var node = graph.NodesById[nodeId];
        foreach(var dependency in node.Dependencies.Concat(node.OrderOnlyDependencies).Concat(node.Outputs).Concat(node.Depfiles))
        {
            Visit(dependency, graph, visited, visiting, ordered);
        }
        visiting.Remove(nodeId);
        ordered.Add(node);
    }

    private static long GetNodeTimestamp(BuildWorkspace workspace, MakeSystemCache cache, BuildGraphNode node)
    {
        if(node.Kind == BuildGraphNodeKind.File)
        {
            if(node.Path is null)
            {
                return 0;
            }
            var path = workspace.ResolveRepositoryPath(node.Path);
            return File.Exists(path) ? File.GetLastWriteTimeUtc(path).Ticks : 0;
        }

        return cache.TryGet(node.Id, out var record) ? record.Timestamp : 0;
    }

    private static IReadOnlyList<string> NodeOutputs(BuildWorkspace workspace, ValidatedGraph graph, BuildGraphNode node)
    {
        var outputs = new List<string>();
        if(node.Kind == BuildGraphNodeKind.File && node.Path is not null)
        {
            outputs.Add(workspace.ResolveRepositoryPath(node.Path));
        }
        outputs.AddRange(node.Outputs.Concat(node.Depfiles)
            .Select(id => graph.NodesById[id])
            .Where(output => output.Kind == BuildGraphNodeKind.File && output.Path is not null)
            .Select(output => workspace.ResolveRepositoryPath(output.Path!)));
        return outputs;
    }

    private static void UpdateCache(BuildWorkspace workspace, ValidatedGraph graph, MakeSystemCache cache, BuildGraphNode node)
    {
        var outputs = NodeOutputs(workspace, graph, node);
        var timestamp = outputs.Count == 0
            ? DateTime.UtcNow.Ticks
            : outputs.Where(File.Exists).Select(path => File.GetLastWriteTimeUtc(path).Ticks).DefaultIfEmpty(DateTime.UtcNow.Ticks).Min();
        var depfiles = node.Depfiles
            .Select(id => graph.NodesById[id])
            .Where(depfile => depfile.Path is not null)
            .Select(depfile => depfile.Path!)
            .ToArray();
        var implicitDependencies = depfiles
            .Select(workspace.ResolveRepositoryPath)
            .Where(File.Exists)
            .SelectMany(ReadDepfileInputs)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Select(workspace.ToRepositoryRelativePath)
            .ToArray();
        cache.Set(node.Id, new MakeSystemCacheRecord(
            node.Command ?? string.Empty,
            timestamp,
            outputs.Select(workspace.ToRepositoryRelativePath).ToArray(),
            depfiles,
            implicitDependencies));
    }

    private static IEnumerable<string> ReadDepfileInputs(string depfile)
    {
        var data = File.ReadAllText(depfile);
        var afterColon = false;
        var token = new List<char>();
        for(var i = 0; i < data.Length; ++i)
        {
            var ch = data[i];
            if(ch == '\\' && i + 1 < data.Length)
            {
                var next = data[++i];
                if(next is '\r' or '\n')
                {
                    if(next == '\r' && i + 1 < data.Length && data[i + 1] == '\n')
                    {
                        ++i;
                    }
                    continue;
                }
                token.Add(next);
                continue;
            }
            if(!afterColon)
            {
                if(ch == ':' && (i + 1 == data.Length || char.IsWhiteSpace(data[i + 1])))
                {
                    afterColon = true;
                    token.Clear();
                }
                continue;
            }
            if(char.IsWhiteSpace(ch))
            {
                if(token.Count > 0)
                {
                    yield return new string(token.ToArray());
                    token.Clear();
                }
                continue;
            }
            token.Add(ch);
        }
        if(afterColon && token.Count > 0)
        {
            yield return new string(token.ToArray());
        }
    }

    private sealed record ValidatedGraph(Dictionary<string, BuildGraphNode> NodesById);

    private sealed class BuildInfo
    {
        public BuildInfo(BuildGraphNode node)
        {
            Node = node;
        }

        public BuildGraphNode Node { get; }
        public bool NeedsBuild { get; set; }
        public bool Scheduled { get; set; }
        public bool Finished { get; set; }
        public int RemainingDependencies { get; set; }
        public List<BuildInfo> Dependents { get; } = new();
    }
}

public abstract class KnownActionExecutor : IMakeActionExecutor
{
    protected KnownActionExecutor(string actionKind)
    {
        ActionKind = actionKind;
    }

    public string ActionKind { get; }

    public bool CanExecute(string actionKind) => string.Equals(ActionKind, actionKind, StringComparison.Ordinal);

    public abstract Task ExecuteAsync(MakeActionContext context, CancellationToken cancellationToken);
}
