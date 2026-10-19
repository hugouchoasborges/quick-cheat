#include "internal.hpp"
#include <bcrypt.h>
#include <array>
#include <limits>
namespace qc::community::detail {
std::string utf8(const std::wstring& text){if(text.empty())return {};int size=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);require(size>0);std::string out(size,'\0');require(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),out.data(),size,nullptr,nullptr)==size);return out;}
std::wstring wide(std::span<const std::uint8_t> bytes){require(!bytes.empty()&&bytes.size()<=32*1024*1024);int size=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,reinterpret_cast<const char*>(bytes.data()),static_cast<int>(bytes.size()),nullptr,0);require(size>0);std::wstring out(size,L'\0');require(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,reinterpret_cast<const char*>(bytes.data()),static_cast<int>(bytes.size()),out.data(),size)==size);return out;}
std::wstring sha256(std::span<const std::uint8_t> bytes){
 require(bytes.size()<=(std::numeric_limits<ULONG>::max)());BCRYPT_ALG_HANDLE algorithm{};
 require(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0,L"Cannot initialize content verification.");
 std::array<UCHAR,32> digest{};auto result=BCryptHash(algorithm,nullptr,0,const_cast<PUCHAR>(bytes.data()),static_cast<ULONG>(bytes.size()),digest.data(),static_cast<ULONG>(digest.size()));BCryptCloseAlgorithmProvider(algorithm,0);
 require(result>=0,L"Cannot verify community content.");std::wstring out;constexpr wchar_t digits[]=L"0123456789abcdef";for(auto value:digest){out+=digits[value>>4];out+=digits[value&15];}return out;
}
namespace {struct File{HANDLE value;~File(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);}};struct Owned{std::filesystem::path path;bool owned{};~Owned(){if(owned)DeleteFileW(path.c_str());}};}
Bytes read(const std::filesystem::path& path,size_t maximum){File file{CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr)};require(file.value!=INVALID_HANDLE_VALUE,L"Community cache is unavailable.");LARGE_INTEGER size{};require(GetFileSizeEx(file.value,&size)&&size.QuadPart>0&&static_cast<unsigned long long>(size.QuadPart)<=maximum);Bytes bytes(static_cast<size_t>(size.QuadPart));DWORD count{};require(ReadFile(file.value,bytes.data(),static_cast<DWORD>(bytes.size()),&count,nullptr)&&count==bytes.size());return bytes;}
std::filesystem::path temporary(const std::filesystem::path& directory){static std::atomic_uint sequence{};return directory/(L"preview-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64())+L"-"+std::to_wstring(++sequence)+L".json");}
void write(const std::filesystem::path& destination,std::span<const std::uint8_t> bytes,bool replace){
 require(bytes.size()<=32*1024*1024);std::error_code error;std::filesystem::create_directories(destination.parent_path(),error);require(!error,L"Cannot create the community data directory.");
 Owned temporaryFile{temporary(destination.parent_path())};File file{CreateFileW(temporaryFile.path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr)};
 require(file.value!=INVALID_HANDLE_VALUE,L"Cannot create community cache data.");temporaryFile.owned=true;DWORD count{};
 require(WriteFile(file.value,bytes.data(),static_cast<DWORD>(bytes.size()),&count,nullptr)&&count==bytes.size()&&FlushFileBuffers(file.value),L"Cannot write complete community cache data.");
 CloseHandle(file.value);file.value=INVALID_HANDLE_VALUE;
 require(MoveFileExW(temporaryFile.path.c_str(),destination.c_str(),MOVEFILE_WRITE_THROUGH|(replace?MOVEFILE_REPLACE_EXISTING:0)),L"Cannot preserve community cache data.");temporaryFile.owned=false;
}
}
