#include "app.hpp"
#include "dock.hpp"
#include "markdown_view.hpp"
#include <algorithm>
namespace qc {
namespace {
constexpr int Search=6;
void executeSelected(App& app){auto selected=app.selectedShortcut();if(selected>=0)app.execute(static_cast<size_t>(selected));}
LRESULT CALLBACK rowProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_PAINT){auto result=DefSubclassProc(h,msg,w,l);HDC dc=GetDC(h);for(size_t i=0;i<app->shortcutCategories.size();++i){NMLVCUSTOMDRAW draw{};draw.nmcd.hdc=dc;draw.nmcd.dwItemSpec=i;app->drawCategory(&draw);}ReleaseDC(h,dc);return result;}
 if(msg==WM_LBUTTONDOWN||msg==WM_LBUTTONDBLCLK){POINT point{static_cast<short>(LOWORD(l)),static_cast<short>(HIWORD(l))};if(app->clickCategory(point))return 0;}
 if(msg==WM_LBUTTONUP||msg==WM_KEYUP){auto result=DefSubclassProc(h,msg,w,l);app->syncCategoryStates();return result;}
 if(msg==WM_KEYDOWN&&(w==VK_LEFT||w==VK_RIGHT||w==VK_SPACE)){
  for(size_t i=0;i<app->shortcutCategories.size();++i)if(ListView_GetGroupState(h,static_cast<int>(i),LVGS_FOCUSED)&LVGS_FOCUSED){LVGROUP group{sizeof(group)};group.mask=LVGF_STATE;group.stateMask=LVGS_COLLAPSED;bool collapsed=(ListView_GetGroupState(h,static_cast<int>(i),LVGS_COLLAPSED)&LVGS_COLLAPSED)!=0;group.state=(w==VK_LEFT||(w==VK_SPACE&&!collapsed))?LVGS_COLLAPSED:0;ListView_SetGroupInfo(h,static_cast<int>(i),&group);app->syncCategoryStates();return 0;}
 }
 if(msg==WM_LBUTTONDOWN){auto result=DefSubclassProc(h,msg,w,l);app->pressedWindow=app->target.window;app->pressedProcess=app->target.process;app->pressedRow=app->selectedShortcut();app->updateShortcutDetail();SetFocus(app->search);return result;}
 if(msg==WM_KEYDOWN&&w==VK_RETURN){executeSelected(*app);return 0;}
 if(msg==WM_KEYDOWN&&w==VK_ESCAPE){app->hidePopup();return 0;}
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK searchProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_GETDLGCODE){auto result=DefSubclassProc(h,msg,w,l);if(l){auto* m=reinterpret_cast<MSG*>(l);if(m->wParam==VK_RETURN||m->wParam==VK_ESCAPE||m->wParam==VK_UP||m->wParam==VK_DOWN)return result|DLGC_WANTALLKEYS;}return result;}
 if(msg==WM_KEYDOWN){
 if(w==VK_ESCAPE){app->hidePopup();return 0;}if(w==VK_RETURN){if(app->workflowsVisible){SetFocus(app->workflowTree);auto item=TreeView_GetSelection(app->workflowTree);if(item)TreeView_Expand(app->workflowTree,item,TVE_TOGGLE);}else executeSelected(*app);return 0;}
 if(w==VK_OEM_COMMA&&(GetKeyState(VK_CONTROL)&0x8000)){app->openEditor();return 0;}
 if(w==VK_UP||w==VK_DOWN){if(app->workflowsVisible){SetFocus(app->workflowTree);SendMessageW(app->workflowTree,msg,w,l);return 0;}app->moveShortcutSelection(w==VK_DOWN?1:-1);return 0;}
 }
 if(msg==WM_CHAR&&(w==VK_RETURN||w==VK_ESCAPE))return 0;
 if(msg==WM_PAINT){auto result=DefSubclassProc(h,msg,w,l);if(GetWindowTextLengthW(h)==0){HDC dc=GetDC(h);RECT r;GetClientRect(h,&r);r.left+=8;auto old=SelectObject(dc,app->bold);paintText(dc,app->workflowsVisible?L"Search workflows...":L"Search shortcuts...",r,app->theme.colors.muted);SelectObject(dc,old);ReleaseDC(h,dc);}return result;}
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK detailProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);if(msg==WM_GETDLGCODE&&l){auto* key=reinterpret_cast<MSG*>(l);if(key->wParam==VK_ESCAPE||key->wParam==VK_RETURN)return DefSubclassProc(h,msg,w,l)|DLGC_WANTALLKEYS;}
 if(msg==WM_KEYDOWN&&w==VK_ESCAPE){app->hidePopup();return 0;}if(msg==WM_KEYDOWN&&w==VK_RETURN)return 0;return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK executeProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_LBUTTONDOWN||(msg==WM_KEYDOWN&&w==VK_SPACE)){app->pressedWindow=app->target.window;app->pressedProcess=app->target.process;app->pressedRow=app->selectedShortcut();}
 return DefSubclassProc(h,msg,w,l);
}
HFONT makeFont(UINT dpi,int size,int weight){return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");}
}
void App::createPopup(){
 WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=popupProc;cls.lpszClassName=L"QuickCheatPopup";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));RegisterClassW(&cls);
 popup=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat",WS_POPUP|WS_THICKFRAME,0,0,1040,760,host,nullptr,instance,this);
 search=control(popup,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Search);SendMessageW(search,EM_SETLIMITTEXT,160,0);SetWindowSubclass(search,searchProc,1,reinterpret_cast<DWORD_PTR>(this));
 heading=control(popup,L"STATIC",L"",SS_CENTER|SS_CENTERIMAGE|SS_ENDELLIPSIS,1);SetPropW(heading,L"QcHeading",reinterpret_cast<HANDLE>(1));
 rows=control(popup,WC_LISTVIEWW,L"",LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_AUTOARRANGE|WS_TABSTOP,2);ListView_SetView(rows,LV_VIEW_TILE);ListView_SetExtendedListViewStyle(rows,LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);SetWindowSubclass(rows,rowProc,1,reinterpret_cast<DWORD_PTR>(this));
 empty=control(popup,L"STATIC",L"",SS_CENTER,3);
 shortcutDetail=control(popup,L"EDIT",L"",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP,9);SetWindowSubclass(shortcutDetail,detailProc,1,reinterpret_cast<DWORD_PTR>(this));
 control(popup,L"BUTTON",L"Settings",BS_PUSHBUTTON|WS_TABSTOP,CmdManage);
 control(popup,L"BUTTON",L"Back to library",BS_PUSHBUTTON|WS_TABSTOP,CmdBack);
 auto executeButton=control(popup,L"BUTTON",L"Execute",BS_PUSHBUTTON|WS_TABSTOP,CmdExecute);SetWindowSubclass(executeButton,executeProc,3,reinterpret_cast<DWORD_PTR>(this));
 control(popup,L"BUTTON",L"Esc",BS_PUSHBUTTON|WS_TABSTOP,CmdExit);
 control(popup,L"BUTTON",L"Expand",BS_PUSHBUTTON|WS_TABSTOP,CmdExpand);
 control(popup,L"BUTTON",L"Pin popup",BS_PUSHBUTTON|WS_TABSTOP,CmdPin);control(popup,L"BUTTON",L"Close popup",BS_PUSHBUTTON|WS_TABSTOP,CmdPopupClose);
 auto tips=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,popup,nullptr,instance,nullptr);
 for(auto id:{CmdManage,CmdBack,CmdPin,CmdPopupClose}){TOOLINFOW tool{sizeof(tool)};tool.uFlags=TTF_IDISHWND|TTF_SUBCLASS;tool.hwnd=popup;tool.uId=reinterpret_cast<UINT_PTR>(GetDlgItem(popup,id));tool.lpszText=const_cast<wchar_t*>(id==CmdManage?L"Settings":id==CmdBack?L"Back to library":id==CmdPin?L"Pin / unpin popup":L"Close popup (Esc)");SendMessageW(tips,TTM_ADDTOOLW,0,reinterpret_cast<LPARAM>(&tool));}
 footer=control(popup,L"STATIC",L"",SS_LEFT,4);createWorkflowPopup();theme.style(popup);layoutPopup();
}
void App::layoutPopup(){
 if(!popup)return;UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 if(!font||popupDpi!=dpi){HFONT next=makeFont(dpi,14,FW_SEMIBOLD),nextSmall=makeFont(dpi,12,FW_NORMAL),nextBold=makeFont(dpi,19,FW_NORMAL);
 for(HWND c=GetWindow(popup,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 SendMessageW(search,WM_SETFONT,reinterpret_cast<WPARAM>(nextBold),TRUE);SendMessageW(heading,WM_SETFONT,reinterpret_cast<WPARAM>(nextBold),TRUE);SendMessageW(footer,WM_SETFONT,reinterpret_cast<WPARAM>(nextSmall),TRUE);
 if(font)DeleteObject(font);if(labelFont)DeleteObject(labelFont);if(bold)DeleteObject(bold);font=next;labelFont=nextSmall;bold=nextBold;popupDpi=dpi;}
 bounds(heading,148,14,width-296,32,dpi);bounds(search,46,62,width-62,30,dpi);
 bounds(GetDlgItem(popup,CmdManage),width-(popupPinned?140:96),12,36,36,dpi);bounds(GetDlgItem(popup,CmdBack),16,12,36,36,dpi);ShowWindow(GetDlgItem(popup,CmdBack),settingsVisible?SW_SHOWNA:SW_HIDE);
 SetPropW(GetDlgItem(popup,CmdManage),L"QcSelected",reinterpret_cast<HANDLE>(settingsVisible));
 bounds(GetDlgItem(popup,CmdPin),width-(popupPinned?96:52),12,36,36,dpi);bounds(GetDlgItem(popup,CmdPopupClose),width-52,12,36,36,dpi);ShowWindow(GetDlgItem(popup,CmdPopupClose),popupPinned?SW_SHOWNA:SW_HIDE);
 SetWindowTextW(GetDlgItem(popup,CmdPin),popupPinned?L"Unpin popup":L"Pin popup");SetPropW(GetDlgItem(popup,CmdPin),L"QcSelected",reinterpret_cast<HANDLE>(popupPinned));
 bounds(rows,8,118,width-16,height-186,dpi);bounds(empty,48,198,width-96,104,dpi);
 bounds(GetDlgItem(popup,CmdExit),16,height-44,46,30,dpi);bounds(footer,76,height-38,width-488,24,dpi);
 bounds(GetDlgItem(popup,CmdExpand),width-228,height-44,96,30,dpi);
 bounds(GetDlgItem(popup,CmdExecute),width-120,height-44,104,30,dpi);
 layoutWorkflowPopup();
 layoutShortcutTiles();
 updateShortcutDetail();
 if(settingsVisible){
  for(HWND child=GetWindow(popup,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)){int id=GetDlgCtrlID(child);if(child!=heading&&id!=CmdManage&&id!=CmdBack&&id!=CmdPin&&id!=CmdPopupClose&&(!editor||child!=editor->window))ShowWindow(child,SW_HIDE);}
  SetWindowTextW(heading,L"Settings");if(editor&&editor->window){bounds(editor->window,0,54,width,height-54,dpi);ShowWindow(editor->window,SW_SHOWNA);editor->layout();}
 }else{
  ShowWindow(search,SW_SHOWNA);ShowWindow(heading,SW_SHOWNA);ShowWindow(footer,SW_SHOWNA);ShowWindow(GetDlgItem(popup,CmdExit),SW_SHOWNA);ShowWindow(GetDlgItem(popup,CmdExpand),SW_SHOWNA);
 }
 if(dock)dock->refresh();InvalidateRect(popup,nullptr,TRUE);
}
void App::refreshPopup(bool clearSearch){
 if(!popup||updatingPopup)return;syncCategoryStates();updatingPopup=true;pressedRow=-1;if(clearSearch)SetWindowTextW(search,L"");
 auto* profile=currentProfile();auto query=textOf(search);visibleRows.clear();SendMessageW(rows,WM_SETREDRAW,FALSE,0);ListView_DeleteAllItems(rows);ListView_RemoveAllGroups(rows);
 SetWindowTextW(heading,contextName().c_str());
 if(profile){for(size_t i=0;i<profile->shortcuts.size();++i)if(matchesShortcut(profile->shortcuts[i],query))visibleRows.push_back(i);
 std::stable_sort(visibleRows.begin(),visibleRows.end(),[&](size_t a,size_t b){return _wcsicmp(profile->shortcuts[a].category.c_str(),profile->shortcuts[b].category.c_str())<0;});
 }populateShortcutTiles();
 ShowWindow(rows,visibleRows.empty()?SW_HIDE:SW_SHOWNA);ShowWindow(empty,visibleRows.empty()?SW_SHOWNA:SW_HIDE);
 SetWindowTextW(empty,query.empty()?L"No shortcuts for this application yet.\n\nOpen Settings > Shortcuts to add your first one.":L"No matching shortcuts.\n\nTry another description, category or key combination.");
 if(!visibleRows.empty())moveShortcutSelection(1);EnableWindow(GetDlgItem(popup,CmdExecute),selectedShortcut()>=0);
 SendMessageW(rows,WM_SETREDRAW,TRUE,0);SetWindowTextW(footer,(std::to_wstring(visibleRows.size())+(visibleRows.size()==1?L" shortcut":L" shortcuts")+L"  ·  Quick Cheat").c_str());
 refreshWorkflowPopup();updatingPopup=false;layoutPopup();
}
LRESULT CALLBACK App::popupProc(HWND h,UINT msg,WPARAM w,LPARAM l){
 auto* app=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){app=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}
 if(!app)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_MOUSEACTIVATE:{HWND external=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(external,&pid);if(pid!=GetCurrentProcessId()&&external!=app->target.window){auto previous=app->target.window;auto process=app->target.process;app->onForeground(external);if(previous!=app->target.window||process!=app->target.process){ListView_SetItemState(app->rows,-1,0,LVIS_SELECTED|LVIS_FOCUSED);EnableWindow(GetDlgItem(h,CmdExecute),FALSE);}}return MA_ACTIVATE;}
 case WM_ACTIVATE:if(LOWORD(w)==WA_INACTIVE&&!app->popupPinned){DWORD process{};GetWindowThreadProcessId(reinterpret_cast<HWND>(l),&process);if(process!=GetCurrentProcessId())app->hidePopup(false);}return 0;
 case WM_NCHITTEST:{auto hit=DefWindowProcW(h,msg,w,l);if(hit!=HTCLIENT)return hit;POINT point{static_cast<short>(LOWORD(l)),static_cast<short>(HIWORD(l))};ScreenToClient(h,&point);auto child=ChildWindowFromPointEx(h,point,CWP_SKIPINVISIBLE|CWP_SKIPDISABLED|CWP_SKIPTRANSPARENT);if(point.y<MulDiv(54,GetDpiForWindow(h),96)&&(child==h||child==app->heading))return HTCAPTION;return HTCLIENT;}
 case WM_NCLBUTTONDBLCLK:if(w==HTCAPTION){app->togglePopupSize();return 0;}break;
 case WM_GETMINMAXINFO:{auto* limits=reinterpret_cast<MINMAXINFO*>(l);MONITORINFO monitor{sizeof(monitor)};if(GetMonitorInfoW(MonitorFromWindow(h,MONITOR_DEFAULTTONEAREST),&monitor)){UINT dpi=GetDpiForWindow(h);LONG width=monitor.rcWork.right-monitor.rcWork.left,height=monitor.rcWork.bottom-monitor.rcWork.top;limits->ptMinTrackSize={(std::min<LONG>)(MulDiv(720,dpi,96),width),(std::min<LONG>)(MulDiv(480,dpi,96),height)};limits->ptMaxTrackSize={width,height};}return 0;}
 case WM_SIZING:app->popupSizing=true;break;
 case WM_EXITSIZEMOVE:if(app->popupSizing){app->rememberPopupSize();app->popupSizing=false;}return 0;
 case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect(reinterpret_cast<HDC>(w),&r,app->theme.background);return 1;}
 case WM_PAINT:{PAINTSTRUCT ps;auto dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);FillRect(dc,&r,app->theme.background);UINT dpi=GetDpiForWindow(h);auto px=[&](int n){return MulDiv(n,dpi,96);};HPEN line=CreatePen(PS_SOLID,1,app->theme.colors.border);auto old=SelectObject(dc,line);MoveToEx(dc,0,px(110),nullptr);LineTo(dc,r.right,px(110));MoveToEx(dc,0,r.bottom-px(58),nullptr);LineTo(dc,r.right,r.bottom-px(58));
 HPEN icon=CreatePen(PS_SOLID,1,app->theme.colors.muted);SelectObject(dc,icon);HBRUSH oldBrush=static_cast<HBRUSH>(SelectObject(dc,GetStockObject(NULL_BRUSH)));Ellipse(dc,px(23),px(68),px(35),px(80));MoveToEx(dc,px(33),px(78),nullptr);LineTo(dc,px(40),px(85));SelectObject(dc,oldBrush);SelectObject(dc,old);DeleteObject(icon);DeleteObject(line);EndPaint(h,&ps);return 0;}
 case WM_SIZE:app->layoutPopup();return 0;
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);app->layoutPopup();return 0;}
 case WM_DRAWITEM:app->theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case WM_NOTIFY:{auto* notice=reinterpret_cast<NMHDR*>(l);if(notice->hwndFrom==app->workflowDetail&&notice->code==EN_LINK)return handleMarkdownLink(reinterpret_cast<ENLINK*>(l))?1:0;if(notice->hwndFrom==app->workflowTree&&notice->code==TVN_SELCHANGEDW)app->selectWorkflowStep();
 if(notice->hwndFrom==app->rows){if(notice->code==NM_CUSTOMDRAW){auto* draw=reinterpret_cast<NMLVCUSTOMDRAW*>(l);if(draw->nmcd.dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;if(draw->nmcd.dwDrawStage==CDDS_ITEMPREPAINT&&draw->dwItemType==LVCDI_GROUP){app->drawCategory(draw);return CDRF_SKIPDEFAULT;}if(draw->nmcd.dwDrawStage==CDDS_ITEMPREPAINT&&draw->dwItemType==LVCDI_ITEM){DRAWITEMSTRUCT item{};item.itemID=static_cast<UINT>(draw->nmcd.dwItemSpec);item.hDC=draw->nmcd.hdc;ListView_GetItemRect(app->rows,item.itemID,&item.rcItem,LVIR_BOUNDS);if(ListView_GetItemState(app->rows,item.itemID,LVIS_SELECTED))item.itemState=ODS_SELECTED;app->drawRow(&item);return CDRF_SKIPDEFAULT;}return CDRF_DODEFAULT;}
 if(notice->code==LVN_ITEMCHANGED&&!app->updatingPopup)app->updateShortcutDetail();
 if(notice->code==NM_DBLCLK){int index=reinterpret_cast<NMITEMACTIVATE*>(l)->iItem;bool valid=index>=0&&index==app->pressedRow&&app->pressedWindow==app->target.window&&app->pressedProcess==app->target.process;app->pressedRow=-1;if(valid)app->execute(static_cast<size_t>(index));}}
 return 0;}
 case ExecuteRow:app->execute(static_cast<size_t>(w));return 0;
 case WM_COMMAND:
 if(LOWORD(w)==Search&&HIWORD(w)==EN_CHANGE)app->refreshPopup();
 else if(LOWORD(w)==CmdPin){app->popupPinned=!app->popupPinned;app->layoutPopup();}
 else if(LOWORD(w)==CmdPopupClose)app->hidePopup();
 else if(LOWORD(w)==CmdExpand)app->togglePopupSize();
 else if(LOWORD(w)==CmdShortcuts||LOWORD(w)==CmdWorkflows){app->workflowsVisible=LOWORD(w)==CmdWorkflows;app->refreshPopup(true);SetFocus(app->search);}
 else if(LOWORD(w)==CmdManage){if(app->settingsVisible)app->closeSettings();else app->openEditor(0);}
 else if(LOWORD(w)==CmdBack)app->closeSettings();
 else if(LOWORD(w)==CmdNew)app->openEditor(1);
 else if(LOWORD(w)==IDOK&&app->settingsVisible&&app->editor)SendMessageW(app->editor->window,WM_COMMAND,IDOK,0);
 else if(LOWORD(w)==CmdExit||LOWORD(w)==IDCANCEL)app->hidePopup();
 else if(LOWORD(w)==CmdExecute){int selected=app->selectedShortcut();bool valid=selected>=0&&selected==app->pressedRow&&app->pressedWindow==app->target.window&&app->pressedProcess==app->target.process;app->pressedRow=-1;if(valid)app->execute(static_cast<size_t>(selected));}
 return 0;
 case WM_CLOSE:app->hidePopup();return 0;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return app->theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 }return DefWindowProcW(h,msg,w,l);
}
}
