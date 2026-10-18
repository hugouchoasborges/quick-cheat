#include "app.hpp"
#include <algorithm>
namespace qc {
namespace {
LRESULT CALLBACK rowProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value) {
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
 if(msg==WM_LBUTTONDOWN){LRESULT hit=SendMessageW(h,LB_ITEMFROMPOINT,0,l);app->pressedRow=-1;if(!HIWORD(hit)){SendMessageW(h,LB_SETCURSEL,LOWORD(hit),0);app->pressedWindow=app->target.window;app->pressedProcess=app->target.process;app->pressedRow=LOWORD(hit);}return 0;}
 if(msg==WM_LBUTTONUP){LRESULT hit=SendMessageW(h,LB_ITEMFROMPOINT,0,l);bool valid=!HIWORD(hit)&&LOWORD(hit)==app->pressedRow&&app->pressedWindow==app->target.window&&app->pressedProcess==app->target.process;app->pressedRow=-1;if(valid)SendMessageW(app->popup,ExecuteRow,LOWORD(hit),0);return 0;}
 if(msg==WM_KEYDOWN&&w==VK_ESCAPE){ShowWindow(app->popup,SW_HIDE);return 0;}
 return DefSubclassProc(h,msg,w,l);
}
HFONT makeFont(UINT dpi,int size,int weight) { return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI"); }
}
void App::createPopup() {
 WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=popupProc;cls.lpszClassName=L"QuickCheatPopup";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=GetSysColorBrush(COLOR_WINDOW);RegisterClassW(&cls);
 popup=CreateWindowExW(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,cls.lpszClassName,L"Quick Cheat",WS_POPUP|WS_BORDER,0,0,600,480,host,nullptr,instance,this);
 heading=control(popup,L"STATIC",L"",SS_LEFT,1);
 rows=control(popup,L"LISTBOX",L"",WS_VSCROLL|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT|LBS_NOTIFY,2);
 SetWindowSubclass(rows,rowProc,1,reinterpret_cast<DWORD_PTR>(this));
 empty=control(popup,L"STATIC",L"No shortcuts yet.\n\nAdd the shortcuts you want to remember,\nand organize them into categories.",SS_CENTER,3);
 control(popup,L"BUTTON",L"Manage shortcuts",BS_PUSHBUTTON,CmdManage);
 control(popup,L"BUTTON",L"Add shortcut",BS_PUSHBUTTON,CmdNew);
 control(popup,L"BUTTON",L"Close",BS_PUSHBUTTON,CmdExit);
 footer=control(popup,L"STATIC",L"Click a shortcut to run it in the application.",SS_LEFT,4);
}
void App::layoutPopup() {
 if(!popup)return;UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 HFONT next=makeFont(dpi,14,FW_NORMAL),nextSmall=makeFont(dpi,11,FW_NORMAL),nextBold=makeFont(dpi,20,FW_SEMIBOLD);
 for(HWND c=GetWindow(popup,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 SendMessageW(heading,WM_SETFONT,reinterpret_cast<WPARAM>(nextBold),TRUE);
 if(font)DeleteObject(font);if(labelFont)DeleteObject(labelFont);if(bold)DeleteObject(bold);font=next;labelFont=nextSmall;bold=nextBold;
 bounds(heading,22,18,width-44,32,dpi);bounds(rows,16,66,width-32,height-152,dpi);
 bounds(empty,38,128,width-76,130,dpi);
 bounds(footer,22,height-76,width-44,20,dpi);
 bounds(GetDlgItem(popup,CmdManage),20,height-44,156,30,dpi);bounds(GetDlgItem(popup,CmdNew),184,height-44,128,30,dpi);bounds(GetDlgItem(popup,CmdExit),width-94,height-44,74,30,dpi);
 SendMessageW(rows,LB_SETITEMHEIGHT,0,MulDiv(72,dpi,96));InvalidateRect(rows,nullptr,TRUE);
}
void App::refreshPopup() {
 if(!popup)return;auto* profile=currentProfile();visibleRows.clear();SendMessageW(rows,WM_SETREDRAW,FALSE,0);SendMessageW(rows,LB_RESETCONTENT,0,0);
 SetWindowTextW(heading,(profile?profile->name:target.name.empty()?L"No active application":target.name).c_str());
 if(profile){for(size_t i=0;i<profile->shortcuts.size();++i)visibleRows.push_back(i);
 std::stable_sort(visibleRows.begin(),visibleRows.end(),[&](size_t a,size_t b){return _wcsicmp(profile->shortcuts[a].category.c_str(),profile->shortcuts[b].category.c_str())<0;});
 for(auto i:visibleRows)SendMessageW(rows,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(profile->shortcuts[i].title.c_str()));}
 ShowWindow(rows,visibleRows.empty()?SW_HIDE:SW_SHOWNA);ShowWindow(empty,visibleRows.empty()?SW_SHOWNA:SW_HIDE);
 SendMessageW(rows,WM_SETREDRAW,TRUE,0);InvalidateRect(rows,nullptr,TRUE);
 SetWindowTextW(footer,(L"Toggle: "+formatChord(store.trigger)+L"  |  Click a shortcut to run it.").c_str());
}
void App::drawRow(DRAWITEMSTRUCT* item) {
 if(item->itemID==static_cast<UINT>(-1))return;auto* p=currentProfile();if(!p||item->itemID>=visibleRows.size())return;
 const auto& shortcut=p->shortcuts[visibleRows[item->itemID]];HDC dc=item->hDC;RECT r=item->rcItem;UINT dpi=GetDpiForWindow(popup);
 auto px=[&](int n){return MulDiv(n,dpi,96);};bool selected=(item->itemState&ODS_SELECTED)!=0;
 FillRect(dc,&r,GetSysColorBrush(selected?COLOR_HIGHLIGHT:COLOR_WINDOW));SetBkMode(dc,TRANSPARENT);SetTextColor(dc,GetSysColor(selected?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
 HFONT old=static_cast<HFONT>(SelectObject(dc,labelFont));RECT categoryRect{r.left+px(12),r.top+px(8),r.right-px(12),r.top+px(24)};
 const auto category=shortcut.category.empty()?L"General":shortcut.category.c_str();DrawTextW(dc,category,-1,&categoryRect,DT_SINGLELINE|DT_END_ELLIPSIS);
 SelectObject(dc,font);RECT label{r.left+px(12),r.top+px(31),r.left+(r.right-r.left)/2,r.bottom-px(8)};
 DrawTextW(dc,shortcut.title.c_str(),-1,&label,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
 auto chord=formatChord(shortcut.chord);int x=label.right+px(10);size_t begin=0;
 while(begin<chord.size()) {
 auto end=chord.find(L'+',begin);auto token=chord.substr(begin,end==chord.npos?end:end-begin);SIZE extent{};GetTextExtentPoint32W(dc,token.c_str(),static_cast<int>(token.size()),&extent);
 RECT key{x,r.top+px(29),x+extent.cx+px(16),r.top+px(58)};
 if(key.right>r.right-px(6)){RECT remaining{x,r.top+px(31),r.right-px(6),r.bottom-px(8)};DrawTextW(dc,chord.substr(begin).c_str(),-1,&remaining,DT_SINGLELINE|DT_END_ELLIPSIS);break;}
 HBRUSH brush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));FillRect(dc,&key,brush);DeleteObject(brush);FrameRect(dc,&key,GetSysColorBrush(COLOR_3DSHADOW));
 auto previous=SetTextColor(dc,GetSysColor(COLOR_BTNTEXT));DrawTextW(dc,token.c_str(),-1,&key,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SetTextColor(dc,previous);
 x=key.right+px(5);if(end==chord.npos)break;begin=end+1;
 }
 SelectObject(dc,old);
}
LRESULT CALLBACK App::popupProc(HWND h,UINT msg,WPARAM w,LPARAM l) {
 auto* app=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){app=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}
 if(!app)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_MOUSEACTIVATE:return MA_NOACTIVATE;
 case WM_SIZE:app->layoutPopup();return 0;
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);app->layoutPopup();return 0;}
 case WM_MEASUREITEM:reinterpret_cast<MEASUREITEMSTRUCT*>(l)->itemHeight=72;return TRUE;
 case WM_DRAWITEM:app->drawRow(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case ExecuteRow:app->execute(static_cast<size_t>(w));return 0;
 case WM_COMMAND:if(LOWORD(w)==CmdManage||LOWORD(w)==CmdNew)app->openEditor();else if(LOWORD(w)==CmdExit)ShowWindow(h,SW_HIDE);return 0;
 case WM_CLOSE:ShowWindow(h,SW_HIDE);return 0;
 case WM_CTLCOLORSTATIC:SetTextColor(reinterpret_cast<HDC>(w),GetSysColor(COLOR_WINDOWTEXT));SetBkColor(reinterpret_cast<HDC>(w),GetSysColor(COLOR_WINDOW));return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
 }return DefWindowProcW(h,msg,w,l);
}
}
