#include "browser_app.hpp"
#include "browser_context.hpp"
#include <objbase.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <wrl/client.h>
#include <iostream>
#include <stdexcept>
#define CHECK(x) do { if(!(x))throw std::runtime_error(#x); } while(false)
int wmain(int argc,wchar_t** argv){
 CoInitializeEx(nullptr,COINIT_MULTITHREADED);
 struct ComScope{~ComScope(){CoUninitialize();}} com;
 try{
  using namespace qc;
  if(argc==4&&std::wstring(argv[1])==L"--live"){
   auto window=reinterpret_cast<HWND>(wcstoull(argv[2],nullptr,16));DWORD process{};GetWindowThreadProcessId(window,&process);
   CHECK(IsWindow(window));auto previous=GetForegroundWindow();auto foregroundThread=GetWindowThreadProcessId(previous,nullptr);auto thread=GetCurrentThreadId();bool attached=thread!=foregroundThread&&AttachThreadInput(thread,foregroundThread,TRUE);
   SetForegroundWindow(window);if(attached)AttachThreadInput(thread,foregroundThread,FALSE);if(GetForegroundWindow()!=window)throw std::runtime_error("Could not activate test target");
   NativeBrowserContext browser(nullptr,0);auto generation=browser.request(window,process,L"msedge.exe");std::optional<BrowserResult> result;
   auto deadline=GetTickCount64()+5000;while(GetTickCount64()<deadline&&!(result=browser.take()))Sleep(20);
   SetForegroundWindow(previous);CHECK(result&&result->generation==generation);std::wcout<<L"website="<<result->website<<L"\n";CHECK(result->website==argv[3]);return 0;
  }
  CHECK(installedAppStartHost(L"parameters?--app-id=x;profile-directory?Profile 1;start-url?https://tgsgaming.monday.com/my_work")==L"tgsgaming.monday.com");
  CHECK(installedAppStartHost(L"start-url?https://other.example/path")==L"other.example");
  CHECK(installedAppStartHost(L"start-url?file:///C:/temp/app.html").empty());
  CHECK(installedAppStartHost(L"start-url?https://good.example;start-url?https://other.example").empty());
  CHECK(installedAppStartHost(L"parameters?--app-id=x").empty());
  auto manifest=[](const std::wstring& hostId,const std::wstring& description){return L"<Package xmlns:uap10='urn:uap10'><Applications><Application Id='App' uap10:HostId='"+hostId+L"'><Extensions><Extension><AppExtension Name='com.ms.webapp.internals.4' Description='"+description+L"'/></Extension></Extensions></Application></Applications></Package>";};
  CHECK(manifestBrowserAppHost(manifest(L"PWA",L"start-url?https://tgsgaming.monday.com/my_work"),L"App")==L"tgsgaming.monday.com");
  CHECK(manifestBrowserAppHost(manifest(L"Other",L"start-url?https://tgsgaming.monday.com"),L"App").empty());
  CHECK(manifestBrowserAppHost(manifest(L"PWA",L"start-url?https://tgsgaming.monday.com"),L"Missing").empty());
  CHECK(manifestBrowserAppHost(L"<!DOCTYPE p [<!ENTITY x SYSTEM 'file:///C:/Windows/win.ini'>]><Package>&x;</Package>",L"App").empty());
  CHECK(manifestBrowserAppHost(L"<Package>",L"App").empty());
  auto fixture=CreateWindowExW(0,L"STATIC",L"Browser identity fixture",WS_POPUP,0,0,1,1,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);CHECK(fixture);
  struct WindowScope{HWND value;~WindowScope(){DestroyWindow(value);}} fixtureScope{fixture};
  Microsoft::WRL::ComPtr<IPropertyStore> properties;CHECK(SUCCEEDED(SHGetPropertyStoreForWindow(fixture,IID_PPV_ARGS(&properties))));
  CHECK(!hasPackagedAppIdentity(fixture));PROPVARIANT identity;CHECK(SUCCEEDED(InitPropVariantFromString(L"tgsgaming.monday.com-3178D4AD_2tfrzefmp6n6m!App",&identity)));
  CHECK(SUCCEEDED(properties->SetValue(PKEY_AppUserModel_ID,identity)));PropVariantClear(&identity);CHECK(hasPackagedAppIdentity(fixture));
  CHECK(SUCCEEDED(InitPropVariantFromString(L"MSEdge.UserData.Profile1",&identity)));CHECK(SUCCEEDED(properties->SetValue(PKEY_AppUserModel_ID,identity)));PropVariantClear(&identity);CHECK(!hasPackagedAppIdentity(fixture));
  AppOriginEvidence evidence;evidence.complete=true;evidence.appToolbar=true;evidence.externalBarCount=1;evidence.header=L"tgsgaming.monday.com";evidence.headerCount=1;
  CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com")==L"tgsgaming.monday.com");
  CHECK(verifiedAppOrigin(evidence,L"other.example").empty());
  CHECK(verifiedAppOrigin(evidence,L"").empty());
  evidence.externalNavigation=true;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.externalNavigation=false;
  evidence.complete=false;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.complete=true;
  evidence.headerCount=2;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.headerCount=1;
  evidence.tabs=2;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.tabs=0;
  evidence.addressPresent=true;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.addressPresent=false;
  evidence.externalBarCount=0;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.externalBarCount=1;
  evidence.appToolbar=false;CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());evidence.appToolbar=true;
  evidence.header=L"Monday - tgsgaming.monday.com";CHECK(verifiedAppOrigin(evidence,L"tgsgaming.monday.com").empty());
  std::cout<<"browser app policy tests passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}
}
