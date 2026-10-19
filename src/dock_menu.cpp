#include "dock.hpp"
#include "app.hpp"
#include <algorithm>
namespace qc {
void Dock::contextMenu(POINT point){
 if(menuOpen||dragging||GetForegroundWindow()!=targetWindow||!targetAlive(app.target))return;
 auto* source=findProfile(app.store,key);if(!source)return;
 const auto menuKey=key;const auto expectedWindow=targetWindow;const auto expectedProcess=targetProcess;
 const auto config=source->dock;
 struct Menu{HMENU value=CreatePopupMenu();~Menu(){if(value)DestroyMenu(value);}} menu;
 if(!menu.value)return;
 constexpr UINT Enable=1,Horizontal=2,Vertical=3,Settings=4,Scale=100,Opacity=400;
 AppendMenuW(menu.value,MF_STRING|(config.enabled?MF_CHECKED:MF_UNCHECKED),Enable,L"Enable Dock");
 HMENU orientation=CreatePopupMenu();if(!orientation)return;
 AppendMenuW(orientation,MF_STRING,Horizontal,L"Horizontal");AppendMenuW(orientation,MF_STRING,Vertical,L"Vertical");
 CheckMenuRadioItem(orientation,Horizontal,Vertical,config.vertical?Vertical:Horizontal,MF_BYCOMMAND);
 AppendMenuW(menu.value,MF_POPUP,reinterpret_cast<UINT_PTR>(orientation),L"Orientation");
 auto percentages=[&](const wchar_t* label,UINT base,UINT current,std::vector<UINT> values){
  if(std::find(values.begin(),values.end(),current)==values.end())values.push_back(current);
  std::sort(values.begin(),values.end());HMENU sub=CreatePopupMenu();if(!sub)return false;
  for(auto value:values)AppendMenuW(sub,MF_STRING,base+value,(std::to_wstring(value)+L"%").c_str());
  CheckMenuRadioItem(sub,base+values.front(),base+values.back(),base+current,MF_BYCOMMAND);
  AppendMenuW(menu.value,MF_POPUP,reinterpret_cast<UINT_PTR>(sub),label);return true;
 };
 if(!percentages(L"Scale",Scale,config.scale,{50,75,100,125,150,175,200})||!percentages(L"Opacity",Opacity,config.opacity,{20,30,40,50,60,70,80,90,100}))return;
 AppendMenuW(menu.value,MF_SEPARATOR,0,nullptr);AppendMenuW(menu.value,MF_STRING,Settings,L"Settings");
 if(point.x==-1&&point.y==-1){RECT rect{};GetWindowRect(window,&rect);point={rect.left+4,rect.bottom};}
 menuOpen=true;SetForegroundWindow(window);
 UINT command=TrackPopupMenuEx(menu.value,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,point.x,point.y,window,nullptr);
 menuOpen=false;PostMessageW(window,WM_NULL,0,0);
 if(GetForegroundWindow()==window)SetForegroundWindow(expectedWindow);
 if(!command||app.target.window!=expectedWindow||app.target.process!=expectedProcess||!targetAlive(app.target)||GetForegroundWindow()!=expectedWindow){refresh();return;}
 if(command==Settings){
  app.openEditor(3);
  if(app.editor&&app.editor->page==3)for(size_t i=0;i<app.editor->draft.profiles.size();++i)if(profileKey(app.editor->draft.profiles[i])==menuKey){
   if(app.editor->selectProfile(static_cast<int>(i)))SendMessageW(app.editor->profiles,CB_SETCURSEL,i,0);break;
  }
  refresh();return;
 }
 if(app.editor&&app.editor->pendingPageEdits()){
  app.error(L"Save or discard pending Settings edits before changing the dock.",window);
  if(GetForegroundWindow()==window)SetForegroundWindow(expectedWindow);refresh();return;
 }
 auto candidate=app.store;auto* profile=findProfile(candidate,menuKey);if(!profile){refresh();return;}
 if(command==Enable)profile->dock.enabled=!profile->dock.enabled;
 else if(command==Horizontal||command==Vertical)profile->dock.vertical=command==Vertical;
 else if(command>=Scale+50&&command<=Scale+200)profile->dock.scale=command-Scale;
 else if(command>=Opacity+20&&command<=Opacity+100)profile->dock.opacity=command-Opacity;
 else {refresh();return;}
 saving=true;bool saved=app.commit(std::move(candidate));saving=false;
 if(saved&&app.editor)if(auto* draft=findProfile(app.editor->draft,menuKey))if(auto* actual=findProfile(app.store,menuKey)){
  draft->dock=actual->dock;
  if(app.editor->page==3&&app.editor->dockSettings&&app.editor->profileIndex>=0&&profileKey(app.editor->draft.profiles[app.editor->profileIndex])==menuKey)app.editor->dockSettings->show(menuKey);
 }
 renderKey.clear();refresh();
}
}
