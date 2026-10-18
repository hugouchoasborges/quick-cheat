#pragma once
#include "model.hpp"
#include <filesystem>
#include <cstdint>
namespace qc {
struct CollectionImage {std::wstring reference;std::vector<std::uint8_t> bytes;};
struct Collection {Profile profile;std::vector<CollectionImage> images;};
struct CollectionImportCounts {size_t shortcuts{},workflows{},duplicates{};};
bool exportCollection(const std::filesystem::path& destination,const Profile& profile,const std::filesystem::path& dataDirectory,std::wstring& error);
bool readCollection(const std::filesystem::path& source,Collection& collection,std::wstring& error);
bool mergeCollection(const Store& current,const Collection& collection,bool replace,Store& candidate,CollectionImportCounts& counts,std::wstring& error);
class CollectionImageInstall {
 std::vector<std::filesystem::path> created;std::filesystem::path directory;bool createdDirectory{},committed{};
public:
 CollectionImageInstall()=default;CollectionImageInstall(const CollectionImageInstall&)=delete;CollectionImageInstall& operator=(const CollectionImageInstall&)=delete;
 ~CollectionImageInstall();
 bool install(const Collection& collection,const std::filesystem::path& dataDirectory,std::wstring& error);
 void keep() noexcept {committed=true;}
};
}
