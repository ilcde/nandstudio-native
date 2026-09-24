param([switch]$CoreOnly, [string]$Configuration = 'Release', [string]$BuildDirectory = 'build')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = [IO.Path]::GetFullPath((Join-Path $root $BuildDirectory))
if (-not $build.StartsWith($root + [IO.Path]::DirectorySeparatorChar)) { throw 'Build directory must be inside workspace' }
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installation = & $vswhere -latest -property installationPath
Import-Module (Join-Path $installation 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $installation -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'
$env:PATH = "C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;C:\Qt\6.11.1\msvc2022_64\bin;$env:PATH"
$gui = if ($CoreOnly) { 'OFF' } else { 'ON' }
cmake -S $root -B $build -G Ninja "-DCMAKE_BUILD_TYPE=$Configuration" "-DNAND_BUILD_GUI=$gui" '-DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64'
if ($LASTEXITCODE) { throw 'CMake configure failed' }
cmake --build $build
if ($LASTEXITCODE) { throw 'Build failed' }
ctest --test-dir $build --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
