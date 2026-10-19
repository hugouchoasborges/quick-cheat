param([ValidateSet('Debug','Release')][string]$Configuration='Release',[switch]$SkipTests,[string]$BuildDirectory)
$ErrorActionPreference='Stop'
$repoRoot=Split-Path $PSScriptRoot -Parent
if(!$BuildDirectory){$BuildDirectory=Join-Path $repoRoot 'build'}
$BuildDirectory=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($BuildDirectory)
$cmakeCommand=Get-Command cmake -ErrorAction SilentlyContinue
if($cmakeCommand){$cmakeExe=$cmakeCommand.Source}else{
 $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
 if(!(Test-Path -LiteralPath $vswhere)){throw 'Install Visual Studio 2022 with Desktop development with C++, Windows SDK and CMake tools.'}
 $vsRoot=& $vswhere -latest -version '[17.0,18.0)' -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 Microsoft.VisualStudio.Component.VC.CMake.Project -property installationPath
 if(!$vsRoot){throw 'Install Visual Studio 2022 Desktop development with C++ including CMake tools.'}
 $cmakeExe=Join-Path $vsRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
}
& $cmakeExe -S $repoRoot -B $BuildDirectory -G 'Visual Studio 17 2022' -A x64 -DQC_BUILD_APP=ON
if($LASTEXITCODE){throw 'CMake configure failed'}
& $cmakeExe --build $BuildDirectory --config $Configuration
if($LASTEXITCODE){throw 'Build failed'}
if(!$SkipTests){
 $ctestExe=Join-Path (Split-Path $cmakeExe) 'ctest.exe'
 & $ctestExe --test-dir $BuildDirectory -C $Configuration --output-on-failure
 if($LASTEXITCODE){throw 'Tests failed'}
}
Write-Output "Executable: $BuildDirectory/$Configuration/QuickCheat.exe"
