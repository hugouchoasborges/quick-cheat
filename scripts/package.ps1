param()
$ErrorActionPreference='Stop'
$repoRoot=Split-Path $PSScriptRoot -Parent
$status=& git -C $repoRoot status --porcelain
if($LASTEXITCODE -ne 0 -or $status){throw 'Packaging requires a clean Git working tree. Commit local changes first.'}
$commit=& git -C $repoRoot rev-parse HEAD
if($LASTEXITCODE -ne 0){throw 'Cannot identify source commit'}
$buildDirectory=Join-Path $repoRoot 'build'
& "$PSScriptRoot/build.ps1" -SkipTests -BuildDirectory $buildDirectory
$exe=Join-Path $buildDirectory 'Release/QuickCheat.exe'
$version=(Get-Item -LiteralPath $exe).VersionInfo.ProductVersion
if($version -notmatch '^\d+\.\d+\.\d+-alpha\.\d+$'){throw 'Expected an alpha candidate version resource'}
$name="QuickCheat-$version-windows-x64-$($commit.Substring(0,12))"
$output=Join-Path $repoRoot 'build/packages'
$stage=Join-Path $output $name
$zip="$stage.zip"
if((Test-Path -LiteralPath $stage) -or (Test-Path -LiteralPath $zip)){throw 'Package already exists; keep it or choose a new source commit.'}
New-Item -ItemType Directory -Path $stage | Out-Null
$files=@('LICENSE','THIRD_PARTY_NOTICES.md','README.md','CHANGELOG.md','docs/local-distribution.md','docs/support-matrix.md','docs/validation-0.5.0-alpha.1.md','docs/validation-0.5.0-alpha.2.md','docs/browser-integration.md','docs/collection-format.md','docs/validation-0.4.0-alpha.1.md','docs/validation-0.4.0-alpha.2.md')
Copy-Item -LiteralPath $exe -Destination (Join-Path $stage 'QuickCheat.exe')
$files+="docs/releases/$version.md"
if($files -notcontains "docs/validation-$version.md"){$files+="docs/validation-$version.md"}
$files+=@(Get-ChildItem -LiteralPath (Join-Path $repoRoot "docs/images/v$version") -Filter '*.png' -File | ForEach-Object {"docs/images/v$version/$($_.Name)"})
foreach($file in $files){
 $target=Join-Path $stage $file
 New-Item -ItemType Directory -Path (Split-Path $target) -Force | Out-Null
 Copy-Item -LiteralPath (Join-Path $repoRoot $file) -Destination $target
}
$toolchain=@(Select-String -LiteralPath (Join-Path $buildDirectory 'CMakeCache.txt') -Pattern '^(CMAKE_COMMAND|CMAKE_GENERATOR|CMAKE_GENERATOR_PLATFORM|CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION|CMAKE_VS_PLATFORM_TOOLSET|CMAKE_CXX_COMPILER):' | ForEach-Object {$_.Line})
$compilerFiles=Get-ChildItem -Path "$buildDirectory/CMakeFiles/*/CMakeCXXCompiler.cmake"
$toolchain+=@(Select-String -LiteralPath $compilerFiles.FullName -Pattern '^set\(CMAKE_CXX_COMPILER(_ID|_VERSION)? ' | ForEach-Object {$_.Line})
$metadata=[ordered]@{version=$version;sourceCommit=$commit;architecture='x64';configuration='Release';createdUtc=[DateTime]::UtcNow.ToString('o');tests='not run (SkipTests)';acceptance='see versioned release notes; broader support matrix remains unverified';toolchain=$toolchain}
$metadata | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath (Join-Path $stage 'build-metadata.json') -Encoding UTF8
$lines=Get-ChildItem -LiteralPath $stage -Recurse -File | Sort-Object FullName | ForEach-Object {
 $relative=$_.FullName.Substring($stage.Length+1).Replace('\','/')
 '{0}  {1}' -f (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant(),$relative
}
$lines | Set-Content -LiteralPath (Join-Path $stage 'SHA256SUMS.txt') -Encoding ASCII
$status=& git -C $repoRoot status --porcelain
if($LASTEXITCODE -ne 0){throw 'Cannot verify final source status'}
$current=& git -C $repoRoot rev-parse HEAD
if($LASTEXITCODE -ne 0 -or $status -or $current -ne $commit){throw 'Source changed while packaging; staging directory retained for inspection.'}
Compress-Archive -Path "$stage/*" -DestinationPath $zip -CompressionLevel Optimal
'{0}  {1}' -f (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant(),(Split-Path $zip -Leaf) | Set-Content -LiteralPath "$zip.sha256" -Encoding ASCII
Write-Output "Local candidate: $zip"
