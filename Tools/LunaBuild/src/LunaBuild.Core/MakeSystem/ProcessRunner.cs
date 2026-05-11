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
        CancellationToken cancellationToken)
    {
        return await RunAsync(fileName, arguments, null, workingDirectory, timeout, cancellationToken);
    }

    public static async Task<ProcessRunResult> RunAsync(
        string fileName,
        IReadOnlyList<string> arguments,
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken)
    {
        return await RunAsync(fileName, null, arguments, workingDirectory, timeout, cancellationToken);
    }

    private static async Task<ProcessRunResult> RunAsync(
        string fileName,
        string? argumentString,
        IReadOnlyList<string>? argumentList,
        string workingDirectory,
        TimeSpan timeout,
        CancellationToken cancellationToken)
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

        using var process = new Process();
        process.StartInfo = startInfo;
        process.OutputDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                output.AppendLine(e.Data);
            }
        };
        process.ErrorDataReceived += (_, e) =>
        {
            if(e.Data is not null)
            {
                output.AppendLine(e.Data);
            }
        };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        try
        {
            await process.WaitForExitAsync(timeoutCts.Token);
        }
        catch(OperationCanceledException) when(!cancellationToken.IsCancellationRequested)
        {
            TryKill(process);
            throw new TimeoutException($"Process timed out after {timeout.TotalSeconds:0}s: {fileName} {FormatArguments(argumentString, argumentList)}");
        }

        return new ProcessRunResult(process.ExitCode, output.ToString());
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
            // Best effort cleanup after timeout.
        }
    }
}
