# 0.6 catalog foundation — implementation evidence

This is local infrastructure, not a published or user-ready 0.6 desktop release.
Desktop version remains 0.5.0-alpha.4. Public release remains alpha.3.

Portal: new module files only under app/apps/quick-cheat in D:/dashboard; no
apps.json/core/compose edits, registration, restart or deployment. Branch
codex/quick-cheat-catalog keeps the pre-existing user edits unchanged.
Node --check passed for contract.js, catalog.js and server.js (host Node24;
Portal target Node22). This is syntax evidence, not a live API/behavior test.

Windows: qc_community static library compiled with Release SkipTests; log
build/community-foundation-build.log; final build: community-foundation-final-build.log.
Initial compile caught a missing Windows
Foundation Collections header for C++/WinRT; added before successful compilation.
No new library code is linked/called by the desktop UI yet; no outbound requests
or user-profile changes were performed by this component during validation.

No automated tests added/run. Pending: real service, native list/preview/install,
network errors/cancellation/cache behavior, hash/tamper failures and provenance
after import. Later phases remain checkboxes in the execution plan, not claims.

Library: [local 0.6 foundation and 0.6–0.9 plan](tgslib://9f13649533b3e4f8).
