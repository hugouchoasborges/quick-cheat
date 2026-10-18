#include "app.hpp"
#include <algorithm>
namespace qc {
namespace {
constexpr int Search=6;
bool groupStart(App& app,size_t row){
 auto* p=app.currentProfile();return p&&row<app.visibleRows.size()&&(row==0||p->shortcuts[app.visibleRows[row]].category!=p->shortcuts[app.visibleRows[row-1]].category);
}
void executeSelected(App& app){auto selected=SendMessageW(app.rows,LB_GETCURSEL,0,0);if(selected!=LB_ERR)app.execute(static_cast<size_t>(selected));}
LRESULT CALLBACK rowProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_LBUTTONDOWN){auto hit=SendMessageW(h,LB_ITEMFROMPOINT,0,l);if(!HIWORD(hit)){SendMessageW(h,LB_SETCURSEL,LOWORD(hit),0);EnableWindow(GetDlgItem(app->popup,CmdExecute),TRUE);SetFocus(app->search);InvalidateRect(h,nullptr,FALSE);}return 0;}
 if(msg==WM_LBUTTONUP)return 0;
 if(msg==WM_KEYDOWN&&w==VK_RETURN){executeSelected(*app);return 0;}
 if(msg==WM_KEYDOWN&&w==VK_ESCAPE){app->hidePopup();return 0;}
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK searchProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_GETDLGCODE){auto result=DefSubclassProc(h,msg,w,l);if(l){auto* m=reinterpret_cast<MSG*>(l);if(m->wParam==VK_RETURN||m->wParam==VK_ESCAPE||m->wParam==VK_UP||m->wParam==VK_DOWN)return result|DLGC_WANTALLKEYS;}return result;}
 if(msg==WM_KEYDOWN){
 if(w==VK_ESCAPE){app->hidePopup();return 0;}if(w==VK_RETURN){executeSelected(*app);return 0;}
 if(w==VK_OEM_COMMA&&(GetKeyState(VK_CONTROL)&0x8000)){app->openEditor();return 0;}
 if(w==VK_UP||w==VK_DOWN){auto count=SendMessageW(app->rows,LB_GETCOUNT,0,0);auto selected=SendMessageW(app->rows,LB_GETCURSEL,0,0);if(count>0){selected=selected==LB_ERR?0:(std::clamp)(selected+(w==VK_DOWN?1:-1),LRESULT(0),count-1);SendMessageW(app->rows,LB_SETCURSEL,selected,0);EnableWindow(GetDlgItem(app->popup,CmdExecute),TRUE);InvalidateRect(app->rows,nullptr,FALSE);}return 0;}
 }
 if(msg==WM_CHAR&&(w==VK_RETURN||w==VK_ESCAPE))return 0;
 if(msg==WM_PAINT){auto result=DefSubclassProc(h,msg,w,l);if(GetWindowTextLengthW(h)==0){HDC dc=GetDC(h);RECT r;GetClientRect(h,&r);r.left+=8;auto old=SelectObject(dc,app->bold);paintText(dc,L"Search shortcuts...",r,app->theme.colors.muted);SelectObject(dc,old);ReleaseDC(h,dc);}return result;}
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK executeProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_LBUTTONDOWN||(msg==WM_KEYDOWN&&w==VK_SPACE)){app->pressedWindow=app->target.window;app->pressedProcess=app->target.process;app->pressedRow=static_cast<int>(SendMessageW(app->rows,LB_GETCURSEL,0,0));}
 return DefSubclassProc(h,msg,w,l);
}
HFONT makeFont(UINT dpi,int size,int weight){return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");}
}
void App::createPopup(){
 WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=popupProc;cls.lpszClassName=L"QuickCheatPopup";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);RegisterClassW(&cls);
 popup=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat",WS_POPUP|WS_BORDER,0,0,740,480,host,nullptr,instance,this);
 search=control(popup,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Search);SendMessageW(search,EM_SETLIMITTEXT,160,0);SetWindowSubclass(search,searchProc,1,reinterpret_cast<DWORD_PTR>(this));
 heading=control(popup,L"STATIC",L"",SS_RIGHT,1);
 rows=control(popup,L"LISTBOX",L"",WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT|LBS_NOTIFY,2);SetWindowSubclass(rows,rowProc,1,reinterpret_cast<DWORD_PTR>(this));
 empty=control(popup,L"STATIC",L"",SS_CENTER,3);
 control(popup,L"BUTTON",L"Manage shortcuts",BS_PUSHBUTTON|WS_TABSTOP,CmdManage);
 auto executeButton=control(popup,L"BUTTON",L"Execute",BS_PUSHBUTTON|WS_TABSTOP,CmdExecute);SetWindowSubclass(executeButton,executeProc,3,reinterpret_cast<DWORD_PTR>(this));
 control(popup,L"BUTTON",L"Esc",BS_PUSHBUTTON|WS_TABSTOP,CmdExit);
 footer=control(popup,L"STATIC",L"",SS_LEFT,4);theme.style(popup);
}
void App::layoutPopup(){
 if(!popup)return;UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 if(!font||popupDpi!=dpi){HFONT next=makeFont(dpi,14,FW_SEMIBOLD),nextSmall=makeFont(dpi,12,FW_NORMAL),nextBold=makeFont(dpi,19,FW_NORMAL);
 for(HWND c=GetWindow(popup,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 SendMessageW(search,WM_SETFONT,reinterpret_cast<WPARAM>(nextBold),TRUE);SendMessageW(heading,WM_SETFONT,reinterpret_cast<WPARAM>(nextSmall),TRUE);SendMessageW(footer,WM_SETFONT,reinterpret_cast<WPARAM>(nextSmall),TRUE);
 if(font)DeleteObject(font);if(labelFont)DeleteObject(labelFont);if(bold)DeleteObject(bold);font=next;labelFont=nextSmall;bold=nextBold;popupDpi=dpi;}
 bounds(search,46,22,width-260,30,dpi);bounds(heading,width-198,29,172,24,dpi);
 bounds(rows,8,78,width-16,height-146,dpi);bounds(empty,48,158,width-96,104,dpi);
 bounds(GetDlgItem(popup,CmdExit),16,height-44,46,30,dpi);bounds(footer,76,height-38,width-360,24,dpi);
 bounds(GetDlgItem(popup,CmdManage),width-280,height-44,148,30,dpi);bounds(GetDlgItem(popup,CmdExecute),width-120,height-44,104,30,dpi);
 for(size_t i=0;i<visibleRows.size();++i)SendMessageW(rows,LB_SETITEMHEIGHT,i,MulDiv(groupStart(*this,i)?68:44,dpi,96));
 InvalidateRect(popup,nullptr,TRUE);
}
void App::refreshPopup(bool clearSearch){
 if(!popup||updatingPopup)return;updatingPopup=true;pressedRow=-1;if(clearSearch)SetWindowTextW(search,L"");
 auto* profile=currentProfile();auto query=textOf(search);visibleRows.clear();SendMessageW(rows,WM_SETREDRAW,FALSE,0);SendMessageW(rows,LB_RESETCONTENT,0,0);
 SetWindowTextW(heading,(profile?profile->name:target.name.empty()?L"No active application":target.name).c_str());
 if(profile){for(size_t i=0;i<profile->shortcuts.size();++i)if(matchesShortcut(profile->shortcuts[i],query))visibleRows.push_back(i);
 std::stable_sort(visibleRows.begin(),visibleRows.end(),[&](size_t a,size_t b){return _wcsicmp(profile->shortcuts[a].category.c_str(),profile->shortcuts[b].category.c_str())<0;});
 for(auto i:visibleRows)SendMessageW(rows,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(profile->shortcuts[i].title.c_str()));}
 ShowWindow(rows,visibleRows.empty()?SW_HIDE:SW_SHOWNA);ShowWindow(empty,visibleRows.empty()?SW_SHOWNA:SW_HIDE);
 SetWindowTextW(empty,query.empty()?L"No shortcuts for this application yet.\n\nOpen Manage shortcuts to add your first one.":L"No matching shortcuts.\n\nTry another description, category or key combination.");
 if(!visibleRows.empty())SendMessageW(rows,LB_SETCURSEL,0,0);EnableWindow(GetDlgItem(popup,CmdExecute),!visibleRows.empty());
 SendMessageW(rows,WM_SETREDRAW,TRUE,0);SetWindowTextW(footer,(std::to_wstring(visibleRows.size())+(visibleRows.size()==1?L" shortcut":L" shortcuts")+L"  ·  Quick Cheat").c_str());
 updatingPopup=false;layoutPopup();
}
void App::drawRow(DRAWITEMSTRUCT* item){
 if(item->itemID==static_cast<UINT>(-1))return;auto* profile=currentProfile();if(!profile||item->itemID>=visibleRows.size())return;
 const auto& shortcut=profile->shortcuts[visibleRows[item->itemID]];HDC dc=item->hDC;RECT r=item->rcItem;UINT dpi=GetDpiForWindow(popup);auto px=[&](int n){return MulDiv(n,dpi,96);};auto& c=theme.colors;
 FillRect(dc,&r,theme.background);bool group=groupStart(*this,item->itemID);RECT body=r;if(group)body.top+=px(24);body.left+=px(4);body.right-=px(4);body.bottom-=px(3);
 bool selected=(item->itemState&ODS_SELECTED)!=0;if(selected)rounded(dc,body,c.selection,c.selection,px(10));
 auto old=SelectObject(dc,labelFont);
 if(group){RECT label{r.left+px(14),r.top+px(3),r.right-px(14),r.top+px(23)};paintText(dc,shortcut.category.empty()?L"General":shortcut.category,label,c.muted);}
 std::vector<std::wstring> tokens;auto chord=formatChord(shortcut.chord);size_t begin=0;int total=0;
 while(begin<chord.size()){auto end=chord.find(L'+',begin);auto token=chord.substr(begin,end==chord.npos?end:end-begin);SIZE size{};GetTextExtentPoint32W(dc,token.c_str(),static_cast<int>(token.size()),&size);total+=size.cx+px(20);tokens.push_back(std::move(token));if(end==chord.npos)break;begin=end+1;}
 total+=(static_cast<int>(tokens.size())-1)*px(5);int x=r.right-px(16)-total;
 SelectObject(dc,font);RECT titleRect{r.left+px(16),body.top,x-px(16),body.bottom};paintText(dc,shortcut.title,titleRect,c.text);
 SelectObject(dc,labelFont);
 for(const auto& token:tokens){SIZE size{};GetTextExtentPoint32W(dc,token.c_str(),static_cast<int>(token.size()),&size);RECT key{x,body.top+px(6),x+size.cx+px(20),body.bottom-px(6)};rounded(dc,key,c.key,c.border,px(7));paintText(dc,token,key,c.text,DT_CENTER|DT_SINGLELINE|DT_VCENTER);x=key.right+px(5);}
 SelectObject(dc,old);
}
LRESULT CALLBACK App::popupProc(HWND h,UINT msg,WPARAM w,LPARAM l){
 auto* app=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){app=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}
 if(!app)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_MOUSEACTIVATE:{HWND external=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(external,&pid);if(pid!=GetCurrentProcessId()&&external!=app->target.window){app->onForeground(external);SendMessageW(app->rows,LB_SETCURSEL,static_cast<WPARAM>(-1),0);EnableWindow(GetDlgItem(h,CmdExecute),FALSE);}return MA_ACTIVATE;}
 case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect(reinterpret_cast<HDC>(w),&r,app->theme.background);return 1;}
 case WM_PAINT:{PAINTSTRUCT ps;auto dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);FillRect(dc,&r,app->theme.background);UINT dpi=GetDpiForWindow(h);auto px=[&](int n){return MulDiv(n,dpi,96);};HPEN line=CreatePen(PS_SOLID,1,app->theme.colors.border);auto old=SelectObject(dc,line);MoveToEx(dc,0,px(70),nullptr);LineTo(dc,r.right,px(70));MoveToEx(dc,0,r.bottom-px(58),nullptr);LineTo(dc,r.right,r.bottom-px(58));
 HPEN icon=CreatePen(PS_SOLID,1,app->theme.colors.muted);SelectObject(dc,icon);HBRUSH oldBrush=static_cast<HBRUSH>(SelectObject(dc,GetStockObject(NULL_BRUSH)));Ellipse(dc,px(23),px(28),px(35),px(40));MoveToEx(dc,px(33),px(38),nullptr);LineTo(dc,px(40),px(45));SelectObject(dc,oldBrush);SelectObject(dc,old);DeleteObject(icon);DeleteObject(line);EndPaint(h,&ps);return 0;}
 case WM_SIZE:app->layoutPopup();return 0;
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);app->layoutPopup();return 0;}
 case WM_MEASUREITEM:{auto* measure=reinterpret_cast<MEASUREITEMSTRUCT*>(l);measure->itemHeight=MulDiv(groupStart(*app,measure->itemID)?68:44,GetDpiForWindow(h),96);return TRUE;}
 case WM_DRAWITEM:{auto* item=reinterpret_cast<DRAWITEMSTRUCT*>(l);if(item->CtlID==2)app->drawRow(item);else app->theme.drawControl(item);return TRUE;}
 case ExecuteRow:app->execute(static_cast<size_t>(w));return 0;
 case WM_COMMAND:
 if(LOWORD(w)==Search&&HIWORD(w)==EN_CHANGE)app->refreshPopup();
 else if(LOWORD(w)==CmdManage||LOWORD(w)==CmdNew)app->openEditor();
 else if(LOWORD(w)==CmdExit||LOWORD(w)==IDCANCEL)app->hidePopup();
 else if(LOWORD(w)==CmdExecute){int selected=static_cast<int>(SendMessageW(app->rows,LB_GETCURSEL,0,0));bool valid=selected>=0&&selected==app->pressedRow&&app->pressedWindow==app->target.window&&app->pressedProcess==app->target.process;app->pressedRow=-1;if(valid)app->execute(static_cast<size_t>(selected));}
 return 0;
 case WM_CLOSE:app->hidePopup();return 0;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return app->theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 }return DefWindowProcW(h,msg,w,l);
}
}
