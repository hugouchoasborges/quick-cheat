#pragma once
#include <windows.h>
#include <objidl.h>
#include <wrl/client.h>
#include <filesystem>
#include <string>
namespace qc {
struct PreparedImage { Microsoft::WRL::ComPtr<IStream> stream;UINT width{},height{}; };
bool isWorkflowImageReference(const std::wstring& reference);
bool importWorkflowImage(const std::filesystem::path& source,const std::filesystem::path& dataDirectory,std::wstring& reference,std::wstring& error);
bool loadWorkflowImage(const std::filesystem::path& dataDirectory,const std::wstring& reference,UINT maximumWidth,PreparedImage& image,std::wstring& error);
}
