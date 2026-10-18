#pragma once
#include <windows.h>
#include <optional>
#include <string>
#include <vector>
namespace qc {
struct Chord { UINT modifiers{}; WORD key{}; };
struct Shortcut { std::wstring title, category; Chord chord; };
struct Profile { std::wstring executable, name; std::vector<Shortcut> shortcuts; };
struct Store { std::vector<Profile> profiles; Chord trigger{MOD_CONTROL | MOD_ALT, VK_SPACE}; };
std::optional<Chord> parseChord(const std::wstring& text);
std::wstring formatChord(Chord chord);
std::wstring appKey(const std::wstring& path);
std::wstring serialize(const Store& store);
bool deserialize(const std::wstring& data, Store& store);
bool validStore(const Store& store);
}
