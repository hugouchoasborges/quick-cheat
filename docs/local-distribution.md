# Local candidate distribution

Requires Git, Windows PowerShell 5.1+, Visual Studio 2022 Desktop development
with C++, Windows SDK and CMake 3.24+. Only x64 Release is packaged; MSVC runtime
is statically linked. Exact minimum Windows/browser versions remain unconfirmed.

From a clean, committed checkout, run `./scripts/package.ps1` in PowerShell.
It builds with SkipTests in `build/local-distribution`, leaving the development
executable and any running instance intact. Output: `build/packages/`, named
with the executable version and source commit. Existing packages are refused.
Failed staging directories are retained; inspect them before removing manually.
No app is launched, stopped, installed, signed, uploaded or published.

The ZIP contains only the executable, explicit documentation/license allowlist,
build metadata and SHA256SUMS.txt. No user data, drafts, attachments, preview
helper or tests are packaged. The external `.zip.sha256` hashes the ZIP itself;
`Get-FileHash -Algorithm SHA256 <zip>` must agree before extracting.
The internal manifest hashes every payload file except the manifest itself.
Hashes detect differences; these unsigned artifacts do not prove publisher identity.
Metadata records commit, candidate version, Release/x64 and CMake toolchain paths.
Paths are local build-machine paths; inspect before sharing. Rebuilding is
repeatable, but timestamps/toolchain differences mean byte-identical ZIPs are
not promised. Toolchain pinning and a public pipeline remain future work.

Extract into a new directory and inspect metadata before use. Exit Quick Cheat
normally via its tray action, resolving pending-edit prompts. Never force exit
or overwrite a running executable. The application uses its existing user-data
location; a ZIP is not a portable-data mode or an installer. For isolated manual
validation use `QuickCheat.exe --data-dir <new-empty-directory>` after exiting
the real instance. Keep the original directory and shortcuts unchanged.

Before trying a candidate, back up the complete data folder while the app is
closed, including shortcuts.qc, backups, attachments and ui-state.ini. See README
for the existing location. Do not delete corrupt stores or missing attachments.
Restore into a separate validation directory first; preserve the damaged copy.
For rollback to pre-v4, restore shortcuts.qc.pre-v4.bak as shortcuts.qc along
with matching attachments. Later v4 changes are absent from that backup.
A restoration UI, automatic updates, signing and installer remain undecided.

Before any release: complete cumulative 0.4 and current candidate acceptance,
confirm metadata/hashes/licenses, try extraction on a second x64 Windows machine,
and obtain explicit acceptance and publication request. A successful build is
not runtime compatibility evidence. Main and public 0.3 remain unchanged.
