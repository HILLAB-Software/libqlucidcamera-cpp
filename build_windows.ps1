param(
    [ValidateSet('Debug', 'Release', 'Both')]
    [string]$Configuration = 'Both',

    [int]$Jobs = 8,

    [string]$ConfigurePreset,

    [string]$BuildPreset
)

$ErrorActionPreference = 'Stop'

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

$arenaCandidates = @(
    'C:\Program Files\Lucid Vision Labs\Arena SDK',
    'C:\Program Files\Lucid Vision Labs\ArenaSDK'
)

if (-not $env:ARENA_SDK_DIR -or -not (Test-Path $env:ARENA_SDK_DIR)) {
    foreach ($candidate in $arenaCandidates) {
        if (Test-Path $candidate) {
            $env:ARENA_SDK_DIR = $candidate
            break
        }
    }
}

if ($env:ARENA_SDK_DIR -and (Test-Path $env:ARENA_SDK_DIR)) {
    # Keep all common env variable names in sync for CMake fallback discovery.
    $env:LUCID_DEV_ROOT = $env:ARENA_SDK_DIR
    $env:ARENA_SDK_ROOT = $env:ARENA_SDK_DIR
}

# LIBS_ROOT: install prefixes for locally built dependencies (semicolon-separated)
if (-not $env:LIBS_ROOT) {
    $env:LIBS_ROOT = 'C:\libs\beamprofiler;C:\libs\qlucidcamera'
}

if (-not $env:ARENA_SDK_DIR -or -not (Test-Path $env:ARENA_SDK_DIR)) {
    Write-Error "ARENA_SDK_DIR is invalid: $($env:ARENA_SDK_DIR)`nSet a valid path: `$env:ARENA_SDK_DIR = 'C:\path\to\Arena SDK'"
    exit 1
}

if (-not $env:OpenCV_DIR) {
    $opencvCandidates = @(
        'C:\opencv\build',
        'C:\tools\opencv\build'
    )
    foreach ($candidate in $opencvCandidates) {
        if (Test-Path $candidate) {
            $env:OpenCV_DIR = $candidate
            break
        }
    }
}

# Detect latest installed Visual Studio and set CMAKE_GENERATOR.
function Get-LatestVSGenerator {
    $vswhere = Join-Path ([System.Environment]::GetEnvironmentVariable('ProgramFiles(x86)')) 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) { return $null }

    $json = & $vswhere -all -latest -format json 2>$null
    if (-not $json) { return $null }

    $inst = ($json | ConvertFrom-Json) | Select-Object -First 1
    if (-not $inst) { return $null }

    $major = [int]($inst.installationVersion -split '\.')[0]
    $year = $inst.catalog.productLineVersion
    return "Visual Studio $major $year"
}

$vsGenerator = Get-LatestVSGenerator
if ($vsGenerator) {
    Write-Host "Detected Visual Studio generator: $vsGenerator"
    $env:CMAKE_GENERATOR = $vsGenerator
}
else {
    Write-Warning 'vswhere.exe not found; falling back to CMake auto-detection'
}

if (-not $ConfigurePreset) {
    if ($Configuration -ne 'Both') {
        $configLower = $Configuration.ToLowerInvariant()
        $ConfigurePreset = "win-qt-env-$configLower"
    }
}

if (-not $BuildPreset) {
    if ($Configuration -ne 'Both') {
        $configLower = $Configuration.ToLowerInvariant()
        $BuildPreset = "build-win-qt-env-$configLower"
    }
}

# If local user presets exist and define local windows presets, prefer them.
$userPresetPath = Join-Path $ScriptDir 'CMakeUserPresets.json'
if ((Test-Path $userPresetPath) -and -not $PSBoundParameters.ContainsKey('ConfigurePreset') -and -not $PSBoundParameters.ContainsKey('BuildPreset')) {
    $configLower = $Configuration.ToLowerInvariant()
    $localConfigurePreset = "win-local-$configLower"
    $localBuildPreset = "build-win-local-$configLower"

    try {
        $userPresets = Get-Content $userPresetPath -Raw | ConvertFrom-Json
        $configureNames = @()
        $buildNames = @()

        if ($userPresets.configurePresets) {
            $configureNames = $userPresets.configurePresets | ForEach-Object { $_.name }
        }
        if ($userPresets.buildPresets) {
            $buildNames = $userPresets.buildPresets | ForEach-Object { $_.name }
        }

        if (($configureNames -contains $localConfigurePreset) -and ($buildNames -contains $localBuildPreset)) {
            $ConfigurePreset = $localConfigurePreset
            $BuildPreset = $localBuildPreset
            Write-Host "Using local user presets: $ConfigurePreset / $BuildPreset"
        }
    }
    catch {
        Write-Warning "Could not parse CMakeUserPresets.json: $($_.Exception.Message)"
    }
}

Write-Host "ARENA_SDK_DIR = $env:ARENA_SDK_DIR"
if ($env:OpenCV_DIR) {
    Write-Host "OpenCV_DIR = $env:OpenCV_DIR"
}
if ($Configuration -eq 'Both' -and -not $PSBoundParameters.ContainsKey('ConfigurePreset') -and -not $PSBoundParameters.ContainsKey('BuildPreset')) {
    Write-Host 'Configuration mode = Both (Debug + Release)'
}
else {
    Write-Host "Configure preset = $ConfigurePreset"
    Write-Host "Build preset = $BuildPreset"
}

Push-Location $ScriptDir
try {
    if ($Configuration -eq 'Both' -and -not $PSBoundParameters.ContainsKey('ConfigurePreset') -and -not $PSBoundParameters.ContainsKey('BuildPreset')) {
        $presetPairs = @(
            @{ Configure = 'win-qt-env-debug'; Build = 'build-win-qt-env-debug' },
            @{ Configure = 'win-qt-env-release'; Build = 'build-win-qt-env-release' }
        )

        foreach ($pair in $presetPairs) {
            Write-Host "Configure preset = $($pair.Configure)"
            cmake --preset $pair.Configure

            Write-Host "Build preset = $($pair.Build)"
            cmake --build --preset $pair.Build -j $Jobs
        }
    }
    else {
        cmake --preset $ConfigurePreset
        cmake --build --preset $BuildPreset -j $Jobs
    }
}
finally {
    Pop-Location
}
