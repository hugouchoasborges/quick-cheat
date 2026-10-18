#include "theme.hpp"
#include <dwmapi.h>
#include <uxtheme.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.ViewManagement.h>
namespace qc {
struct SystemThemeObserver {
 winrt::Windows::UI::ViewManagement::UISettings settings{nullptr};winrt::event_token token{};
 ~SystemThemeObserver(){if(settings&&token.value)try{settings.ColorValuesChanged(token);}catch(...){} }
};
void rounded(HDC dc,RECT rect,COLORREF fill,COLORREF border,int radius) {
 HBRUSH brush=CreateSolidBrush(fill);HPEN pen=CreatePen(PS_SOLID,1,border);auto oldBrush=SelectObject(dc,brush);auto oldPen=SelectObject(dc,pen);
 RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,radius,radius);SelectObject(dc,oldPen);SelectObject(dc,oldBrush);DeleteObject(pen);DeleteObject(brush);
}
void paintText(HDC dc,const std::wstring& text,RECT rect,COLORREF color,UINT flags){SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);DrawTextW(dc,text.c_str(),-1,&rect,flags);}
Theme::Theme(){resolve(ThemeMode::System);}
Theme::~Theme(){observer.reset();if(background)DeleteObject(background);if(field)DeleteObject(field);if(panel)DeleteObject(panel);}
void Theme::stop(){observer.reset();}
void Theme::observe(HWND host){
 try{observer=std::make_unique<SystemThemeObserver>();observer->settings=winrt::Windows::UI::ViewManagement::UISettings();observer->token=observer->settings.ColorValuesChanged([host](auto const&,auto const&){PostMessageW(host,ThemeChanged,0,0);});}catch(...){observer.reset();}
}
void Theme::resolve(ThemeMode mode){
 HIGHCONTRASTW contrast{sizeof(contrast)};highContrast=SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)&&(contrast.dwFlags&HCF_HIGHCONTRASTON);
 dark=mode==ThemeMode::Dark;
 if(mode==ThemeMode::System){try{if(observer&&observer->settings){auto c=observer->settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Foreground);dark=5*c.G+2*c.R+c.B>8*128;}else{auto c=GetSysColor(COLOR_WINDOW);dark=GetRValue(c)+GetGValue(c)+GetBValue(c)<384;}}catch(...){dark=false;}}
 if(highContrast){colors={GetSysColor(COLOR_WINDOW),GetSysColor(COLOR_WINDOW),GetSysColor(COLOR_WINDOW),GetSysColor(COLOR_HIGHLIGHT),GetSysColor(COLOR_WINDOWTEXT),GetSysColor(COLOR_WINDOWTEXT),GetSysColor(COLOR_WINDOWTEXT),GetSysColor(COLOR_HIGHLIGHT),GetSysColor(COLOR_HIGHLIGHTTEXT),GetSysColor(COLOR_WINDOW)};}
 else if(dark){colors={RGB(30,30,30),RGB(37,37,37),RGB(41,41,41),RGB(54,54,54),RGB(68,68,68),RGB(244,244,244),RGB(180,180,180),RGB(206,180,255),RGB(30,22,43),RGB(45,45,45)};}
 else{colors={RGB(250,249,247),RGB(241,240,237),RGB(255,255,255),RGB(230,227,237),RGB(212,208,204),RGB(34,33,38),RGB(98,95,104),RGB(105,65,165),RGB(255,255,255),RGB(241,240,237)};}
 if(background)DeleteObject(background);if(field)DeleteObject(field);if(panel)DeleteObject(panel);
 background=CreateSolidBrush(colors.background);field=CreateSolidBrush(colors.field);panel=CreateSolidBrush(colors.panel);
}
void Theme::style(HWND window){
 if(!window)return;
 BOOL useDark=dark&&!highContrast;DwmSetWindowAttribute(window,DWMWA_USE_IMMERSIVE_DARK_MODE,&useDark,sizeof(useDark));
 DWM_WINDOW_CORNER_PREFERENCE corner=DWMWCP_ROUND;DwmSetWindowAttribute(window,DWMWA_WINDOW_CORNER_PREFERENCE,&corner,sizeof(corner));DwmSetWindowAttribute(window,DWMWA_BORDER_COLOR,&colors.border,sizeof(colors.border));
 DwmSetWindowAttribute(window,DWMWA_CAPTION_COLOR,&colors.background,sizeof(colors.background));DwmSetWindowAttribute(window,DWMWA_TEXT_COLOR,&colors.text,sizeof(colors.text));
 for(HWND child=GetWindow(window,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)){
 wchar_t cls[64]{};GetClassNameW(child,cls,64);SetWindowSubclass(child,widgetProc,91,reinterpret_cast<DWORD_PTR>(this));
 if(wcscmp(cls,L"Button")==0){SetWindowLongPtrW(child,GWL_STYLE,(GetWindowLongPtrW(child,GWL_STYLE)&~BS_TYPEMASK)|BS_OWNERDRAW);}
 if(wcscmp(cls,L"Edit")==0){SetWindowTheme(child,L"",L"");SetWindowLongPtrW(child,GWL_EXSTYLE,GetWindowLongPtrW(child,GWL_EXSTYLE)&~WS_EX_CLIENTEDGE);SendMessageW(child,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,MAKELPARAM(8,8));}
 if(wcscmp(cls,L"ComboBox")==0){SetWindowTheme(child,L"",L"");}
 if(wcscmp(cls,WC_LISTVIEWW)==0){ListView_SetBkColor(child,colors.background);ListView_SetTextBkColor(child,colors.background);ListView_SetTextColor(child,colors.text);auto header=ListView_GetHeader(child);if(header){SetWindowTheme(header,L"",L"");SetWindowSubclass(header,headerProc,92,reinterpret_cast<DWORD_PTR>(this));}}
 }
 RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_FRAME);
}
LRESULT Theme::controlColor(UINT message,HDC dc,HWND widget){
 wchar_t cls[64]{};GetClassNameW(widget,cls,64);bool editable=message==WM_CTLCOLOREDIT||message==WM_CTLCOLORLISTBOX||wcscmp(cls,L"Edit")==0||wcscmp(cls,L"ComboBox")==0;
 bool caption=wcscmp(cls,L"Static")==0&&GetDlgCtrlID(widget)!=306;
 SetTextColor(dc,!IsWindowEnabled(widget)||caption?colors.muted:colors.text);SetBkMode(dc,editable?OPAQUE:TRANSPARENT);SetBkColor(dc,editable?colors.field:colors.background);
 if(wcscmp(cls,L"ListBox")==0||GetDlgCtrlID(widget)==6){SetBkColor(dc,colors.background);return reinterpret_cast<LRESULT>(background);}
 return reinterpret_cast<LRESULT>(editable?field:GetStockObject(NULL_BRUSH));
}
bool Theme::drawControl(DRAWITEMSTRUCT* item){
 if(item->CtlType!=ODT_BUTTON)return false;auto dc=item->hDC;RECT r=item->rcItem;bool enabled=IsWindowEnabled(item->hwndItem)!=FALSE;
 bool primary=item->CtlID==105||item->CtlID==208||item->CtlID==405||GetPropW(item->hwndItem,L"QcSelected")!=nullptr;bool pressed=(item->itemState&ODS_SELECTED)!=0;bool hover=GetPropW(item->hwndItem,L"QcHover")!=nullptr;
 auto fill=primary&&enabled?colors.accent:pressed?colors.selection:hover?colors.selection:colors.panel;
 auto text=primary&&enabled?colors.accentText:enabled?colors.text:colors.muted;
 bool onPanel=item->CtlID==206||item->CtlID==207||item->CtlID==208||item->CtlID==209;
 FillRect(dc,&r,onPanel?panel:background);
 rounded(dc,r,fill,primary&&enabled?colors.accent:colors.border,MulDiv(10,GetDpiForWindow(item->hwndItem),96));
 auto old=SelectObject(dc,reinterpret_cast<HFONT>(SendMessageW(item->hwndItem,WM_GETFONT,0,0)));wchar_t label[160]{};GetWindowTextW(item->hwndItem,label,160);
 paintText(dc,label,r,text,DT_SINGLELINE|DT_VCENTER|DT_CENTER);SelectObject(dc,old);
 if(item->itemState&ODS_FOCUS){InflateRect(&r,-3,-3);DrawFocusRect(dc,&r);}return true;
}
LRESULT CALLBACK Theme::widgetProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* theme=reinterpret_cast<Theme*>(value);wchar_t cls[64]{};GetClassNameW(h,cls,64);bool combo=wcscmp(cls,L"ComboBox")==0;
 if(msg==WM_MOUSEMOVE){if(!GetPropW(h,L"QcHover")){SetPropW(h,L"QcHover",reinterpret_cast<HANDLE>(1));TRACKMOUSEEVENT track{sizeof(track),TME_LEAVE,h,0};TrackMouseEvent(&track);InvalidateRect(h,nullptr,FALSE);}}
 if(msg==WM_MOUSELEAVE){RemovePropW(h,L"QcHover");InvalidateRect(h,nullptr,FALSE);}
 if(msg==WM_SETFOCUS||msg==WM_KILLFOCUS)RedrawWindow(h,nullptr,nullptr,RDW_INVALIDATE|RDW_FRAME);
 if(combo&&(msg==WM_CTLCOLOREDIT||msg==WM_CTLCOLORLISTBOX))return theme->controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 if(combo&&msg==WM_PAINT){
 PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);auto& c=theme->colors;rounded(dc,r,c.field,GetFocus()==h?c.accent:c.border,8);
 int arrow=MulDiv(28,GetDpiForWindow(h),96);RECT label=r;label.left+=10;label.right-=arrow;
 if((GetWindowLongPtrW(h,GWL_STYLE)&3)==CBS_DROPDOWNLIST){int length=GetWindowTextLengthW(h);std::wstring text(static_cast<size_t>(length)+1,L'\0');GetWindowTextW(h,text.data(),length+1);text.resize(length);auto old=SelectObject(dc,reinterpret_cast<HFONT>(SendMessageW(h,WM_GETFONT,0,0)));paintText(dc,text,label,c.text);SelectObject(dc,old);}
 int x=r.right-arrow/2,y=(r.bottom+r.top)/2;HPEN pen=CreatePen(PS_SOLID,1,c.muted);auto old=SelectObject(dc,pen);MoveToEx(dc,x-4,y-2,nullptr);LineTo(dc,x,y+2);LineTo(dc,x+4,y-2);SelectObject(dc,old);DeleteObject(pen);EndPaint(h,&ps);return 0;}
 if(msg==WM_NCDESTROY){RemovePropW(h,L"QcHover");RemoveWindowSubclass(h,widgetProc,91);}
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK Theme::headerProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* theme=reinterpret_cast<Theme*>(value);
 if(msg==WM_PAINT){PAINTSTRUCT ps;auto dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);FillRect(dc,&r,theme->background);auto old=SelectObject(dc,reinterpret_cast<HFONT>(SendMessageW(h,WM_GETFONT,0,0)));
 int count=Header_GetItemCount(h);for(int i=0;i<count;++i){RECT cell;Header_GetItemRect(h,i,&cell);wchar_t text[128]{};HDITEMW item{};item.mask=HDI_TEXT;item.pszText=text;item.cchTextMax=128;Header_GetItem(h,i,&item);cell.left+=12;paintText(dc,text,cell,theme->colors.muted);}
 SelectObject(dc,old);EndPaint(h,&ps);return 0;}
 if(msg==WM_NCDESTROY)RemoveWindowSubclass(h,headerProc,92);return DefSubclassProc(h,msg,w,l);
}
}
