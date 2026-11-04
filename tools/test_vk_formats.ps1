# Vulkan format probe (tools/test_vk_formats.c), no game files needed: builds with the project's NDK, runs through adb,
# writes the JSON report to android/out/test_vk_formats.json (or -OutFile). Windows, macOS or Linux, in pwsh.
#   pwsh tools/test_vk_formats.ps1                                                  the system driver
#   pwsh tools/test_vk_formats.ps1 -Driver ~/drivers/libvulkan_freedreno.so         a driver .so from this computer (Turnip: the .so out of its zip)
#   pwsh tools/test_vk_formats.ps1 -Driver /data/local/tmp/libvulkan_freedreno.so   a driver .so already on the device
# Other options: -Sdk <Android SDK>, -DeviceSerial <serial from adb devices>. It takes about a minute. It touches only
# its own files in /data/local/tmp. A group that crashes the driver shows up in the report as "crashed".
param([string]$Sdk, [string]$DeviceSerial, [string]$Driver, [string]$OutFile)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
# (Windows PowerShell 5.1 has none of the $Is variables: it is the last branch)
if ($IsMacOS) {
    $ndkHost = 'darwin-x86_64'; $exe = ''; $defaultSdk = Join-Path $HOME 'Library/Android/sdk'
} elseif ($IsLinux) {
    $ndkHost = 'linux-x86_64'; $exe = ''; $defaultSdk = Join-Path $HOME 'Android/Sdk'
} else {
    $ndkHost = 'windows-x86_64'; $exe = '.exe'; $defaultSdk = Join-Path $env:LOCALAPPDATA 'Android/Sdk'
}
if (-not $Sdk) { $Sdk = $env:ANDROID_HOME }
if (-not $Sdk) {
    $sdkProperties = Join-Path $repoRoot 'android/botwrecompandroid/local.properties'
    if (Test-Path -LiteralPath $sdkProperties) {
        $sdkLine = Get-Content -LiteralPath $sdkProperties | Where-Object { $_ -match '^sdk\.dir=' } | Select-Object -First 1
        if ($sdkLine) { $Sdk = $sdkLine.Substring(8).Replace('\:', ':').Replace('\\', '\') }
    }
}
if (-not $Sdk) { $Sdk = $defaultSdk }
$ndk = Join-Path $Sdk 'ndk/30.0.16248370'
$compilerPath = Join-Path $ndk "toolchains/llvm/prebuilt/$ndkHost/bin/clang$exe"
$glslcPath = Join-Path $ndk "shader-tools/$ndkHost/glslc$exe"
$adbPath = Join-Path $Sdk "platform-tools/adb$exe"
foreach ($tool in @($compilerPath, $glslcPath, $adbPath)) {
    if (-not (Test-Path -LiteralPath $tool)) { throw "Not found: $tool (NDK 30.0.16248370 and platform-tools in the SDK; -Sdk or ANDROID_HOME says where it is)" }
}
$deviceArgs = @()
if ($DeviceSerial) { $deviceArgs = @('-s', $DeviceSerial) }
$deviceDir = '/data/local/tmp/botwrecompandroid-tests'
$shaders = @{
    vs_full = 'vert'; vs_tex = 'vert'; vs_cmp = 'vert'; vs_point = 'vert'; vs_point1 = 'vert'; vs_atomic = 'vert'
    fs_tex = 'frag'; fs_cmp = 'frag'; fs_pass = 'frag'; fs_const = 'frag'; cs_cmp = 'comp'
}
if (-not $OutFile) { $OutFile = Join-Path $repoRoot 'android/out/test_vk_formats.json' }
Push-Location $repoRoot
try {
    $buildDir = 'android/out/test_vk_formats'
    New-Item -ItemType Directory -Force $buildDir | Out-Null
    foreach ($shader in $shaders.Keys) {
        & $glslcPath -x glsl "-fshader-stage=$($shaders[$shader])" "-D$($shader.ToUpper())" -mfmt=c tools/test_vk_formats.glsl -o "$buildDir/$shader.inc"
        if ($LASTEXITCODE -ne 0) { throw "Shader compilation failed: $shader" }
    }
    & $compilerPath --target=aarch64-linux-android30 -O1 -g -Wno-format "-I$buildDir" tools/test_vk_formats.c -o "$buildDir/test_vk_formats" -ldl -lm
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed: test_vk_formats' }
    & $adbPath @deviceArgs shell mkdir -p $deviceDir
    if ($LASTEXITCODE -ne 0) { throw 'Could not create the device test directory.' }
    & $adbPath @deviceArgs push "$buildDir/test_vk_formats" "$deviceDir/test_vk_formats"
    if ($LASTEXITCODE -ne 0) { throw 'Upload failed: test_vk_formats' }
    & $adbPath @deviceArgs shell chmod 755 "$deviceDir/test_vk_formats"
    if ($LASTEXITCODE -ne 0) { throw 'Could not make executable: test_vk_formats' }
    $driverArgs = ''
    $pushedDriver = $null
    if ($Driver) {
        $devicePath = $Driver
        if (Test-Path -LiteralPath $Driver) {
            $devicePath = "$deviceDir/$(Split-Path $Driver -Leaf)"
            & $adbPath @deviceArgs push $Driver $devicePath
            if ($LASTEXITCODE -ne 0) { throw "Upload failed: $Driver" }
            $pushedDriver = $devicePath
        }
        $driverArgs = "--driver '$devicePath'"
    }
    $deviceOut = "$deviceDir/test_vk_formats.json"
    try {
        & $adbPath @deviceArgs shell "$deviceDir/test_vk_formats $driverArgs --out $deviceOut"
        if ($LASTEXITCODE -ne 0) { throw 'The probe could not write its report.' }
        & $adbPath @deviceArgs pull $deviceOut $OutFile
        if ($LASTEXITCODE -ne 0) { throw 'Could not fetch the report.' }
    } finally {
        & $adbPath @deviceArgs shell rm -f $deviceOut
        if ($pushedDriver) { & $adbPath @deviceArgs shell rm -f $pushedDriver }
    }
    Write-Host "Report: $OutFile"
} finally {
    Pop-Location
}
