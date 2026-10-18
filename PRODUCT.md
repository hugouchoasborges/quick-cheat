# Quick Cheat

## Platform
Windows desktop, C++20 + Win32 + native controls; x64 builds. Stability, responsiveness and low idle memory/CPU are priorities. English UI. MIT.

## Purpose
Help users remember and deliberately execute shortcuts for the foreground application. Users create and organize their own local shortcuts; profiles follow the external application even while Quick Cheat has keyboard focus.

## Confirmed scope
Popup, local profile/shortcut editor and settings. Hugo selected the supplied Raycast screenshot as visual reference for both popup and management UI, and explicitly requested Dark, Light and System themes. A single click selects a shortcut row; double-click, Execute or Enter executes it. Existing global toggle remains configurable. Local reference workflows are approved for 0.2: titled, ordered text steps with native expand/collapse and editing. Dock and community are deferred.

## Interaction
The popup is a compact command palette: search, categorized rows, keycaps, selected state, explicit Execute and management access. Search intentionally takes keyboard focus while preserving the external target identity. Closing restores that target when appropriate. Selection alone must never inject input.

## Delivery
Gitflow with main reserved for published releases. Each implemented version receives a Library child page with decisions, changes, validation and limitations. Product source: Apps > Quick Cheat (`tgslib://51be8b49abe30ba3`).
