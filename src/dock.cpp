#include "dock.hpp"
#include "app.hpp"
#include <algorithm>
namespace qc {
namespace {
constexpr UINT Collapse=700,Previous=701,Next=702,FirstShortcut=1000;
HWND child(HWND parent,const wchar_t* cls,const wchar_t* label,DWORD style,int id,HINSTANCE instance){return CreateWindowExW(0,cls,label,WS_CHILD|WS_VISIBLE|style,0,0,1,1,parent,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),instance,nullptr);}
}
Dock::Dock(App& value):app(value){}
Dock::~Dock(){if(window)DestroyWindow(window);if(font)DeleteObject(font);}
void Dock::themeChanged(){renderKey.clear();if(window)app.theme.style(window);refresh();}
void Dock::refresh(){
 if(saving)return;
 if(menuOpen){if(app.target.window!=targetWindow||app.target.process!=targetProcess||!IsWindow(targetWindow))EndMenu();return;}
 auto hide=[&]{dragging=false;HWND captured=GetCapture();if(captured&&(captured==window||(window&&IsChild(window,captured))))ReleaseCapture();if(window)ShowWindow(window,SW_HIDE);};
 if(!targetAlive(app.target)||GetForegroundWindow()!=app.target.window||IsIconic(app.target.window)||(app.popup&&IsWindowVisible(app.popup))){hide();return;}
 if(dragging)return;Profile* profile=nullptr;
 if(!app.target.website.empty()){auto* site=findProfile(app.store,app.target.executable+L"|"+app.target.website);if(site&&site->dock.enabled)profile=site;}
 if(!profile)profile=findProfile(app.store,app.target.executable);
 if(!profile||!profile->dock.enabled||profile->dock.shortcuts.empty()){hide();return;}
 if(key!=profileKey(*profile))page=0;key=profileKey(*profile);targetWindow=app.target.window;targetProcess=app.target.process;
 if(!window){WNDCLASSW cls{};cls.style=CS_DBLCLKS;cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatDock";cls.hCursor=LoadCursorW(nullptr,IDC_SIZEALL);RegisterClassW(&cls);
 window=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE|WS_EX_TOPMOST|WS_EX_LAYERED,cls.lpszClassName,L"Quick Cheat dock",WS_POPUP|WS_BORDER,0,0,1,1,nullptr,nullptr,app.instance,this);if(!window)return;}
 dpi=GetDpiForWindow(app.target.window);MONITORINFO work{sizeof(work)};GetMonitorInfoW(MonitorFromWindow(app.target.window,MONITOR_DEFAULTTONEAREST),&work);std::wstring stamp=key+L"/"+std::to_wstring(dpi)+L"/"+std::to_wstring(profile->dock.scale)+L"/"+std::to_wstring(profile->dock.vertical)+L"/"+std::to_wstring(profile->dock.collapsed)+L"/"+std::to_wstring(work.rcWork.right-work.rcWork.left)+L"/"+std::to_wstring(work.rcWork.bottom-work.rcWork.top);for(auto index:profile->dock.shortcuts)if(index<profile->shortcuts.size()){auto& shortcut=profile->shortcuts[index];stamp+=L"/"+std::to_wstring(index)+L":"+std::to_wstring(shortcut.title.size())+L":"+shortcut.title+shortcutActionText(shortcut)+shortcut.url+L"/"+shortcut.dockLabel;}if(stamp!=renderKey){rebuild(*profile);renderKey=std::move(stamp);}
 RECT target{};GetWindowRect(app.target.window,&target);RECT rect{};GetWindowRect(window,&rect);int width=rect.right-rect.left,height=rect.bottom-rect.top;
 int x=target.left+(profile->dock.x<0?MulDiv(24,dpi,96):MulDiv(profile->dock.x,dpi,96));int y=target.top+(profile->dock.y<0?MulDiv(80,dpi,96):MulDiv(profile->dock.y,dpi,96));
 MONITORINFO monitor{sizeof(monitor)};GetMonitorInfoW(MonitorFromWindow(app.target.window,MONITOR_DEFAULTTONEAREST),&monitor);x=std::clamp<int>(x,monitor.rcWork.left,std::max(monitor.rcWork.left,monitor.rcWork.right-width));y=std::clamp<int>(y,monitor.rcWork.top,std::max(monitor.rcWork.top,monitor.rcWork.bottom-height));
 SetLayeredWindowAttributes(window,0,static_cast<BYTE>(std::clamp(profile->dock.opacity,20u,100u)*255/100),LWA_ALPHA);
 SetWindowPos(window,HWND_TOPMOST,x,y,width,height,SWP_NOACTIVATE|SWP_SHOWWINDOW);
}
void Dock::rebuild(const Profile& profile){
 for(auto button:buttons)DestroyWindow(button);buttons.clear();if(tips)DestroyWindow(tips);tips=nullptr;
 if(font)DeleteObject(font);UINT effective=MulDiv(dpi,profile.dock.scale,100);font=CreateFontW(-MulDiv(12,effective,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 MONITORINFO monitor{sizeof(monitor)};GetMonitorInfoW(MonitorFromWindow(app.target.window,MONITOR_DEFAULTTONEAREST),&monitor);
 int border=2*GetSystemMetricsForDpi(SM_CXBORDER,dpi),availableWidth=std::max<int>(1,monitor.rcWork.right-monitor.rcWork.left-border),availableHeight=std::max<int>(1,monitor.rcWork.bottom-monitor.rcWork.top-border);
 int pad=std::max(1,MulDiv(5,effective,96)),handle=std::max(1,MulDiv(24,effective,96)),height=std::max(1,std::min(MulDiv(38,effective,96),availableHeight-2*pad));
 int buttonWidth=std::max(1,std::min(MulDiv(180,effective,96),availableWidth-handle-pad*3));int maxRight{},maxBottom{};
 auto add=[&](const std::wstring& label,int id,int x,int y,int width,int h){HWND button=child(window,L"BUTTON",label.c_str(),BS_PUSHBUTTON,id,app.instance);MoveWindow(button,x,y,width,h,FALSE);SendMessageW(button,WM_SETFONT,reinterpret_cast<WPARAM>(font),FALSE);SetWindowSubclass(button,buttonProc,1,reinterpret_cast<DWORD_PTR>(this));buttons.push_back(button);maxRight=std::max(maxRight,x+width);maxBottom=std::max(maxBottom,y+h);return button;};
 std::vector<size_t> items;for(auto index:profile.dock.shortcuts)if(index<profile.shortcuts.size())items.push_back(index);
 std::vector<POINT> positions;int x=pad+handle*2+pad,y=pad;
 if(profile.dock.vertical){x=pad+handle;y+=height+pad;}
 for(size_t i=0;i<items.size();++i){if(profile.dock.vertical&&y+height+pad>availableHeight){y=pad;x+=buttonWidth+pad;}else if(!profile.dock.vertical&&x+buttonWidth+pad>availableWidth){x=pad+handle;y+=height+pad;}positions.push_back({x,y});if(profile.dock.vertical)y+=height+pad;else x+=buttonWidth+pad;}
 bool overflow=false;for(auto position:positions)if(position.x+buttonWidth+pad>availableWidth||position.y+height+pad>availableHeight)overflow=true;
 add(profile.dock.collapsed?L"+":L"−",Collapse,pad+handle,pad,handle,height);pageCount=1;
 size_t begin=0,end=items.size();
 if(overflow&&!profile.dock.collapsed){
  int left=pad+handle,top=pad*2+height,columns=std::max(1,(availableWidth-left)/(buttonWidth+pad)),rows=std::max(1,(availableHeight-top)/(height+pad));size_t capacity=static_cast<size_t>(columns)*rows;
  pageCount=(items.size()+capacity-1)/capacity;page=std::min(page,pageCount-1);begin=page*capacity;end=std::min(items.size(),begin+capacity);
  int navX=pad+handle*2+pad,navWidth=std::max(1,std::min(MulDiv(100,effective,96),(availableWidth-navX-2*pad)/2));
  EnableWindow(add(L"Previous",Previous,navX,pad,navWidth,height),page>0);EnableWindow(add(L"Next",Next,navX+navWidth+pad,pad,navWidth,height),page+1<pageCount);
  for(size_t i=begin;i<end;++i){size_t offset=i-begin;int column=static_cast<int>(profile.dock.vertical?offset/rows:offset%columns),row=static_cast<int>(profile.dock.vertical?offset%rows:offset/columns);positions[i]={left+column*(buttonWidth+pad),top+row*(height+pad)};}
 }else page=0;
 if(!profile.dock.collapsed)for(size_t i=begin;i<end;++i){const auto& shortcut=profile.shortcuts[items[i]];auto text=shortcut.dockLabel.empty()?shortcut.title+L"  "+shortcutActionText(shortcut):shortcut.dockLabel;add(text,FirstShortcut+static_cast<int>(items[i]),positions[i].x,positions[i].y,buttonWidth,height);}
 SetWindowLongPtrW(window,GWL_STYLE,GetWindowLongPtrW(window,GWL_STYLE)&~(WS_HSCROLL|WS_VSCROLL));
 SetWindowPos(window,nullptr,0,0,std::min(maxRight+pad,availableWidth)+border,std::min(maxBottom+pad,availableHeight)+border,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);app.theme.style(window);
 tips=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,WS_POPUP|TTS_ALWAYSTIP,0,0,0,0,window,nullptr,app.instance,nullptr);
 for(auto button:buttons){int id=GetDlgCtrlID(button);TOOLINFOW tool{sizeof(tool)};tool.uFlags=TTF_IDISHWND|TTF_SUBCLASS;tool.hwnd=window;tool.uId=reinterpret_cast<UINT_PTR>(button);tool.lpszText=const_cast<wchar_t*>(id==Collapse?L"Expand or collapse dock":id==Previous?L"Previous dock page":id==Next?L"Next dock page":L"Execute shortcut");SendMessageW(tips,TTM_ADDTOOLW,0,reinterpret_cast<LPARAM>(&tool));}
}
void Dock::toggleCollapsed(){auto candidate=app.store;auto* profile=findProfile(candidate,key);if(!profile)return;profile->dock.collapsed=!profile->dock.collapsed;saving=true;bool saved=app.commit(std::move(candidate));saving=false;if(saved&&app.editor)if(auto* draft=findProfile(app.editor->draft,key))if(auto* actual=findProfile(app.store,key))draft->dock=actual->dock;refresh();}
void Dock::savePosition(){RECT rect{},target{};if(!GetWindowRect(window,&rect)||!GetWindowRect(targetWindow,&target))return;auto candidate=app.store;auto* profile=findProfile(candidate,key);if(!profile)return;profile->dock.x=std::clamp(MulDiv(rect.left-target.left,96,dpi),0,32767);profile->dock.y=std::clamp(MulDiv(rect.top-target.top,96,dpi),0,32767);saving=true;bool saved=app.commit(std::move(candidate));saving=false;if(saved&&app.editor)if(auto* draft=findProfile(app.editor->draft,key))if(auto* actual=findProfile(app.store,key))draft->dock=actual->dock;}
void Dock::execute(size_t index){
 if(app.target.window!=targetWindow||app.target.process!=targetProcess||GetForegroundWindow()!=targetWindow||!targetAlive(app.target))return;
 auto* profile=findProfile(app.store,key);if(!profile||index>=profile->shortcuts.size())return;
 if(!profile->website.empty()&&profile->website!=app.target.website)return;
 app.runShortcut(profile->shortcuts[index],profile->website);
}
void Dock::openShortcuts(){
 if(app.target.window!=targetWindow||app.target.process!=targetProcess||GetForegroundWindow()!=targetWindow||!targetAlive(app.target))return;
 auto expected=app.target;
 if(app.settingsVisible){app.closeSettings();if(app.settingsVisible)return;}
 if(app.target.window!=expected.window||app.target.process!=expected.process||!targetAlive(expected))return;
 if(GetForegroundWindow()!=expected.window){DWORD process{};GetWindowThreadProcessId(GetForegroundWindow(),&process);if(process!=GetCurrentProcessId())return;SetForegroundWindow(expected.window);if(GetForegroundWindow()!=expected.window)return;}
 app.workflowsVisible=false;if(app.pendingPopupOpen)return;
 if(app.popup&&IsWindowVisible(app.popup))app.hidePopup(false);
 app.togglePopup();
}
LRESULT CALLBACK Dock::buttonProc(HWND h,UINT m,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR){
 constexpr wchar_t pressed[]=L"QcDockPressed";
 switch(m){
 case WM_MOUSEACTIVATE:return MA_NOACTIVATE;
 case WM_LBUTTONDOWN:
  if(IsWindowEnabled(h)){SetPropW(h,pressed,reinterpret_cast<HANDLE>(1));SetCapture(h);SendMessageW(h,BM_SETSTATE,TRUE,0);}return 0;
 case WM_LBUTTONDBLCLK:return 0;
 case WM_MOUSEMOVE:
  if(GetPropW(h,pressed)&&GetCapture()==h){RECT rect;GetClientRect(h,&rect);POINT point{static_cast<short>(LOWORD(l)),static_cast<short>(HIWORD(l))};SendMessageW(h,BM_SETSTATE,PtInRect(&rect,point),0);}return 0;
 case WM_LBUTTONUP:{
  bool active=GetPropW(h,pressed)&&GetCapture()==h;RECT rect;GetClientRect(h,&rect);POINT point{static_cast<short>(LOWORD(l)),static_cast<short>(HIWORD(l))};bool click=active&&IsWindowEnabled(h)&&PtInRect(&rect,point);
  HWND parent=GetParent(h);int id=GetDlgCtrlID(h);RemovePropW(h,pressed);SendMessageW(h,BM_SETSTATE,FALSE,0);if(GetCapture()==h)ReleaseCapture();
  // The command may rebuild the dock and destroy this button.
  if(click)SendMessageW(parent,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),reinterpret_cast<LPARAM>(h));return 0;
 }
 case WM_CANCELMODE:RemovePropW(h,pressed);SendMessageW(h,BM_SETSTATE,FALSE,0);if(GetCapture()==h)ReleaseCapture();return 0;
 case WM_CAPTURECHANGED:RemovePropW(h,pressed);SendMessageW(h,BM_SETSTATE,FALSE,0);return 0;
 case WM_NCDESTROY:RemovePropW(h,pressed);RemoveWindowSubclass(h,buttonProc,1);break;
 }
 return DefSubclassProc(h,m,w,l);
}
LRESULT CALLBACK Dock::proc(HWND h,UINT m,WPARAM w,LPARAM l){
 auto* dock=reinterpret_cast<Dock*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){dock=static_cast<Dock*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(dock));}if(!dock)return DefWindowProcW(h,m,w,l);
 switch(m){case WM_MOUSEACTIVATE:return MA_NOACTIVATE;
 case WM_CONTEXTMENU:if(reinterpret_cast<HWND>(w)==h){POINT point{static_cast<short>(LOWORD(l)),static_cast<short>(HIWORD(l))};dock->contextMenu(point);}return 0;
 case WM_CANCELMODE:if(dock->menuOpen)EndMenu();dock->dragging=false;if(GetCapture()==h)ReleaseCapture();return 0;
 case WM_COMMAND:if(HIWORD(w)==BN_CLICKED){if(LOWORD(w)==Collapse)dock->toggleCollapsed();else if(LOWORD(w)==Previous||LOWORD(w)==Next){if(LOWORD(w)==Previous&&dock->page>0)--dock->page;else if(LOWORD(w)==Next&&dock->page+1<dock->pageCount)++dock->page;dock->renderKey.clear();dock->refresh();}else if(LOWORD(w)>=FirstShortcut)dock->execute(LOWORD(w)-FirstShortcut);}return 0;
 case WM_LBUTTONDOWN:if(GetForegroundWindow()!=dock->targetWindow)return 0;dock->dragging=true;GetCursorPos(&dock->dragStart);GetWindowRect(h,&dock->dragRect);SetCapture(h);return 0;
 case WM_LBUTTONDBLCLK:dock->dragging=false;if(GetCapture()==h)ReleaseCapture();dock->openShortcuts();return 0;
 case WM_MOUSEMOVE:if(dock->dragging){POINT cursor;GetCursorPos(&cursor);SetWindowPos(h,nullptr,dock->dragRect.left+cursor.x-dock->dragStart.x,dock->dragRect.top+cursor.y-dock->dragStart.y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);}return 0;
 case WM_LBUTTONUP:if(dock->dragging){dock->dragging=false;ReleaseCapture();RECT current{};if(GetWindowRect(h,&current)&&(current.left!=dock->dragRect.left||current.top!=dock->dragRect.top))dock->savePosition();dock->refresh();}return 0;
 case WM_CAPTURECHANGED:dock->dragging=false;return 0;
 case WM_DRAWITEM:dock->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case WM_ERASEBKGND:{RECT rect;GetClientRect(h,&rect);FillRect(reinterpret_cast<HDC>(w),&rect,dock->app.theme.panel);return 1;}
 case WM_PAINT:{PAINTSTRUCT paint;auto dc=BeginPaint(h,&paint);RECT rect;GetClientRect(h,&rect);FillRect(dc,&rect,dock->app.theme.panel);RECT grip{0,0,MulDiv(24,dock->dpi,96),rect.bottom};paintText(dc,L"⠿",grip,dock->app.theme.colors.muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);EndPaint(h,&paint);return 0;}
 }return DefWindowProcW(h,m,w,l);
}
}
