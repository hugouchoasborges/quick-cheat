#include "app.hpp"
#include "browser_context.hpp"
#include "dock.hpp"
namespace qc {
std::wstring App::contextName(){
 if(isBrowserExecutable(target.executable))return target.website.empty()?target.executable:target.website+L" — "+target.executable;
 auto* profile=currentProfile();return profile?profile->name:target.name.empty()?L"No active application":target.name;
}
Profile* App::currentProfile(){
 if(!contextDirty)return hasContextProfile?&contextProfile:nullptr;contextDirty=false;contextProfile={};browserShortcutCount=browserWorkflowCount=0;
 auto* general=findProfile(store,target.executable);auto* site=target.website.empty()?nullptr:findProfile(store,target.executable+L"|"+target.website);hasContextProfile=general||site;if(!hasContextProfile)return nullptr;
 if(general)contextProfile=*general;else{contextProfile.executable=target.executable;contextProfile.name=target.name;}
 browserShortcutCount=contextProfile.shortcuts.size();browserWorkflowCount=contextProfile.workflows.size();contextProfile.website=target.website;
 if(isBrowserExecutable(target.executable)){
  for(auto& shortcut:contextProfile.shortcuts)shortcut.category=L"Browser / "+(shortcut.category.empty()?L"General":shortcut.category);
  if(!target.website.empty())contextProfile.name+=(contextProfile.name.empty()?L"":L" — ")+target.website;
  if(site){for(auto shortcut:site->shortcuts){shortcut.category=target.website+L" / "+(shortcut.category.empty()?L"General":shortcut.category);contextProfile.shortcuts.push_back(std::move(shortcut));}contextProfile.workflows.insert(contextProfile.workflows.end(),site->workflows.begin(),site->workflows.end());}
 }
 return &contextProfile;
}
void App::requestWebsite(){
 if(!browser||pendingExecution||pendingPopupOpen)return;if(NativeBrowserContext::supported(target.executable)&&targetAlive(target)){if(GetForegroundWindow()==target.window)browser->request(target.window,target.process,target.executable);}else browser->clear();
}
void App::receiveWebsite(){
 if(!browser)return;auto result=browser->take();if(!result)return;
 bool open=pendingPopupOpen&&result->generation==openingGeneration;if(open)pendingPopupOpen=false;
 bool complete=pendingExecution&&result->generation==pendingGeneration;auto expected=pendingTarget;auto chord=pendingChord;if(complete)pendingExecution=false;
 if(result->window!=target.window||result->process!=target.process||!targetAlive(target))return;
 if(target.website!=result->website){syncCategoryStates();target.website=result->website;contextDirty=true;if(popup&&IsWindowVisible(popup))refreshPopup();if(dock)dock->refresh();}
 if(open&&GetForegroundWindow()==target.window)showPopupNow();
 if(complete){HWND foreground=GetForegroundWindow();DWORD process{};GetWindowThreadProcessId(foreground,&process);
  if(expected.window!=target.window||expected.process!=target.process||expected.website!=result->website||(foreground!=target.window&&process!=GetCurrentProcessId())){error(L"The active website changed or could not be confirmed. Open the shortcut list again before executing this website shortcut.");return;}
  std::wstring message;if(!sendChord(target,chord,message))error(message);else if(!popupPinned)hidePopup(false);
 }
}
void App::runShortcut(Chord chord,const std::wstring& expectedWebsite){
 if(pendingExecution)return;
 if(chord.key==store.trigger.key&&chord.modifiers==store.trigger.modifiers){error(L"This shortcut is reserved for Quick Cheat activation.");return;}
 HWND foreground=GetForegroundWindow();DWORD process{};GetWindowThreadProcessId(foreground,&process);if(!targetAlive(target)||(foreground!=target.window&&process!=GetCurrentProcessId()))return;
 if(!expectedWebsite.empty()){
  if(pendingExecution||!browser||!NativeBrowserContext::supported(target.executable))return;
  if(!popupPinned)hidePopup(false);
  if(GetForegroundWindow()!=target.window){SetForegroundWindow(target.window);if(GetForegroundWindow()!=target.window){error(L"Activate the browser window before executing this website shortcut.");return;}}
  pendingTarget=target;pendingTarget.website=expectedWebsite;pendingChord=chord;pendingExecution=true;pendingGeneration=browser->request(target.window,target.process,target.executable);return;
 }
 std::wstring message;if(!sendChord(target,chord,message))error(message);else if(!popupPinned)hidePopup(false);
}
}
