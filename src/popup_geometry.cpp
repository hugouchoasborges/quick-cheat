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
}
