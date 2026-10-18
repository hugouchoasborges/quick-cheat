#include "dock.hpp"
#include "app.hpp"
#include <algorithm>
namespace qc {
namespace {
enum {Enabled=710,Orientation,Collapsed,Scale,Opacity,List,Up,Down,Apply,Status};
std::wstring controlText(HWND control){int count=GetWindowTextLengthW(control);std::wstring result(count+1,L'\0');GetWindowTextW(control,result.data(),count+1);result.resize(count);return result;}
}
DockSettings::DockSettings(App& value,HWND owner):app(value),parent(owner){}
DockSettings::~DockSettings(){if(window)DestroyWindow(window);if(font)DeleteObject(font);}
bool DockSettings::show(const std::wstring& next){
 if(key!=next&&!discard())return false;bool reload=key!=next||!changed;key=next;
 if(!window){WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatDockSettings";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);RegisterClassW(&cls);window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Dock settings",WS_CHILD,0,0,500,400,parent,nullptr,app.instance,this);if(!window)return false;}
 if(reload)populate();app.theme.style(window);ShowWindow(window,SW_SHOWNA);return true;
}
void DockSettings::hide(){if(window)ShowWindow(window,SW_HIDE);}
bool DockSettings::discard(){if(!changed)return true;if(MessageBoxW(parent,L"Discard unsaved dock changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)return false;changed=false;return true;}
void DockSettings::mark(){if(!loading){changed=true;SetWindowTextW(status,L"Unsaved changes. Choose Apply dock to save.");}}
void DockSettings::populate(){
 loading=true;order.clear();auto* profile=findProfile(app.store,key);EnableWindow(window,profile!=nullptr);ListView_DeleteAllItems(list);
 if(profile){auto& dock=profile->dock;SendMessageW(enabled,CB_SETCURSEL,dock.enabled?1:0,0);SendMessageW(collapsed,CB_SETCURSEL,dock.collapsed?1:0,0);SendMessageW(orientation,CB_SETCURSEL,dock.vertical?1:0,0);SetWindowTextW(scale,std::to_wstring(dock.scale).c_str());SetWindowTextW(opacity,std::to_wstring(dock.opacity).c_str());
 order=dock.shortcuts;for(size_t i=0;i<profile->shortcuts.size();++i)if(std::find(order.begin(),order.end(),i)==order.end())order.push_back(i);
 for(auto index:order)if(index<profile->shortcuts.size()){auto label=profile->shortcuts[index].title+L"  "+formatChord(profile->shortcuts[index].chord);LVITEMW item{};item.mask=LVIF_TEXT;item.iItem=ListView_GetItemCount(list);item.pszText=label.data();int row=ListView_InsertItem(list,&item);ListView_SetCheckState(list,row,std::find(dock.shortcuts.begin(),dock.shortcuts.end(),index)!=dock.shortcuts.end());}}
 SetWindowTextW(status,profile?L"Check shortcuts, then arrange their order. Drag the dock by its grip.":L"Choose an application profile first.");changed=false;loading=false;
}
void DockSettings::move(int direction){int row=ListView_GetNextItem(list,-1,LVNI_SELECTED);int next=row+direction;if(row<0||next<0||next>=static_cast<int>(order.size()))return;bool checked=ListView_GetCheckState(list,row)!=FALSE,other=ListView_GetCheckState(list,next)!=FALSE;auto* profile=findProfile(app.store,key);if(!profile)return;std::swap(order[row],order[next]);loading=true;for(int current:{row,next}){auto index=order[current];auto label=profile->shortcuts[index].title+L"  "+formatChord(profile->shortcuts[index].chord);ListView_SetItemText(list,current,0,label.data());}ListView_SetCheckState(list,row,other);ListView_SetCheckState(list,next,checked);ListView_SetItemState(list,row,0,LVIS_SELECTED|LVIS_FOCUSED);ListView_SetItemState(list,next,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);ListView_EnsureVisible(list,next,FALSE);loading=false;mark();}
void DockSettings::apply(){
 auto number=[](HWND field,UINT low,UINT high,UINT& result){auto value=controlText(field);if(value.empty()||value.find_first_not_of(L"0123456789")!=value.npos||value.size()>3)return false;result=static_cast<UINT>(std::stoul(value));return result>=low&&result<=high;};
 UINT size{},alpha{};if(!number(scale,50,200,size)||!number(opacity,20,100,alpha)){SetWindowTextW(status,L"Scale must be 50–200%; opacity must be 20–100%.");return;}
 auto candidate=app.store;auto* profile=findProfile(candidate,key);if(!profile)return;auto& dock=profile->dock;dock.enabled=SendMessageW(enabled,CB_GETCURSEL,0,0)==1;dock.vertical=SendMessageW(orientation,CB_GETCURSEL,0,0)==1;dock.collapsed=SendMessageW(collapsed,CB_GETCURSEL,0,0)==1;dock.scale=size;dock.opacity=alpha;dock.shortcuts.clear();
 for(size_t row=0;row<order.size();++row)if(ListView_GetCheckState(list,static_cast<int>(row))&&order[row]<profile->shortcuts.size())dock.shortcuts.push_back(order[row]);
 if(dock.shortcuts.size()>64){SetWindowTextW(status,L"Choose at most 64 dock shortcuts.");return;}if(dock.enabled&&dock.shortcuts.empty()){SetWindowTextW(status,L"Choose at least one shortcut before enabling the dock.");return;}
 if(!app.commit(std::move(candidate)))return;if(app.editor)if(auto* draft=findProfile(app.editor->draft,key))if(auto* saved=findProfile(app.store,key))draft->dock=saved->dock;changed=false;SetWindowTextW(status,L"Dock saved on this computer. Activate the application to show it.");
}
void DockSettings::layout(int x,int y,int width,int height,UINT dpi){
 if(!window)return;MoveWindow(window,x,y,width,height,TRUE);int w=MulDiv(width,96,dpi),h=MulDiv(height,96,dpi);auto move=[&](HWND child,int cx,int cy,int cw,int ch){MoveWindow(child,MulDiv(cx,dpi,96),MulDiv(cy,dpi,96),MulDiv(cw,dpi,96),MulDiv(ch,dpi,96),TRUE);};
 if(font)DeleteObject(font);font=CreateFontW(-MulDiv(14,dpi,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");for(HWND item=GetWindow(window,GW_CHILD);item;item=GetWindow(item,GW_HWNDNEXT))SendMessageW(item,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);
 move(enabled,0,0,230,180);move(collapsed,260,0,250,180);move(GetDlgItem(window,740),0,42,140,24);move(orientation,0,70,220,180);move(GetDlgItem(window,741),260,42,130,24);move(scale,260,70,120,32);move(GetDlgItem(window,742),420,42,160,24);move(opacity,420,70,120,32);
 move(GetDlgItem(window,743),0,118,std::max(100,w),24);move(list,0,150,std::max(180,w-140),std::max(100,h-245));move(GetDlgItem(window,Up),w-120,150,120,34);move(GetDlgItem(window,Down),w-120,196,120,34);move(GetDlgItem(window,Apply),0,h-82,150,36);move(status,0,h-36,w,32);ListView_SetColumnWidth(list,0,std::max(100,MulDiv(w-170,dpi,96)));
}
LRESULT CALLBACK DockSettings::proc(HWND h,UINT m,WPARAM w,LPARAM l){
 auto* self=reinterpret_cast<DockSettings*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){self=static_cast<DockSettings*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}if(!self)return DefWindowProcW(h,m,w,l);
 switch(m){case WM_CREATE:{auto add=[&](const wchar_t* cls,const wchar_t* label,DWORD style,int id){return CreateWindowExW(0,cls,label,WS_CHILD|WS_VISIBLE|style,0,0,1,1,h,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),self->app.instance,nullptr);};self->enabled=add(L"COMBOBOX",L"Dock visibility",CBS_DROPDOWNLIST|WS_TABSTOP,Enabled);SendMessageW(self->enabled,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Dock disabled"));SendMessageW(self->enabled,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Dock enabled"));self->collapsed=add(L"COMBOBOX",L"Initial dock state",CBS_DROPDOWNLIST|WS_TABSTOP,Collapsed);SendMessageW(self->collapsed,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Expanded"));SendMessageW(self->collapsed,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Collapsed"));self->orientation=add(L"COMBOBOX",L"Orientation",CBS_DROPDOWNLIST|WS_TABSTOP,Orientation);SendMessageW(self->orientation,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Horizontal"));SendMessageW(self->orientation,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Vertical"));self->scale=add(L"EDIT",L"100",WS_BORDER|ES_NUMBER|WS_TABSTOP,Scale);self->opacity=add(L"EDIT",L"100",WS_BORDER|ES_NUMBER|WS_TABSTOP,Opacity);SendMessageW(self->scale,EM_SETLIMITTEXT,3,0);SendMessageW(self->opacity,EM_SETLIMITTEXT,3,0);self->list=add(WC_LISTVIEWW,L"Dock shortcuts",LVS_REPORT|LVS_SINGLESEL|LVS_NOCOLUMNHEADER|WS_BORDER|WS_TABSTOP,List);ListView_SetExtendedListViewStyle(self->list,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);LVCOLUMNW column{};column.mask=LVCF_WIDTH;column.cx=400;ListView_InsertColumn(self->list,0,&column);add(L"BUTTON",L"Move up",BS_PUSHBUTTON|WS_TABSTOP,Up);add(L"BUTTON",L"Move down",BS_PUSHBUTTON|WS_TABSTOP,Down);add(L"BUTTON",L"Apply dock",BS_PUSHBUTTON|WS_TABSTOP,Apply);self->status=add(L"STATIC",L"",0,Status);add(L"STATIC",L"Orientation",0,740);add(L"STATIC",L"Scale (%)",0,741);add(L"STATIC",L"Opacity (%)",0,742);add(L"STATIC",L"Dock shortcuts · checked items appear in this order",0,743);return 0;}
 case WM_COMMAND:if(LOWORD(w)==Apply&&HIWORD(w)==BN_CLICKED)self->apply();else if(LOWORD(w)==Up&&HIWORD(w)==BN_CLICKED)self->move(-1);else if(LOWORD(w)==Down&&HIWORD(w)==BN_CLICKED)self->move(1);else if(HIWORD(w)==BN_CLICKED||HIWORD(w)==EN_CHANGE||HIWORD(w)==CBN_SELCHANGE)self->mark();return 0;
 case WM_NOTIFY:if(reinterpret_cast<NMHDR*>(l)->idFrom==List&&reinterpret_cast<NMHDR*>(l)->code==LVN_ITEMCHANGED){auto* changed=reinterpret_cast<NMLISTVIEW*>(l);if((changed->uOldState&LVIS_STATEIMAGEMASK)!=(changed->uNewState&LVIS_STATEIMAGEMASK))self->mark();}return 0;
 case WM_DRAWITEM:self->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return self->app.theme.controlColor(m,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_ERASEBKGND:{RECT rect;GetClientRect(h,&rect);FillRect(reinterpret_cast<HDC>(w),&rect,self->app.theme.background);return 1;}
 }return DefWindowProcW(h,m,w,l);
}
}
