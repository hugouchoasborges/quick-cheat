#include "app.hpp"
#include <algorithm>
namespace qc {
void App::positionPopup(bool opening){
 HWND anchor=opening?target.window:popup;MONITORINFO monitor{sizeof(monitor)};if(!GetMonitorInfoW(MonitorFromWindow(anchor,MONITOR_DEFAULTTONEAREST),&monitor))return;
 UINT dpi=GetDpiForWindow(anchor);if(!dpi)dpi=96;
 int availableWidth=monitor.rcWork.right-monitor.rcWork.left,availableHeight=monitor.rcWork.bottom-monitor.rcWork.top;
 int margin=popupExpanded?0:MulDiv(24,dpi,96);
 int width=popupExpanded?availableWidth:(std::min)(MulDiv(popupSize.cx,dpi,96),(std::max)(availableWidth-2*margin,1));
 int height=popupExpanded?availableHeight:(std::min)(MulDiv(popupSize.cy,dpi,96),(std::max)(availableHeight-2*margin,1));
 SetWindowPos(popup,HWND_TOPMOST,monitor.rcWork.left+(availableWidth-width)/2,monitor.rcWork.top+(availableHeight-height)/2,width,height,SWP_NOACTIVATE|(opening?SWP_SHOWWINDOW:0));
 SetWindowTextW(GetDlgItem(popup,CmdExpand),popupExpanded?L"Restore":L"Expand");
}
void App::rememberPopupSize(){
 RECT r;GetWindowRect(popup,&r);UINT dpi=GetDpiForWindow(popup);if(!dpi)dpi=96;
 popupSize={MulDiv(r.right-r.left,96,dpi),MulDiv(r.bottom-r.top,96,dpi)};popupExpanded=false;SetWindowTextW(GetDlgItem(popup,CmdExpand),L"Expand");
}
void App::togglePopupSize(){
 if(!popupExpanded)rememberPopupSize();popupExpanded=!popupExpanded;positionPopup(false);layoutPopup();
}
void App::measurePopupRows(){
 if(!rows||workflowsVisible)return;HDC dc=GetDC(rows);UINT dpi=GetDpiForWindow(popup);auto px=[&](int n){return MulDiv(n,dpi,96);};RECT area;GetClientRect(rows,&area);area.right-=GetSystemMetricsForDpi(SM_CXVSCROLL,dpi);auto* profile=currentProfile();auto original=SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));
 for(size_t row=0;profile&&row<visibleRows.size();++row){const auto& shortcut=profile->shortcuts[visibleRows[row]];auto chord=formatChord(shortcut.chord);int keysWidth=0;size_t begin=0;SelectObject(dc,labelFont?labelFont:GetStockObject(DEFAULT_GUI_FONT));
 while(begin<chord.size()){auto end=chord.find(L'+',begin);auto token=chord.substr(begin,end==chord.npos?end:end-begin);SIZE size{};GetTextExtentPoint32W(dc,token.c_str(),static_cast<int>(token.size()),&size);keysWidth+=size.cx+px(25);if(end==chord.npos)break;begin=end+1;}
 SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));RECT text{0,0,(std::max<LONG>)(area.right-keysWidth-px(48),px(80)),0};DrawTextW(dc,shortcut.title.c_str(),-1,&text,DT_CALCRECT|DT_WORDBREAK|DT_NOPREFIX);
 bool group=row==0||profile->shortcuts[visibleRows[row-1]].category!=shortcut.category;int height=(std::max<LONG>)(px(44),text.bottom+px(23))+(group?px(24):0);
 // Native listbox rows are limited to 255 physical pixels. The detail field
 // exposes the entire description when a row reaches that limit.
 if(SendMessageW(rows,LB_SETITEMHEIGHT,row,(std::min)(height,255))==LB_ERR)break;
 }
 SelectObject(dc,original);ReleaseDC(rows,dc);
}
void App::updateShortcutDetail(){
 if(!shortcutDetail)return;auto selected=SendMessageW(rows,LB_GETCURSEL,0,0);auto* profile=currentProfile();
 bool show=!workflowsVisible&&profile&&selected!=LB_ERR&&static_cast<size_t>(selected)<visibleRows.size()&&SendMessageW(rows,LB_GETITEMHEIGHT,selected,0)>=255;
 ShowWindow(shortcutDetail,show?SW_SHOWNA:SW_HIDE);
 UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);int top=profile&&!profile->workflows.empty()?112:78;
 if(workflowsVisible)return;
 bounds(rows,8,top,width-16,(std::max)(height-top-(show?188:68),60),dpi);
 if(!show)return;
 const auto& shortcut=profile->shortcuts[visibleRows[static_cast<size_t>(selected)]];auto text=shortcut.title+L"\r\n"+shortcut.category+L" · "+formatChord(shortcut.chord);
 if(textOf(shortcutDetail)!=text){SetWindowTextW(shortcutDetail,text.c_str());SendMessageW(shortcutDetail,EM_SETSEL,0,0);SendMessageW(shortcutDetail,EM_SCROLLCARET,0,0);}
 bounds(shortcutDetail,16,height-176,width-32,104,dpi);
}
}
