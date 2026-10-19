#include "storage.hpp"
#include <shlobj.h>
#include <limits>
namespace qc {
namespace {
struct Handle { HANDLE value{INVALID_HANDLE_VALUE}; ~Handle(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);} };
bool fail(std::wstring& error,const wchar_t* operation) { error=std::wstring(operation)+L" (Windows error "+std::to_wstring(GetLastError())+L").";return false; }
bool preserveLegacy(const std::filesystem::path& file,std::wstring& error){
 bool legacy=false;
 {Handle h{CreateFileW(file.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr)};
 if(h.value==INVALID_HANDLE_VALUE){if(GetLastError()==ERROR_FILE_NOT_FOUND||GetLastError()==ERROR_PATH_NOT_FOUND)return true;return fail(error,L"Cannot inspect data before migration");}
 char header[5]{};DWORD read=0;if(!ReadFile(h.value,header,5,&read,nullptr))return fail(error,L"Cannot inspect data before migration");
 legacy=read==5&&header[0]=='Q'&&header[1]=='C'&&header[2]=='\t'&&(header[3]=='1'||header[3]=='2'||header[3]=='3'||header[3]=='4')&&header[4]=='\n';}
 if(legacy&&!CopyFileW(file.c_str(),(file.wstring()+L".pre-v5.bak").c_str(),TRUE)&&GetLastError()!=ERROR_FILE_EXISTS)return fail(error,L"Cannot preserve pre-migration backup");
 return true;
}
}
std::filesystem::path dataPath() {
 PWSTR folder=nullptr;
 if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,KF_FLAG_CREATE,nullptr,&folder)))return {};
 auto path=std::filesystem::path(folder)/L"QuickCheat"/L"shortcuts.qc";CoTaskMemFree(folder);return path;
}
bool loadStore(const std::filesystem::path& file,Store& store,std::wstring& error) {
 error.clear();Handle h{CreateFileW(file.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr)};
 if(h.value==INVALID_HANDLE_VALUE) { if(GetLastError()==ERROR_FILE_NOT_FOUND||GetLastError()==ERROR_PATH_NOT_FOUND){store=Store{};return true;}return fail(error,L"Cannot open shortcut file"); }
 LARGE_INTEGER size;if(!GetFileSizeEx(h.value,&size))return fail(error,L"Cannot read file size");
 if(size.QuadPart<=0||static_cast<unsigned long long>(size.QuadPart)>maxStoreBytes) {error=L"The shortcut file is empty or exceeds the size limit.";return false;}
 std::string bytes(static_cast<size_t>(size.QuadPart),'\0');DWORD read=0;
 if(!ReadFile(h.value,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)||read!=bytes.size())return fail(error,L"Cannot read shortcut file");
 int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),nullptr,0);
 if(n<=0) {error=L"Invalid UTF-8 in shortcut file.";return false;}
 std::wstring text(n,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),text.data(),n);
 if(!deserialize(text,store)) {error=L"Invalid or unsupported shortcut file. Your file has not been changed. A previous save may be available in shortcuts.qc.bak.";return false;}
 return true;
}
bool saveStore(const std::filesystem::path& file,const Store& store,std::wstring& error) {
 error.clear();if(!validStore(store)){error=L"Invalid shortcut data or size limit reached.";return false;}
 const auto text=serialize(store);int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);
 if(n<=0||static_cast<size_t>(n)>maxStoreBytes){error=L"Cannot encode shortcut data or the 8 MiB file size limit was exceeded.";return false;}
 std::string bytes(n,'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),bytes.data(),n,nullptr,nullptr);
 if(!preserveLegacy(file,error))return false;
 std::error_code ec;std::filesystem::create_directories(file.parent_path(),ec);if(ec){error=L"Cannot create the data directory.";return false;}
 const auto temp=file.wstring()+L".tmp."+std::to_wstring(GetCurrentProcessId());
 bool written=false;
 { Handle h{CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr)};
 if(h.value==INVALID_HANDLE_VALUE)return fail(error,L"Cannot create temporary file");DWORD count=0;
 written=WriteFile(h.value,bytes.data(),n,&count,nullptr)&&count==bytes.size()&&FlushFileBuffers(h.value);if(!written)fail(error,L"Cannot persist shortcut data"); }
 if(!written){DeleteFileW(temp.c_str());return false;}
 const auto backup=file.wstring()+L".bak";
 BOOL ok=GetFileAttributesW(file.c_str())!=INVALID_FILE_ATTRIBUTES ? ReplaceFileW(file.c_str(),temp.c_str(),backup.c_str(),0,nullptr,nullptr) : MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_WRITE_THROUGH);
 if(!ok){fail(error,L"Cannot replace shortcut file");DeleteFileW(temp.c_str());return false;}return true;
}
}
