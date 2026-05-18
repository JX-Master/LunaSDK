param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptDir "..\..")

$CliOutput = Join-Path $ScriptDir "src\CPPSL.Cli\bin\$Configuration\net9.0"
$NativeExtractor = Join-Path $RepoRoot "build\LunaBuild\Windows\x64\$Configuration\bin\cppsl-native-extractor.exe"
if (-not (Test-Path $NativeExtractor -PathType Leaf)) {
    $NativeExtractor = Join-Path $ScriptDir "native\bin\cppsl-native-extractor.exe"
}
$SdkBin = Join-Path $RepoRoot "SDKs\CPPSL\windows\x64\bin"

if (-not (Test-Path $CliOutput -PathType Container)) {
    Write-Error "CPPSL CLI output does not exist: $CliOutput`nBuild it first, for example: dotnet build Tools\CPPSL\src\CPPSL.Cli\CPPSL.Cli.csproj -c $Configuration -m:1 /nr:false"
}

$Cppslc = Join-Path $CliOutput "cppslc.exe"
if (-not (Test-Path $Cppslc -PathType Leaf)) {
    Write-Error "cppslc does not exist: $Cppslc"
}

if (-not (Test-Path $NativeExtractor -PathType Leaf)) {
    Write-Error "CPPSL native extractor does not exist: $NativeExtractor`nBuild it first, for example: dotnet run --project Tools\LunaBuild\src\LunaBuild.Cli -- build --target cppsl-native-extractor --platform Windows --arch x64"
}

New-Item -ItemType Directory -Force -Path $SdkBin | Out-Null
Copy-Item -Path (Join-Path $CliOutput "*") -Destination $SdkBin -Recurse -Force
Copy-Item -Path $NativeExtractor -Destination (Join-Path $SdkBin "cppsl-native-extractor.exe") -Force

Write-Host "CPPSL Windows x64 artifacts copied to: $SdkBin"
