#pragma once
#include "model.hpp"
#include <filesystem>
namespace qc {
std::filesystem::path dataPath();
bool loadStore(const std::filesystem::path& file, Store& store, std::wstring& error);
bool saveStore(const std::filesystem::path& file, const Store& store, std::wstring& error);
}
