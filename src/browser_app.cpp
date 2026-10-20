#include "browser_app.hpp"
#include "model.hpp"
#include <appmodel.h>
#include <msxml6.h>
#include <propkey.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <wrl/client.h>
#include <filesystem>
#include <fstream>
#include <vector>
namespace qc {
using Microsoft::WRL::ComPtr;
namespace {
struct Text {
 BSTR value;
 explicit Text(const std::wstring& text):value(SysAllocStringLen(text.data(),static_cast<UINT>(text.size()))){}
 ~Text(){SysFreeString(value);}
 Text(const Text&)=delete;Text& operator=(const Text&)=delete;
 operator BSTR()const{return value;}
};
std::wstring nodeText(IXMLDOMNode* node,const wchar_t* query){
 ComPtr<IXMLDOMNode> child;if(FAILED(node->selectSingleNode(Text(query),&child))||!child)return {};
 BSTR value{};if(FAILED(child->get_text(&value)))return {};std::wstring result=value?std::wstring(value,SysStringLen(value)):L"";SysFreeString(value);return result;
}
std::wstring windowAppId(HWND window){
 ComPtr<IPropertyStore> properties;if(FAILED(SHGetPropertyStoreForWindow(window,IID_PPV_ARGS(&properties))))return {};
 PROPVARIANT value;PropVariantInit(&value);std::wstring result;
 if(SUCCEEDED(properties->GetValue(PKEY_AppUserModel_ID,&value))&&value.vt==VT_LPWSTR&&value.pwszVal)result=value.pwszVal;
 PropVariantClear(&value);return result;
}
std::wstring readManifest(const std::filesystem::path& path){
 std::ifstream file(path,std::ios::binary|std::ios::ate);if(!file)return {};auto size=file.tellg();if(size<=0||size>65536)return {};
 std::string bytes(static_cast<size_t>(size),'\0');file.seekg(0);if(!file.read(bytes.data(),size))return {};
 if(bytes.size()>=2&&static_cast<unsigned char>(bytes[0])==0xff&&static_cast<unsigned char>(bytes[1])==0xfe){
  if(bytes.size()%2)return {};std::wstring xml;xml.reserve(bytes.size()/2-1);for(size_t i=2;i<bytes.size();i+=2)xml.push_back(static_cast<wchar_t>(static_cast<unsigned char>(bytes[i])|(static_cast<unsigned char>(bytes[i+1])<<8)));return xml;
 }
 int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),nullptr,0);if(!count)return {};
 std::wstring xml(count,L'\0');if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),xml.data(),count))return {};return xml;
}
}
std::wstring installedAppStartHost(const std::wstring& description){
 if(description.size()>8192)return {};std::wstring result;unsigned matches{};size_t offset{};
 do{auto end=description.find(L';',offset);auto field=description.substr(offset,end==description.npos?end:end-offset);
  if(field.starts_with(L"start-url?")){auto url=field.substr(10);if(++matches>1||!validWebUrl(url))return {};result=normalizeWebsite(url);}
  if(end==description.npos)break;offset=end+1;
 }while(offset<description.size());return result;
}
std::wstring manifestBrowserAppHost(const std::wstring& xml,const std::wstring& applicationId){
 if(xml.empty()||xml.size()>65536||applicationId.empty())return {};
 ComPtr<IXMLDOMDocument2> document;if(FAILED(CoCreateInstance(__uuidof(DOMDocument60),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&document))))return {};
 document->put_async(VARIANT_FALSE);document->put_resolveExternals(VARIANT_FALSE);document->put_validateOnParse(VARIANT_FALSE);
 VARIANT prohibited;VariantInit(&prohibited);prohibited.vt=VT_BOOL;prohibited.boolVal=VARIANT_TRUE;if(FAILED(document->setProperty(Text(L"ProhibitDTD"),prohibited)))return {};
 VARIANT_BOOL loaded=VARIANT_FALSE;if(FAILED(document->loadXML(Text(xml),&loaded))||loaded!=VARIANT_TRUE)return {};
 ComPtr<IXMLDOMNodeList> applications;if(FAILED(document->selectNodes(Text(L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']"),&applications))||!applications)return {};
 long count{};applications->get_length(&count);if(count>64)return {};std::wstring result;unsigned matches{};
 for(long i=0;i<count;++i){ComPtr<IXMLDOMNode> app;applications->get_item(i,&app);if(!app||nodeText(app.Get(),L"@Id")!=applicationId)continue;
  if(nodeText(app.Get(),L"@*[local-name()='HostId']")!=L"PWA")return {};
  ComPtr<IXMLDOMNodeList> extensions;if(FAILED(app->selectNodes(Text(L"./*[local-name()='Extensions']/*[local-name()='Extension']/*[local-name()='AppExtension']"),&extensions))||!extensions)return {};
  long length{};extensions->get_length(&length);if(length>64)return {};
  for(long j=0;j<length;++j){ComPtr<IXMLDOMNode> extension;extensions->get_item(j,&extension);if(!extension||nodeText(extension.Get(),L"@Name")!=L"com.ms.webapp.internals.4")continue;
   if(++matches>1)return {};result=installedAppStartHost(nodeText(extension.Get(),L"@Description"));}
 }
 return result;
}
std::wstring registeredBrowserAppHost(HWND window){
 const auto identity=windowAppId(window);auto separator=identity.find(L'!');if(separator==identity.npos||separator==0||separator+1==identity.size()||identity.size()>512)return {};
 auto family=identity.substr(0,separator),application=identity.substr(separator+1);UINT32 count{},length{};
 if(GetPackagesByPackageFamily(family.c_str(),&count,nullptr,&length,nullptr)!=ERROR_INSUFFICIENT_BUFFER||count!=1||length>32768)return {};
 std::vector<PWSTR> names(count);std::vector<wchar_t> buffer(length);
 if(GetPackagesByPackageFamily(family.c_str(),&count,names.data(),&length,buffer.data())!=ERROR_SUCCESS||count!=1)return {};
 UINT32 pathLength{};if(GetPackagePathByFullName(names[0],&pathLength,nullptr)!=ERROR_INSUFFICIENT_BUFFER||pathLength>32768)return {};
 std::wstring path(pathLength,L'\0');if(GetPackagePathByFullName(names[0],&pathLength,path.data())!=ERROR_SUCCESS)return {};path.resize(wcslen(path.c_str()));
 auto result=manifestBrowserAppHost(readManifest(std::filesystem::path(path)/L"AppxManifest.xml"),application);
 return identity==windowAppId(window)?result:L"";
}
bool hasPackagedAppIdentity(HWND window){auto identity=windowAppId(window);auto separator=identity.find(L'!');return separator!=identity.npos&&separator>0&&separator+1<identity.size();}
std::wstring verifiedAppOrigin(const AppOriginEvidence& evidence,const std::wstring& registeredHost){
 if(!evidence.complete||evidence.addressPresent||evidence.externalNavigation||!evidence.appToolbar||evidence.externalBarCount!=1||evidence.tabs>1||evidence.headerCount!=1||registeredHost.empty())return {};
 // Only an origin label from native browser chrome is accepted, never a page title.
 auto header=evidence.header;auto scheme=header.find(L"://");if(scheme!=header.npos)header.erase(0,scheme+3);
 if(header.find_first_of(L"/?#")!=header.npos)return {};
 return normalizeWebsite(evidence.header)==registeredHost?registeredHost:L"";
}
}
