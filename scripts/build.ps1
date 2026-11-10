param([switch]$Package)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$vsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath = & $vsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Install Visual Studio Desktop development with C++ and Windows SDK.' }
$cmakeExe = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
if (-not (Test-Path -LiteralPath $cmakeExe)) { $cmakeExe = (Get-Command cmake -ErrorAction Stop).Source }
$vsVersion = & $vsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
$generator = switch ($vsVersion.Split('.')[0]) {
    '18' { 'Visual Studio 18 2026' }
    '17' { 'Visual Studio 17 2022' }
    default { throw 'Visual Studio 2022 or 2026 is required.' }
}
& $cmakeExe -S $repoRoot -B (Join-Path $repoRoot 'build') -G $generator -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
& $cmakeExe --build (Join-Path $repoRoot 'build') --config Release
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
if ($Package) {
    $packageDir = Join-Path $repoRoot 'dist/MouseOptions-x64'
    & $cmakeExe --install (Join-Path $repoRoot 'build') --config Release --prefix $packageDir
    if ($LASTEXITCODE -ne 0) { throw 'Packaging failed.' }
    $packageFiles = @('MouseOptions.exe', 'README.md', 'MANUAL-CHECKS.md') | ForEach-Object { Join-Path $packageDir $_ }
    Compress-Archive -LiteralPath $packageFiles -DestinationPath (Join-Path $repoRoot 'dist/MouseOptions-x64.zip') -Force
}
