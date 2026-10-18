#pragma once
#include <windows.h>
#include <optional>
#include <string>
#include <vector>
namespace qc {
inline constexpr size_t maxStoreBytes=8*1024*1024;
struct Chord { UINT modifiers{}; WORD key{}; };
struct Shortcut { std::wstring title, category; Chord chord; };
struct Workflow { std::wstring title; std::vector<std::wstring> steps; };
struct DockConfig { bool enabled{},vertical{},collapsed{}; UINT scale{100},opacity{100}; int x{-1},y{-1}; std::vector<size_t> shortcuts; };
struct Profile { std::wstring executable, name; std::vector<Shortcut> shortcuts; std::vector<Workflow> workflows; std::wstring website; DockConfig dock; };
enum class ThemeMode : UINT { System, Light, Dark };
struct Store { std::vector<Profile> profiles; Chord trigger{MOD_CONTROL | MOD_ALT, VK_SPACE}; ThemeMode theme{ThemeMode::System}; };
std::wstring normalizeWebsite(const std::wstring& input);
bool isBrowserExecutable(const std::wstring& executable);
std::wstring profileKey(const Profile& profile);
bool sameProfile(const Profile& first,const Profile& second);
Profile* findProfile(Store& store,const std::wstring& key);
const Profile* findProfile(const Store& store,const std::wstring& key);
void removeDockShortcut(Profile& profile,size_t erasedIndex);
bool matchesShortcut(const Shortcut& shortcut,const std::wstring& query);
std::optional<Chord> parseChord(const std::wstring& text);
std::wstring formatChord(Chord chord);
std::wstring appKey(const std::wstring& path);
std::wstring serialize(const Store& store);
bool deserialize(const std::wstring& data, Store& store);
bool validStore(const Store& store);
}
