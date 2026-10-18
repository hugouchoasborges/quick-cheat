param([ValidateSet('Debug','Release')][string]$Configuration='Release')
$ErrorActionPreference='Stop'
$repoRoot=Split-Path $PSScriptRoot -Parent
$target=[IO.Path]::GetFullPath((Join-Path $repoRoot "build/$Configuration/QuickCheat.exe"))
if(!(Test-Path -LiteralPath $target -PathType Leaf)){throw 'Build Quick Cheat before creating its desktop shortcut.'}
$desktop=[Environment]::GetFolderPath('DesktopDirectory')
if(!$desktop){throw 'Windows did not return a desktop directory.'}
$shell=New-Object -ComObject WScript.Shell
$shortcutPath=Join-Path $desktop 'Quick Cheat.lnk'
if(Test-Path -LiteralPath $shortcutPath){$existing=$shell.CreateShortcut($shortcutPath);if($existing.TargetPath-ne $target){$shortcutPath=Join-Path $desktop 'Quick Cheat (Development).lnk';if(Test-Path -LiteralPath $shortcutPath){$alternate=$shell.CreateShortcut($shortcutPath);if($alternate.TargetPath-ne $target){throw 'Existing desktop shortcuts point elsewhere; choose a different shortcut name.'}}}}
$shortcut=$shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath=$target;$shortcut.WorkingDirectory=Split-Path $target -Parent;$shortcut.IconLocation="$target,0";$shortcut.Description='Quick Cheat — development build';$shortcut.Save()
$verified=$shell.CreateShortcut($shortcutPath)
if($verified.TargetPath-ne $target){throw 'Shortcut target verification failed.'}
[pscustomobject]@{Shortcut=$shortcutPath;Target=$verified.TargetPath;Icon=$verified.IconLocation}|ConvertTo-Json -Compress
