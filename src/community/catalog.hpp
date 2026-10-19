#pragma once
#include "collections.hpp"
#include <atomic>
#include <filesystem>
#include <string>
#include <vector>
namespace qc::community {
inline constexpr wchar_t Origin[]=L"https://portal.hugoborges.dev.br/apps/quick-cheat/api/v1";
struct Query {std::wstring executable,hostname,search,language,keyboardLayout,cursor;};
struct Entry {
 std::wstring id,version,title,summary,authorId,authorName,executable,hostname;
 std::wstring minimumClientVersion,language,keyboardLayout,sha256,license,updatedAt;
 unsigned collectionFormat{},shortcuts{},workflows{},images{},votes{};
};
struct Page {std::vector<Entry> items;std::wstring revision,nextCursor,warning;unsigned total{};bool fromCache{};};
struct PreparedCollection {Entry source;Collection collection;std::vector<std::uint8_t> baseline;};
// Call from an explicit user-action worker, never the UI or foreground hook.
bool list(const Query&,const std::filesystem::path& dataDirectory,Page&,std::wstring& error,const std::atomic_bool* cancel=nullptr);
// Downloads/validates only. Caller must present the existing Add/Replace preview.
bool prepare(const Entry&,const std::wstring& clientVersion,const std::filesystem::path& dataDirectory,PreparedCollection&,std::wstring& error,const std::atomic_bool* cancel=nullptr);
// Call only after the user-confirmed store/attachment installation succeeds.
bool recordInstallation(const PreparedCollection&,const Store& installed,const std::filesystem::path& dataDirectory,std::wstring& error);
}
