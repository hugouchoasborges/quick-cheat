#pragma once
#include "catalog.hpp"
#include <windows.h>
#include <roapi.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <span>
namespace qc::community::detail {
using Bytes=std::vector<std::uint8_t>;
using namespace winrt::Windows::Data::Json;
struct Failure {std::wstring message;};
[[noreturn]] inline void fail(const wchar_t* message){throw Failure{message};}
inline void require(bool condition,const wchar_t* message=L"Invalid community catalog data."){if(!condition)fail(message);}
struct Apartment {HRESULT result=RoInitialize(RO_INIT_MULTITHREADED);Apartment(){require(SUCCEEDED(result),L"Community requests must run on an available worker thread.");}~Apartment(){RoUninitialize();}};
std::string utf8(const std::wstring&);
std::wstring wide(std::span<const std::uint8_t>);
std::wstring sha256(std::span<const std::uint8_t>);
Bytes read(const std::filesystem::path&,size_t maximum);
void write(const std::filesystem::path&,std::span<const std::uint8_t>,bool replace=true);
std::filesystem::path temporary(const std::filesystem::path& directory);
bool get(const std::wstring& relative,size_t maximum,Bytes&,std::wstring& error,const std::atomic_bool* cancel);
std::wstring encode(const std::wstring&);
JsonObject object(const Bytes&);
std::wstring field(const JsonObject&,const wchar_t* key,size_t maximum,bool empty=false);
unsigned number(const JsonObject&,const wchar_t* key,unsigned maximum);
bool identifier(const std::wstring&);
bool version(const std::wstring&);
Page page(const JsonObject&);
template<class Action> bool guarded(std::wstring& error,Action&& action){error.clear();try{action();return true;}catch(const Failure& problem){error=problem.message;}catch(const winrt::hresult_error&){error=L"Invalid catalog JSON or unavailable Windows data service.";}catch(const std::exception&){error=L"Community data could not be processed. Local profiles were not changed.";}return false;}
}
