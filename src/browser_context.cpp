#include "browser_context.hpp"
#include "browser_app.hpp"
#include "model.hpp"
#include <objbase.h>
#include <oleauto.h>
#include <UIAutomation.h>
#include <wrl/client.h>
#include <condition_variable>
#include <cwchar>
#include <deque>
#include <mutex>
#include <thread>
namespace qc {
using Microsoft::WRL::ComPtr;
namespace {
bool matchesWindow(HWND window,DWORD expected) {
 DWORD process{};return IsWindow(window)&&GetWindowThreadProcessId(window,&process)&&process==expected;
}
bool allowedForeground(HWND window) {
 return GetForegroundWindow()==window;
}
std::wstring address(IUIAutomation2* automation,HWND window,DWORD process) {
 if(!automation||!matchesWindow(window,process)||!allowedForeground(window))return {};
 const ULONGLONG deadline=GetTickCount64()+400;
 ComPtr<IUIAutomationElement> root;
 if(FAILED(automation->ElementFromHandle(window,&root)))return {};
 ComPtr<IUIAutomationTreeWalker> walker;ComPtr<IUIAutomationCacheRequest> cache;
 if(FAILED(automation->get_RawViewWalker(&walker))||FAILED(automation->CreateCacheRequest(&cache)))return {};
 cache->put_TreeScope(TreeScope_Element);
 for(auto property:{UIA_ControlTypePropertyId,UIA_ClassNamePropertyId,UIA_IsOffscreenPropertyId,UIA_HasKeyboardFocusPropertyId})cache->AddProperty(property);
 struct Node {ComPtr<IUIAutomationElement> element;unsigned depth{};bool toolbar{},externalBar{};};
 std::deque<Node> queue;queue.push_back({root,0,false,false});ComPtr<IUIAutomationElement> candidate;AppOriginEvidence app;bool traversalFailed{},depthTruncated{};
 // Modern Edge chrome can exceed 128 elements before traversal completes,
 // even when the omnibox has already been found. Keep the time bound unchanged.
 constexpr unsigned maxElements=512;unsigned examined{};
 // Walk only the browser chrome. Never descend into a document or renderer host.
 while(!queue.empty()&&examined<maxElements&&GetTickCount64()<deadline){
  auto node=std::move(queue.front());queue.pop_front();if(node.depth>=(app.appToolbar?14u:9u)){depthTruncated=true;continue;}
  ComPtr<IUIAutomationElement> child;
  if(FAILED(walker->GetFirstChildElementBuildCache(node.element.Get(),cache.Get(),&child))){traversalFailed=true;continue;}
  while(child&&examined++<maxElements&&GetTickCount64()<deadline){
   CONTROLTYPEID type{};BOOL hidden=TRUE,focused=TRUE;BSTR className{};
   if(FAILED(child->get_CachedControlType(&type))||FAILED(child->get_CachedIsOffscreen(&hidden))||FAILED(child->get_CachedHasKeyboardFocus(&focused))||FAILED(child->get_CachedClassName(&className)))traversalFailed=true;
   bool renderer=className&&wcsstr(className,L"RenderWidgetHost");
   bool appHeader=className&&wcscmp(className,L"WebAppOriginText")==0;
   bool externalBar=className&&wcscmp(className,L"CustomTabBarTitleOriginView")==0;
   bool appToolbar=className&&wcscmp(className,L"WebAppFrameToolbarView")==0;SysFreeString(className);
   if(!hidden&&!renderer&&type!=UIA_DocumentControlTypeId){
    app.appToolbar|=appToolbar;app.externalBarCount+=externalBar;app.tabs+=type==UIA_TabItemControlTypeId;
    if(appHeader||externalBar||node.externalBar){BSTR name{};if(FAILED(child->get_CurrentName(&name)))traversalFailed=true;
     if(appHeader){++app.headerCount;if(name&&SysStringLen(name)<=2048)app.header.assign(name,SysStringLen(name));}
     if((externalBar||node.externalBar)&&name&&SysStringLen(name))app.externalNavigation=true;SysFreeString(name);}
    if(type==UIA_EditControlTypeId&&node.toolbar){
     app.addressPresent=true;
     // An actively edited address can be an uncommitted URL/search, not the page.
     if(focused||candidate)return {};
     candidate=child;
    }
    if(type==UIA_PaneControlTypeId||type==UIA_GroupControlTypeId||type==UIA_WindowControlTypeId||type==UIA_CustomControlTypeId||type==UIA_ToolBarControlTypeId)
     queue.push_back({child,node.depth+1,node.toolbar||type==UIA_ToolBarControlTypeId,node.externalBar||externalBar});
   }
   ComPtr<IUIAutomationElement> next;
   if(FAILED(walker->GetNextSiblingElementBuildCache(child.Get(),cache.Get(),&next))){traversalFailed=true;break;}
   child=std::move(next);
  }
 }
 // An incomplete traversal cannot establish that the address field is unique.
 if(!queue.empty()||examined>=maxElements||GetTickCount64()>=deadline||traversalFailed)return {};
 if(!candidate){
  app.complete=!depthTruncated;std::wstring website;
  if(app.headerCount==1&&!app.externalNavigation)website=verifiedAppOrigin(app,registeredBrowserAppHost(window));
  if(!matchesWindow(window,process)||!allowedForeground(window))return {};return website;
 }
 ComPtr<IUIAutomationValuePattern> value;
 if(FAILED(candidate->GetCurrentPatternAs(UIA_ValuePatternId,IID_PPV_ARGS(&value)))||!value)return {};
 BSTR text{};if(FAILED(value->get_CurrentValue(&text))){SysFreeString(text);return {};}
 if(text&&SysStringLen(text)>2048){SysFreeString(text);return {};}
 std::wstring raw=text?std::wstring(text,SysStringLen(text)):std::wstring{};SysFreeString(text);
 if(!matchesWindow(window,process)||!allowedForeground(window))return {};
 // Chromium may omit the scheme in its accessible address. A slash then still
 // belongs to a URL; normalizeWebsite intentionally requires a scheme in that case.
 if(raw.find(L"://")==raw.npos&&raw.find_first_of(L"/?#")!=raw.npos)raw=L"https://"+raw;
 return normalizeWebsite(raw);
}
}
struct NativeBrowserContext::State {
 std::mutex mutex;std::condition_variable wake;HWND host{};UINT message{};
 bool stopping{},started{},pending{};std::uint64_t generation{};
 HWND window{};DWORD process{};std::optional<BrowserResult> result;
};
NativeBrowserContext::NativeBrowserContext(HWND host,UINT message):state(std::make_shared<State>()) {state->host=host;state->message=message;}
NativeBrowserContext::~NativeBrowserContext(){
 std::lock_guard lock(state->mutex);state->stopping=true;state->host=nullptr;state->wake.notify_one();
 // The worker owns its state. A slow external accessibility provider never blocks
 // application shutdown, and can no longer post to the destroyed notification HWND.
}
bool NativeBrowserContext::supported(const std::wstring& executable){auto key=appKey(executable);return key==L"msedge.exe"||key==L"chrome.exe";}
std::uint64_t NativeBrowserContext::request(HWND window,DWORD process,const std::wstring& executable){
 std::lock_guard lock(state->mutex);auto generation=++state->generation;state->result.reset();
 if(!supported(executable)||!matchesWindow(window,process)){state->pending=false;state->result=BrowserResult{window,process,{},generation};if(state->host)PostMessageW(state->host,state->message,0,0);return generation;}
 state->window=window;state->process=process;state->pending=true;
 if(!state->started){
  auto shared=state;
  std::thread([shared]{
   HRESULT initialized=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
   ComPtr<IUIAutomation2> automation;
   if(SUCCEEDED(initialized)&&SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation8,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&automation)))){
    automation->put_ConnectionTimeout(150);automation->put_TransactionTimeout(150);automation->put_AutoSetFocus(FALSE);
   }
   for(;;){
    BrowserResult result;
    {std::unique_lock lock(shared->mutex);shared->wake.wait(lock,[&]{return shared->stopping||shared->pending;});if(shared->stopping)break;
     result={shared->window,shared->process,{},shared->generation};shared->pending=false;}
    try{result.website=address(automation.Get(),result.window,result.process);}catch(...){result.website.clear();}
    {std::lock_guard lock(shared->mutex);if(shared->stopping)break;if(result.generation!=shared->generation)continue;
     shared->result=std::move(result);if(shared->host)PostMessageW(shared->host,shared->message,0,0);}
   }
   automation.Reset();if(SUCCEEDED(initialized))CoUninitialize();
  }).detach();state->started=true;
 }
 state->wake.notify_one();return generation;
}
void NativeBrowserContext::clear(){std::lock_guard lock(state->mutex);++state->generation;state->pending=false;state->result.reset();}
std::optional<BrowserResult> NativeBrowserContext::take(){std::lock_guard lock(state->mutex);auto result=std::move(state->result);state->result.reset();return result;}
}
