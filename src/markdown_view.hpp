#pragma once
#include "theme.hpp"
#include <richedit.h>
#include <filesystem>
#include <vector>
namespace qc {
HWND createMarkdownView(HWND parent,int id);
void showMarkdown(HWND reader,const std::wstring& markdown,const std::filesystem::path& dataDirectory,const Palette& palette);
void showMarkdownSections(HWND reader,const std::vector<std::wstring>& sections,const std::filesystem::path& dataDirectory,const Palette& palette);
void restyleMarkdown(HWND reader,const Palette& palette);
void clearMarkdown(HWND reader);
bool handleMarkdownLink(ENLINK* event);
}
