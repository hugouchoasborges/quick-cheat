#include "dock.hpp"
#include "app.hpp"
#include <algorithm>
namespace qc {
namespace {
enum {Enabled=710,Orientation,Collapsed,Scale,Opacity,List,Up,Down,Apply,Status,
 BehaviorTitle=740,BehaviorHelp,CollapsedHelp,AppearanceTitle,OrientationLabel,ScaleLabel,OpacityLabel,RangeHelp,ShortcutsTitle,ShortcutsHelp,Count,Empty};
std::wstring controlText(HWND control){int count=GetWindowTextLengthW(control);std::wstring result(count+1,L'\0');GetWindowTextW(control,result.data(),count+1);result.resize(count);return result;}
}
DockSettings::DockSettings(App& value,HWND owner):app(value),parent(owner){}
DockSettings::~DockSettings(){if(window)DestroyWindow(window);if(font)DeleteObject(font);if(headingFont)DeleteObject(headingFont);if(captionFont)DeleteObject(captionFont);}
bool DockSettings::show(const std::wstring& next){
 if(key!=next&&!discard())return false;bool reload=key!=next||!changed;key=next;
 if(!window){WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatDockSettings";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);RegisterClassW(&cls);window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Dock settings",WS_CHILD,0,0,500,400,parent,nullptr,app.instance,this);if(!window)return false;}
 if(reload)populate();themeChanged();ShowWindow(window,SW_SHOWNA);return true;
}
void DockSettings::hide(){if(window)ShowWindow(window,SW_HIDE);}
void DockSettings::themeChanged(){if(window){app.theme.style(window);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);}}
bool DockSettings::discard(){if(!changed)return true;if(MessageBoxW(parent,L"Discard unsaved dock changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)return false;changed=false;updateActions();return true;}
void DockSettings::mark(){if(!loading){changed=true;SetWindowTextW(status,L"Unsaved changes. Apply to update this application's dock.");updateActions();}}
void DockSettings::updateActions(){
 int count=ListView_GetItemCount(list),checked=0;for(int row=0;row<count;++row)if(ListView_GetCheckState(list,row))++checked;
 auto label=std::to_wstring(checked)+L" / 64 selected";SetWindowTextW(GetDlgItem(window,Count),label.c_str());
 int selected=ListView_GetNextItem(list,-1,LVNI_SELECTED);EnableWindow(GetDlgItem(window,Up),selected>0);EnableWindow(GetDlgItem(window,Down),selected>=0&&selected+1<count);
 EnableWindow(GetDlgItem(window,Apply),changed&&findProfile(app.store,key)!=nullptr);ShowWindow(GetDlgItem(window,Empty),count==0?SW_SHOWNA:SW_HIDE);
}
void DockSettings::populate(){
 loading=true;order.clear();auto* profile=findProfile(app.store,key);EnableWindow(window,profile!=nullptr);ListView_DeleteAllItems(list);
 if(profile){auto& dock=profile->dock;SendMessageW(enabled,BM_SETCHECK,dock.enabled?BST_CHECKED:BST_UNCHECKED,0);SendMessageW(collapsed,BM_SETCHECK,dock.collapsed?BST_CHECKED:BST_UNCHECKED,0);SendMessageW(orientation,CB_SETCURSEL,dock.vertical?1:0,0);SetWindowTextW(scale,std::to_wstring(dock.scale).c_str());SetWindowTextW(opacity,std::to_wstring(dock.opacity).c_str());
 order=dock.shortcuts;for(size_t i=0;i<profile->shortcuts.size();++i)if(std::find(order.begin(),order.end(),i)==order.end())order.push_back(i);
 for(auto index:order)if(index<profile->shortcuts.size()){LVITEMW item{};item.mask=LVIF_TEXT;item.iItem=ListView_GetItemCount(list);item.pszText=profile->shortcuts[index].title.data();int row=ListView_InsertItem(list,&item);auto chord=formatChord(profile->shortcuts[index].chord);ListView_SetItemText(list,row,1,chord.data());ListView_SetCheckState(list,row,std::find(dock.shortcuts.begin(),dock.shortcuts.end(),index)!=dock.shortcuts.end());}}
 SetWindowTextW(status,profile?L"Activate this application to show its dock.":L"Choose an application profile first.");changed=false;loading=false;updateActions();
}
void DockSettings::move(int direction){
 int row=ListView_GetNextItem(list,-1,LVNI_SELECTED),next=row+direction;if(row<0||next<0||next>=static_cast<int>(order.size()))return;
 bool checked=ListView_GetCheckState(list,row)!=FALSE,other=ListView_GetCheckState(list,next)!=FALSE;auto* profile=findProfile(app.store,key);if(!profile)return;std::swap(order[row],order[next]);loading=true;
 for(int current:{row,next}){auto index=order[current];ListView_SetItemText(list,current,0,profile->shortcuts[index].title.data());auto chord=formatChord(profile->shortcuts[index].chord);ListView_SetItemText(list,current,1,chord.data());}
 ListView_SetCheckState(list,row,other);ListView_SetCheckState(list,next,checked);ListView_SetItemState(list,row,0,LVIS_SELECTED|LVIS_FOCUSED);ListView_SetItemState(list,next,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);ListView_EnsureVisible(list,next,FALSE);loading=false;mark();
}
void DockSettings::apply(){
 auto number=[](HWND field,UINT low,UINT high,UINT& result){auto value=controlText(field);if(value.empty()||value.find_first_not_of(L"0123456789")!=value.npos||value.size()>3)return false;result=static_cast<UINT>(std::stoul(value));return result>=low&&result<=high;};
 UINT size{},alpha{};if(!number(scale,50,200,size)||!number(opacity,20,100,alpha)){SetWindowTextW(status,L"Enter a scale of 50–200% and an opacity of 20–100%.");return;}
 auto candidate=app.store;auto* profile=findProfile(candidate,key);if(!profile)return;auto& dock=profile->dock;dock.enabled=SendMessageW(enabled,BM_GETCHECK,0,0)==BST_CHECKED;dock.vertical=SendMessageW(orientation,CB_GETCURSEL,0,0)==1;dock.collapsed=SendMessageW(collapsed,BM_GETCHECK,0,0)==BST_CHECKED;dock.scale=size;dock.opacity=alpha;dock.shortcuts.clear();
 for(size_t row=0;row<order.size();++row)if(ListView_GetCheckState(list,static_cast<int>(row))&&order[row]<profile->shortcuts.size())dock.shortcuts.push_back(order[row]);
 if(dock.shortcuts.size()>64){SetWindowTextW(status,L"Choose up to 64 shortcuts, then apply again.");return;}if(dock.enabled&&dock.shortcuts.empty()){SetWindowTextW(status,L"Check at least one shortcut before enabling the dock.");return;}
 if(!app.commit(std::move(candidate)))return;if(app.editor)if(auto* draft=findProfile(app.editor->draft,key))if(auto* saved=findProfile(app.store,key))draft->dock=saved->dock;changed=false;updateActions();SetWindowTextW(status,L"Dock saved. Activate this application to use it.");
}
void DockSettings::layout(int x,int y,int width,int height,UINT dpi){
 if(!window)return;MoveWindow(window,x,y,width,height,TRUE);int w=MulDiv(width,96,dpi),h=MulDiv(height,96,dpi);auto move=[&](int id,int cx,int cy,int cw,int ch){MoveWindow(GetDlgItem(window,id),MulDiv(cx,dpi,96),MulDiv(cy,dpi,96),MulDiv(cw,dpi,96),MulDiv(ch,dpi,96),TRUE);};
 auto make=[&](int size,int weight){return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");};HFONT next=make(14,FW_NORMAL),nextHeading=make(18,FW_SEMIBOLD),nextCaption=make(12,FW_NORMAL);
 for(HWND item=GetWindow(window,GW_CHILD);item;item=GetWindow(item,GW_HWNDNEXT))SendMessageW(item,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 for(int id:{BehaviorTitle,AppearanceTitle,ShortcutsTitle})SendMessageW(GetDlgItem(window,id),WM_SETFONT,reinterpret_cast<WPARAM>(nextHeading),TRUE);
 for(int id:{BehaviorHelp,CollapsedHelp,RangeHelp,ShortcutsHelp,Count,Status,Empty})SendMessageW(GetDlgItem(window,id),WM_SETFONT,reinterpret_cast<WPARAM>(nextCaption),TRUE);
 if(font)DeleteObject(font);if(headingFont)DeleteObject(headingFont);if(captionFont)DeleteObject(captionFont);font=next;headingFont=nextHeading;captionFont=nextCaption;
 int right=328,rightWidth=std::max(200,w-right),footer=h-70,listBottom=h-116;
 move(BehaviorTitle,0,0,286,28);move(Enabled,0,38,286,30);move(BehaviorHelp,30,72,256,36);move(Collapsed,0,118,286,30);move(CollapsedHelp,30,152,256,36);
 move(AppearanceTitle,0,214,286,28);move(OrientationLabel,0,252,286,22);move(Orientation,0,280,286,180);move(ScaleLabel,0,330,136,22);move(OpacityLabel,150,330,136,22);move(Scale,0,358,136,34);move(Opacity,150,358,136,34);move(RangeHelp,0,406,286,34);
 move(ShortcutsTitle,right,0,rightWidth-150,28);move(Count,w-148,5,148,22);move(ShortcutsHelp,right,36,rightWidth,34);move(List,right,82,rightWidth,std::max(140,listBottom-82));move(Up,right,listBottom+12,120,32);move(Down,right+132,listBottom+12,120,32);move(Empty,right+24,140,rightWidth-48,70);
 move(Apply,0,footer+16,150,36);move(Status,174,footer+16,w-174,38);ListView_SetColumnWidth(list,0,MulDiv(std::max(80,rightWidth-180),dpi,96));ListView_SetColumnWidth(list,1,MulDiv(150,dpi,96));InvalidateRect(window,nullptr,TRUE);
}
LRESULT CALLBACK DockSettings::proc(HWND h,UINT m,WPARAM w,LPARAM l){
 auto* self=reinterpret_cast<DockSettings*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){self=static_cast<DockSettings*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}if(!self)return DefWindowProcW(h,m,w,l);
 switch(m){
 case WM_CREATE:{
 auto add=[&](const wchar_t* cls,const wchar_t* label,DWORD style,int id){return CreateWindowExW(0,cls,label,WS_CHILD|WS_VISIBLE|style,0,0,1,1,h,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),self->app.instance,nullptr);};
 add(L"STATIC",L"Behavior",0,BehaviorTitle);self->enabled=add(L"BUTTON",L"Enable dock for this application",BS_AUTOCHECKBOX|WS_TABSTOP,Enabled);add(L"STATIC",L"Show only while this application is active.",0,BehaviorHelp);self->collapsed=add(L"BUTTON",L"Start collapsed",BS_AUTOCHECKBOX|WS_TABSTOP,Collapsed);add(L"STATIC",L"Keep the dock compact until you expand it.",0,CollapsedHelp);
 add(L"STATIC",L"Appearance",0,AppearanceTitle);add(L"STATIC",L"Orientation",0,OrientationLabel);self->orientation=add(L"COMBOBOX",L"Orientation",CBS_DROPDOWNLIST|WS_TABSTOP,Orientation);SendMessageW(self->orientation,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Horizontal"));SendMessageW(self->orientation,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Vertical"));add(L"STATIC",L"Scale (%)",0,ScaleLabel);self->scale=add(L"EDIT",L"100",WS_BORDER|ES_NUMBER|WS_TABSTOP,Scale);add(L"STATIC",L"Opacity (%)",0,OpacityLabel);self->opacity=add(L"EDIT",L"100",WS_BORDER|ES_NUMBER|WS_TABSTOP,Opacity);SendMessageW(self->scale,EM_SETLIMITTEXT,3,0);SendMessageW(self->opacity,EM_SETLIMITTEXT,3,0);add(L"STATIC",L"Scale 50–200% · Opacity 20–100%\nDrag the dock grip to position it.",0,RangeHelp);
 add(L"STATIC",L"Shortcuts",0,ShortcutsTitle);add(L"STATIC",L"0 / 64 selected",SS_RIGHT,Count);add(L"STATIC",L"Check the shortcuts to show. Arrange them in your preferred order.",0,ShortcutsHelp);self->list=add(WC_LISTVIEWW,L"Dock shortcuts",LVS_REPORT|LVS_SINGLESEL|WS_TABSTOP|WS_BORDER,List);ListView_SetExtendedListViewStyle(self->list,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);LVCOLUMNW column{};column.mask=LVCF_TEXT|LVCF_WIDTH;column.cx=300;column.pszText=const_cast<wchar_t*>(L"Shortcut");ListView_InsertColumn(self->list,0,&column);column.cx=150;column.pszText=const_cast<wchar_t*>(L"Keys");ListView_InsertColumn(self->list,1,&column);add(L"BUTTON",L"Move up",BS_PUSHBUTTON|WS_TABSTOP,Up);add(L"BUTTON",L"Move down",BS_PUSHBUTTON|WS_TABSTOP,Down);add(L"STATIC",L"No shortcuts yet. Add some in the Shortcuts tab, then choose which ones to show here.",0,Empty);add(L"BUTTON",L"Apply dock",BS_PUSHBUTTON|WS_TABSTOP,Apply);self->status=add(L"STATIC",L"",0,Status);return 0;
 }
 case WM_COMMAND:if(LOWORD(w)==Apply&&HIWORD(w)==BN_CLICKED)self->apply();else if(LOWORD(w)==Up&&HIWORD(w)==BN_CLICKED)self->move(-1);else if(LOWORD(w)==Down&&HIWORD(w)==BN_CLICKED)self->move(1);else if(HIWORD(w)==BN_CLICKED||HIWORD(w)==EN_CHANGE||HIWORD(w)==CBN_SELCHANGE)self->mark();return 0;
 case WM_NOTIFY:{auto* header=reinterpret_cast<NMHDR*>(l);if(header->code==NM_CUSTOMDRAW&&(header->idFrom==Enabled||header->idFrom==Collapsed))return self->app.theme.drawCheckbox(reinterpret_cast<NMCUSTOMDRAW*>(l));if(header->idFrom==List&&header->code==LVN_ITEMCHANGED){auto* changed=reinterpret_cast<NMLISTVIEW*>(l);if(!self->loading){if((changed->uOldState&LVIS_STATEIMAGEMASK)!=(changed->uNewState&LVIS_STATEIMAGEMASK))self->mark();else self->updateActions();}}return 0;}
 case WM_DRAWITEM:self->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return self->app.theme.controlColor(m,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_ERASEBKGND:{RECT rect;GetClientRect(h,&rect);FillRect(reinterpret_cast<HDC>(w),&rect,self->app.theme.background);return 1;}
 case WM_PAINT:{PAINTSTRUCT paint;auto dc=BeginPaint(h,&paint);RECT rect;GetClientRect(h,&rect);UINT dpi=GetDpiForWindow(h);int split=MulDiv(306,dpi,96),bottom=rect.bottom-MulDiv(70,dpi,96);HPEN pen=CreatePen(PS_SOLID,1,self->app.theme.colors.border);auto previous=SelectObject(dc,pen);MoveToEx(dc,split,0,nullptr);LineTo(dc,split,bottom-MulDiv(20,dpi,96));MoveToEx(dc,0,bottom,nullptr);LineTo(dc,rect.right,bottom);SelectObject(dc,previous);DeleteObject(pen);EndPaint(h,&paint);return 0;}
 }
 return DefWindowProcW(h,m,w,l);
}
}
