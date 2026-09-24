param([string]$Destination = 'dist/NandStudio-windows-x86_64', [string]$BuildDirectory = 'build')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$target = [IO.Path]::GetFullPath((Join-Path $root $Destination))
if (-not $target.StartsWith($root + [IO.Path]::DirectorySeparatorChar)) { throw 'Package destination must be inside workspace' }
$env:PATH = "C:\Qt\Tools\CMake_64\bin;C:\Qt\6.11.1\msvc2022_64\bin;$env:PATH"
$build = [IO.Path]::GetFullPath((Join-Path $root $BuildDirectory))
if (-not $build.StartsWith($root + [IO.Path]::DirectorySeparatorChar)) { throw 'Build directory must be inside workspace' }
cmake --install $build --prefix $target --config Release
if ($LASTEXITCODE) { throw 'Deployment failed' }
# App-local release CRT; never copy debug_nonredist files. Version is pinned to
# the compiler recorded in toolchain-lock.json for this workstation package.
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installation = & $vswhere -latest -property installationPath
$crt = Join-Path $installation 'VC/Redist/MSVC/14.51.36231/x64/Microsoft.VC145.CRT'
if (-not (Test-Path -LiteralPath $crt)) { throw 'Pinned MSVC release redistributable not found' }
Copy-Item -Path "$crt/*.dll" -Destination "$target/bin"
$runtimeEvidence = Get-ChildItem -LiteralPath $crt -Filter '*.dll' | ForEach-Object {
    [ordered]@{ file = $_.Name; version = $_.VersionInfo.FileVersion; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
}
$runtimeEvidence | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath "$root/docs/evidence/windows-crt.json" -Encoding utf8
New-Item -ItemType Directory -Force "$target/docs" | Out-Null
Copy-Item -LiteralPath "$root/NOTICE.md","$root/README.md","$root/toolchain-lock.json" -Destination $target
Copy-Item -Path "$root/docs/*.md","$root/docs/parity-manifest.json","$root/docs/platforms.json" -Destination "$target/docs"
Copy-Item -LiteralPath "$root/docs/evidence" -Destination "$target/docs" -Recurse -Force
New-Item -ItemType Directory -Force "$target/licenses" | Out-Null
Copy-Item -Path "$root/licenses/*" -Destination "$target/licenses"
Copy-Item -Path 'C:/Qt/Licenses/*' -Destination "$target/licenses"
$zip = "$target.zip"
Compress-Archive -Path "$target/*" -DestinationPath $zip -Force
Get-FileHash -Algorithm SHA256 -LiteralPath $zip
