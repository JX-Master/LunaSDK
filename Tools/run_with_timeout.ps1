param(
    [Parameter(Mandatory = $true)]
    [string]$FilePath,

    [string[]]$ArgumentList = @(),

    [int]$TimeoutSeconds = 300,

    [string]$WorkingDirectory = (Get-Location).Path
)

$ErrorActionPreference = "Stop"

if ($TimeoutSeconds -le 0) {
    throw "TimeoutSeconds must be greater than 0."
}

$resolvedFile = (Resolve-Path -LiteralPath $FilePath).Path
$resolvedWorkingDirectory = (Resolve-Path -LiteralPath $WorkingDirectory).Path
$logDirectory = Join-Path $resolvedWorkingDirectory "build\LunaBuild\ProcessLogs"
New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
$logId = [Guid]::NewGuid().ToString("N")
$stdoutPath = Join-Path $logDirectory "$logId.out.log"
$stderrPath = Join-Path $logDirectory "$logId.err.log"

function ConvertTo-CommandLineArgument([string]$Argument) {
    if($null -eq $Argument -or $Argument.Length -eq 0) {
        return '""'
    }
    if($Argument -notmatch '[\s"]') {
        return $Argument
    }

    $builder = [System.Text.StringBuilder]::new()
    [void]$builder.Append('"')
    $backslashes = 0
    foreach($character in $Argument.ToCharArray()) {
        if($character -eq '\') {
            $backslashes += 1
        }
        elseif($character -eq '"') {
            [void]$builder.Append('\' * (($backslashes * 2) + 1))
            [void]$builder.Append('"')
            $backslashes = 0
        }
        else {
            if($backslashes -gt 0) {
                [void]$builder.Append('\' * $backslashes)
                $backslashes = 0
            }
            [void]$builder.Append($character)
        }
    }
    if($backslashes -gt 0) {
        [void]$builder.Append('\' * ($backslashes * 2))
    }
    [void]$builder.Append('"')
    return $builder.ToString()
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $resolvedFile
$startInfo.WorkingDirectory = $resolvedWorkingDirectory
$startInfo.UseShellExecute = $false
$startInfo.CreateNoWindow = $true
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true
if($ArgumentList.Count -gt 0) {
    $startInfo.Arguments = ($ArgumentList | ForEach-Object { ConvertTo-CommandLineArgument $_ }) -join ' '
}

$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo
[void]$process.Start()
$stdoutTask = $process.StandardOutput.ReadToEndAsync()
$stderrTask = $process.StandardError.ReadToEndAsync()

$deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
while((-not $process.HasExited) -and ([DateTime]::UtcNow -lt $deadline)) {
    Start-Sleep -Milliseconds 100
    $process.Refresh()
}

if (-not $process.HasExited) {
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 500
    $process.Refresh()
    $process.WaitForExit()
    $stdoutTask.Wait()
    $stderrTask.Wait()
    [System.IO.File]::WriteAllText($stdoutPath, $stdoutTask.Result, [System.Text.Encoding]::UTF8)
    [System.IO.File]::WriteAllText($stderrPath, $stderrTask.Result, [System.Text.Encoding]::UTF8)
    if($stdoutTask.Result.Length -gt 0) {
        Write-Host $stdoutTask.Result
    }
    if($stderrTask.Result.Length -gt 0) {
        Write-Error $stderrTask.Result
    }
    Write-Host "TIMEOUT after $TimeoutSeconds seconds"
    Write-Host "stdout: $stdoutPath"
    Write-Host "stderr: $stderrPath"
    exit 124
}

$process.WaitForExit()
$stdoutTask.Wait()
$stderrTask.Wait()
[System.IO.File]::WriteAllText($stdoutPath, $stdoutTask.Result, [System.Text.Encoding]::UTF8)
[System.IO.File]::WriteAllText($stderrPath, $stderrTask.Result, [System.Text.Encoding]::UTF8)
if($stdoutTask.Result.Length -gt 0) {
    Write-Host $stdoutTask.Result
}
if($stderrTask.Result.Length -gt 0) {
    Write-Error $stderrTask.Result
}
Write-Host "EXIT $($process.ExitCode)"
Write-Host "stdout: $stdoutPath"
Write-Host "stderr: $stderrPath"
exit $process.ExitCode
