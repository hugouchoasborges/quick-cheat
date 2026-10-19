#include "internal.hpp"
#include <winhttp.h>
#include <algorithm>
#include <array>
namespace qc::community::detail {
std::wstring encode(const std::wstring& value){std::wstring out;constexpr wchar_t digits[]=L"0123456789ABCDEF";for(unsigned char c:utf8(value)){if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')out+=c;else{out+=L'%';out+=digits[c>>4];out+=digits[c&15];}}return out;}
bool get(const std::wstring& relative,size_t maximum,Bytes& bytes,std::wstring& error,const std::atomic_bool* cancel){
 return guarded(error,[&]{
  require(relative.starts_with(L"/")&&relative.size()<=4096&&relative.find_first_of(L"\r\n#")==relative.npos);bytes.clear();
  auto canceled=[&]{return cancel&&cancel->load();};require(!canceled(),L"Community request canceled.");
  struct Internet{HINTERNET value{};~Internet(){if(value)WinHttpCloseHandle(value);}};
  Internet session{WinHttpOpen(L"QuickCheat/0.6-catalog-preview",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0)};
  require(session.value,L"Community networking is unavailable.");require(WinHttpSetTimeouts(session.value,1000,2000,2000,2000),L"Cannot bound community request timeouts.");
  Internet connection{WinHttpConnect(session.value,L"portal.hugoborges.dev.br",INTERNET_DEFAULT_HTTPS_PORT,0)};require(connection.value,L"Cannot connect to the community service.");
  auto path=L"/apps/quick-cheat/api/v1"+relative;
  Internet request{WinHttpOpenRequest(connection.value,L"GET",path.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE)};require(request.value);
  DWORD redirect=WINHTTP_OPTION_REDIRECT_POLICY_NEVER;require(WinHttpSetOption(request.value,WINHTTP_OPTION_REDIRECT_POLICY,&redirect,sizeof(redirect)));
  require(WinHttpSendRequest(request.value,L"Accept: application/json, application/vnd.quick-cheat.collection+json\r\n",static_cast<DWORD>(-1L),WINHTTP_NO_REQUEST_DATA,0,0,0)&&WinHttpReceiveResponse(request.value,nullptr),L"Community service unavailable. Local data is unaffected.");
  DWORD status{},size=sizeof(status);require(WinHttpQueryHeaders(request.value,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX));
  require(status==200,L"Community service refused or could not complete the request.");
  std::array<wchar_t,256> contentType{};size=static_cast<DWORD>(contentType.size()*sizeof(wchar_t));require(WinHttpQueryHeaders(request.value,WINHTTP_QUERY_CONTENT_TYPE,WINHTTP_HEADER_NAME_BY_INDEX,contentType.data(),&size,WINHTTP_NO_HEADER_INDEX));
  std::wstring type(contentType.data());type=type.substr(0,type.find(L';'));while(!type.empty()&&type.back()==L' ')type.pop_back();for(auto& c:type)if(c>=L'A'&&c<=L'Z')c+=L'a'-L'A';require(type==L"application/json"||type==L"application/vnd.quick-cheat.collection+json",L"Community service returned an unexpected document.");
  std::array<std::uint8_t,8192> buffer{};ULONGLONG deadline=GetTickCount64()+15000;
  for(;;){require(!canceled(),L"Community request canceled.");require(GetTickCount64()<deadline,L"Community download timed out.");DWORD count{};require(WinHttpReadData(request.value,buffer.data(),static_cast<DWORD>(buffer.size()),&count),L"Community download was interrupted.");if(!count)break;require(bytes.size()+count<=maximum,L"Community response exceeds its size limit.");bytes.insert(bytes.end(),buffer.begin(),buffer.begin()+count);}
  require(!bytes.empty());
 });
}
}
