param([string]$Destination = 'dist/NandStudio-windows-x86_64', [string]$BuildDirectory = 'build',
      [string]$QtRoot = 'C:/Qt/6.11.1/msvc2022_64', [string]$CrtDirectory = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$target = [IO.Path]::GetFullPath((Join-Path $root $Destination))
if (-not $target.StartsWith($root + [IO.Path]::DirectorySeparatorChar)) { throw 'Package destination must be inside workspace' }
$env:PATH = "$QtRoot/bin;$env:PATH"
$build = [IO.Path]::GetFullPath((Join-Path $root $BuildDirectory))
if (-not $build.StartsWith($root + [IO.Path]::DirectorySeparatorChar)) { throw 'Build directory must be inside workspace' }
cmake --install $build --prefix $target --config Release
if ($LASTEXITCODE) { throw 'Deployment failed' }
# App-local release CRT; never copy debug_nonredist files. Default is the pinned
# workstation runtime; CI passes its compiler's redist and records DLL hashes.
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installation = & $vswhere -latest -property installationPath
$crt = if ($CrtDirectory) { $CrtDirectory } else { Join-Path $installation 'VC/Redist/MSVC/14.51.36231/x64/Microsoft.VC145.CRT' }
if (-not (Test-Path -LiteralPath $crt)) { throw 'Pinned MSVC release redistributable not found' }
Copy-Item -Path "$crt/*.dll" -Destination "$target/bin"
$runtimeEvidence = Get-ChildItem -LiteralPath $crt -Filter '*.dll' | ForEach-Object {
    [ordered]@{ file = $_.Name; version = $_.VersionInfo.FileVersion; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
}
New-Item -ItemType Directory -Force "$target/docs" | Out-Null
Copy-Item -LiteralPath "$root/NOTICE.md","$root/README.md","$root/toolchain-lock.json" -Destination $target
Copy-Item -Path "$root/docs/*.md","$root/docs/parity-manifest.json","$root/docs/platforms.json" -Destination "$target/docs"
Copy-Item -LiteralPath "$root/docs/evidence" -Destination "$target/docs" -Recurse -Force
$runtimeEvidence | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath "$target/docs/evidence/windows-crt.json" -Encoding utf8
New-Item -ItemType Directory -Force "$target/licenses" | Out-Null
Copy-Item -Path "$root/licenses/*" -Destination "$target/licenses"
$qtLicenses = Join-Path $QtRoot '../../Licenses'
if (Test-Path -LiteralPath $qtLicenses) { Copy-Item -Path "$qtLicenses/*" -Destination "$target/licenses" }
$zip = "$target.zip"
Compress-Archive -Path "$target/*" -DestinationPath $zip -Force
Get-FileHash -Algorithm SHA256 -LiteralPath $zip
