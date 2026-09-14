using System.Diagnostics;
using System.Text;

namespace LunaBuild.Core.MakeSystem;

internal sealed record ProcessRunResult(int ExitCode, string Output);

internal static class ProcessRunner
{
    public static async Task<ProcessRunResult> RunAsync(
        string fileName,
        string arguments,
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken,
        IReadOnlyDictionary<string, string>? environmentVariables = null)
    {
        return await RunAsync(fileName, arguments, null, workingDirectory, timeout, cancellationToken, environmentVariables);
    }

    public static async Task<ProcessRunResult> RunAsync(
        string fileName,
        IReadOnlyList<string> arguments,
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken,
        IReadOnlyDictionary<string, string>? environmentVariables = null)
    {
        return await RunAsync(fileName, null, arguments, workingDirectory, timeout, cancellationToken, environmentVariables);
    }

    private static async Task<ProcessRunResult> RunAsync(
        string fileName,
        string? argumentString,
        IReadOnlyList<string>? argumentList,
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken,
        IReadOnlyDictionary<string, string>? environmentVariables)
    {
        using var timeoutCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeoutCts.CancelAfter(timeout);

        var output = new StringBuilder();
        var startInfo = new ProcessStartInfo
        {
            FileName = fileName,
            WorkingDirectory = workingDirectory,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            StandardOutputEncoding = Encoding.Default,
            StandardErrorEncoding = Encoding.Default,
            CreateNoWindow = true,
        };
        if(argumentList is not null)
        {
            foreach(var argument in argumentList)
            {
                startInfo.ArgumentList.Add(argument);
            }
        }
        else
        {
            startInfo.Arguments = argumentString ?? string.Empty;
        }
        ApplyEnvironmentVariables(startInfo, environmentVariables);

        using var process = new Process();
        process.StartInfo = startInfo;
        var outputLock = new object();
        process.OutputDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                lock(outputLock)
                {
                    output.AppendLine(e.Data);
                }
            }
        };
        process.ErrorDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                lock(outputLock)
                {
                    output.AppendLine(e.Data);
                }
            }
        };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        try
        {
            await process.WaitForExitAsync(timeoutCts.Token);
        }
        catch(OperationCanceledException)
        {
            TryKill(process);
            if(cancellationToken.IsCancellationRequested)
            {
                throw;
            }
            throw new TimeoutException($"Process timed out after {timeout.TotalSeconds:0}s: {fileName} {FormatArguments(argumentString, argumentList)}");
        }

        lock(outputLock)
        {
            return new ProcessRunResult(process.ExitCode, output.ToString());
        }
    }

    internal static void ApplyEnvironmentVariables(
        ProcessStartInfo startInfo,
        IReadOnlyDictionary<string, string>? environmentVariables)
    {
        if(environmentVariables is null)
        {
            return;
        }

        foreach(var variable in environmentVariables)
        {
            foreach(var existingKey in startInfo.Environment.Keys
                .Where(key => key.Equals(variable.Key, StringComparison.OrdinalIgnoreCase))
                .ToArray())
            {
                startInfo.Environment.Remove(existingKey);
            }
            startInfo.Environment[variable.Key] = variable.Value;
        }
    }

    private static string FormatArguments(string? argumentString, IReadOnlyList<string>? argumentList)
    {
        return argumentList is null ? argumentString ?? string.Empty : string.Join(" ", argumentList);
    }

    private static void TryKill(Process process)
    {
        try
        {
            if(!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
            }
        }
        catch
        {
            // Best effort cleanup after timeout or build cancellation.
        }
    }
}
