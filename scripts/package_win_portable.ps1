param(
    [string]$BuildDir = "build",
    [string]$Configuration = "Release",
    [string]$OutputRoot = "dist",
    [string]$PackageName = "RotaryTableControl-win11-portable"
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Get-CMakeCacheValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$CachePath,
        [Parameter(Mandatory = $true)]
        [string]$Key
    )

    if (-not (Test-Path -LiteralPath $CachePath)) {
        return $null
    }

    $match = Select-String -Path $CachePath -Pattern "^${Key}:[^=]+=(.*)$" | Select-Object -First 1
    if ($null -eq $match) {
        return $null
    }

    return $match.Matches[0].Groups[1].Value.Trim()
}

function Copy-IfExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,
        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if (Test-Path -LiteralPath $Source) {
        Copy-Item -LiteralPath $Source -Destination $Destination -Force
        return $true
    }

    return $false
}

function Copy-MatchingFiles {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Roots,
        [Parameter(Mandatory = $true)]
        [string]$Filter,
        [Parameter(Mandatory = $true)]
        [string]$Destination,
        [switch]$ExcludeDebug
    )

    $copied = @{}
    foreach ($root in $Roots) {
        if (-not (Test-Path -LiteralPath $root)) {
            continue
        }

        Get-ChildItem -LiteralPath $root -Filter $Filter -File | ForEach-Object {
            if ($ExcludeDebug -and $_.BaseName.EndsWith('d')) {
                return
            }

            if (-not $copied.ContainsKey($_.Name)) {
                Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $Destination $_.Name) -Force
                $copied[$_.Name] = $true
            }
        }
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildPath = Join-Path $repoRoot $BuildDir
$cachePath = Join-Path $buildPath "CMakeCache.txt"
$packageRoot = Join-Path (Join-Path $repoRoot $OutputRoot) $PackageName
$appBuildDir = Join-Path $buildPath $Configuration
$appExe = Join-Path $appBuildDir "RotaryTableControl.exe"

Write-Host "==> Building RotaryTableControl ($Configuration)"
& cmake --build $buildPath --config $Configuration --target RotaryTableControl
if ($LASTEXITCODE -ne 0) {
    throw "cmake build failed."
}

if (-not (Test-Path -LiteralPath $appExe)) {
    throw "Expected executable not found: $appExe"
}

$qtCMakeDir = Get-CMakeCacheValue -CachePath $cachePath -Key "Qt5_DIR"
if ([string]::IsNullOrWhiteSpace($qtCMakeDir)) {
    $qtCMakeDir = Get-CMakeCacheValue -CachePath $cachePath -Key "Qt6_DIR"
}

$windeployqt = $null
if (-not [string]::IsNullOrWhiteSpace($qtCMakeDir)) {
    $qtBinDir = Join-Path $qtCMakeDir "..\..\..\bin"
    $candidate = Join-Path $qtBinDir "windeployqt.exe"
    if (Test-Path -LiteralPath $candidate) {
        $windeployqt = (Resolve-Path $candidate).Path
    }
}

if ([string]::IsNullOrWhiteSpace($windeployqt)) {
    $command = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        $windeployqt = $command.Source
    }
}

if ([string]::IsNullOrWhiteSpace($windeployqt)) {
    throw "windeployqt.exe not found. Please make sure the Qt deployment tool is installed."
}

Write-Host "==> Preparing portable package: $packageRoot"
if (Test-Path -LiteralPath $packageRoot) {
    Remove-Item -LiteralPath $packageRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $packageRoot | Out-Null

Copy-Item -LiteralPath $appExe -Destination (Join-Path $packageRoot "RotaryTableControl.exe") -Force

$configSource = Join-Path $repoRoot "config"
if (Test-Path -LiteralPath $configSource) {
    Copy-Item -LiteralPath $configSource -Destination (Join-Path $packageRoot "config") -Recurse -Force
}

foreach ($dirName in @("logs", "captures", "RunTimeLog", "WatchData")) {
    New-Item -ItemType Directory -Path (Join-Path $packageRoot $dirName) -Force | Out-Null
}

$watchDataFile = Join-Path $packageRoot "WatchData\WatchData.txt"
if (-not (Test-Path -LiteralPath $watchDataFile)) {
    New-Item -ItemType File -Path $watchDataFile -Force | Out-Null
}

$boardDllCandidates = @(
    (Join-Path $appBuildDir "GAS.dll"),
    (Join-Path $repoRoot "third_party\board\x64\GAS.dll")
)

$boardDllCopied = $false
foreach ($candidate in $boardDllCandidates) {
    if (Copy-IfExists -Source $candidate -Destination (Join-Path $packageRoot "GAS.dll")) {
        $boardDllCopied = $true
        break
    }
}

if (-not $boardDllCopied) {
    Write-Warning "GAS.dll not found. Board functions may be unavailable in the portable package."
}

Copy-MatchingFiles -Roots @(
    $appBuildDir,
    (Join-Path $repoRoot "third_party\opencv\build\x64\vc16\bin"),
    (Join-Path $repoRoot "third_party\opencv\build\bin")
) -Filter "opencv*.dll" -Destination $packageRoot -ExcludeDebug

$vcRuntimeNames = @(
    "vcruntime140.dll",
    "vcruntime140_1.dll",
    "msvcp140.dll",
    "msvcp140_1.dll",
    "msvcp140_2.dll",
    "msvcp140_atomic_wait.dll",
    "concrt140.dll"
)

$runtimeSearchRoots = @(
    "$env:WINDIR\System32",
    "$env:WINDIR\SysWOW64"
)

foreach ($dllName in $vcRuntimeNames) {
    foreach ($root in $runtimeSearchRoots) {
        $candidate = Join-Path $root $dllName
        if (Copy-IfExists -Source $candidate -Destination (Join-Path $packageRoot $dllName)) {
            break
        }
    }
}

Write-Host "==> Deploying Qt runtime with $windeployqt"
& $windeployqt --release --force --no-compiler-runtime (Join-Path $packageRoot "RotaryTableControl.exe")
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed."
}

Write-Host ""
Write-Host "Portable package ready:"
Write-Host "  $packageRoot"
