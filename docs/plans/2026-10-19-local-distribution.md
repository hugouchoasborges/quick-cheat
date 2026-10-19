# Local distribution implementation plan

Goal: deliver 0.5.0-alpha.1 locally without accepting or publishing 0.4.
Architecture: retain C++20/Win32 runtime and data formats. Isolate build output;
package only an explicit allowlist, with source identity and SHA-256 hashes.
Tech stack: existing VS 2022 x64/CMake and Windows PowerShell.

Alternatives: runtime/UIA changes need concrete reproduction; installer/signing
need separate decisions. Choose local ZIP tooling first, within the authorized
robustness/distribution roadmap. This does not close the whole 0.5 increment.

1. Add an optional build directory; preserve the current default and SkipTests.
2. Add local Release packaging, require clean tracked source, reject overwrites,
   include licenses/docs, record commit, toolchain and executable metadata.
3. Document observed environment versus pending support, recovery and a manual
   performance measurement protocol. Do not infer minimum Windows/browser versions.
4. Bump candidate metadata and document changes/acceptance in README/changelog.
5. Build with SkipTests in build/local-0.5; inspect the package allowlist, metadata,
   hashes and PE x64 identity. No automated tests added or executed.
6. Create a version child page in Library and a Harness memory; commit locally on
   feature branch. Keep main and the unaccepted 0.4 release branch unchanged.

Acceptance: Hugo validates the ZIP on another machine and runs the cumulative
0.4 checklists. No publication, signature, installer or bit-identical ZIP claim.
