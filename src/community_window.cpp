#include "community_window.hpp"
#include <algorithm>
namespace qc {
namespace {
constexpr UINT Completed=WM_APP+30;
constexpr int Search=501,Current=502,Rows=503,Next=504,Install=505,Cancel=506;
}
CommunityWindow::~CommunityWindow(){canceled=true;if(worker.joinable())worker.join();if(window)DestroyWindow(window);if(font)DeleteObject(font);}
void CommunityWindow::show(){
 if(!window){
  WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatCommunity";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(app.instance,MAKEINTRESOURCEW(1));RegisterClassW(&cls);
  UINT dpi=GetDpiForWindow(app.host);window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat — Community",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,MulDiv(800,dpi,96),MulDiv(600,dpi,96),nullptr,nullptr,app.instance,this);
  if(!window){app.error(L"Cannot open the community catalog.");return;}
  search=control(window,L"EDIT",L"",WS_TABSTOP|WS_BORDER|ES_AUTOHSCROLL,Search);SendMessageW(search,EM_SETLIMITTEXT,160,0);SendMessageW(search,EM_SETCUEBANNER,TRUE,reinterpret_cast<LPARAM>(L"Search public collections..."));
  searchButton=control(window,L"BUTTON",L"Search",WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK);
  current=control(window,L"BUTTON",L"Current application",WS_TABSTOP|BS_AUTOCHECKBOX,Current);
  rows=control(window,WC_LISTVIEWW,L"",WS_TABSTOP|WS_BORDER|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,Rows);ListView_SetExtendedListViewStyle(rows,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
  const wchar_t* labels[]={L"Collection",L"Application / site",L"Language",L"Version"};int widths[]={245,210,95,105};
  for(int i=0;i<4;++i){LVCOLUMNW column{};column.mask=LVCF_TEXT|LVCF_WIDTH;column.pszText=const_cast<wchar_t*>(labels[i]);column.cx=MulDiv(widths[i],dpi,96);ListView_InsertColumn(rows,i,&column);}
  details=control(window,L"EDIT",L"Select a collection to see its author, license and contents.",WS_TABSTOP|WS_BORDER|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL,507);
  status=control(window,L"STATIC",L"Search to load the public catalog. Nothing is uploaded.",SS_LEFT,508);
  next=control(window,L"BUTTON",L"Next page",WS_TABSTOP,Next);install=control(window,L"BUTTON",L"Download and preview...",WS_TABSTOP,Install);cancelButton=control(window,L"BUTTON",L"Cancel request",WS_TABSTOP,Cancel);
  themeChanged();layout();updateControls();
 }
 ShowWindow(window,SW_SHOW);SetForegroundWindow(window);SetFocus(search);
}
void CommunityWindow::themeChanged(){if(window){
 UINT dpi=GetDpiForWindow(window);HFONT nextFont=CreateFontW(-MulDiv(14,dpi,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 if(nextFont){for(HWND child=GetWindow(window,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT))SendMessageW(child,WM_SETFONT,reinterpret_cast<WPARAM>(nextFont),TRUE);if(font)DeleteObject(font);font=nextFont;}
 app.theme.style(window);InvalidateRect(window,nullptr,TRUE);
}}
bool CommunityWindow::dialogMessage(MSG& message){return window&&IsWindowVisible(window)&&GetAncestor(GetForegroundWindow(),GA_ROOT)==window&&IsDialogMessageW(window,&message);}
void CommunityWindow::layout(){
 if(!search)return;RECT rect{};GetClientRect(window,&rect);UINT dpi=GetDpiForWindow(window);int width=MulDiv(rect.right,96,dpi),height=MulDiv(rect.bottom,96,dpi);
 bounds(search,12,12,width-116,28,dpi);bounds(searchButton,width-96,12,84,28,dpi);bounds(current,12,48,width-24,24,dpi);
 bounds(rows,12,80,width-24,std::max(80,height-286),dpi);bounds(details,12,height-196,width-24,112,dpi);
 bounds(status,12,height-78,width-24,28,dpi);bounds(next,12,height-40,96,28,dpi);bounds(cancelButton,116,height-40,120,28,dpi);bounds(install,width-224,height-40,212,28,dpi);
}
void CommunityWindow::updateControls(){
 EnableWindow(search,!busy);EnableWindow(searchButton,!busy);EnableWindow(current,!busy&&!app.target.executable.empty());EnableWindow(rows,!busy);
 EnableWindow(next,!busy&&!page.nextCursor.empty());EnableWindow(install,!busy&&ListView_GetNextItem(rows,-1,LVNI_SELECTED)>=0);EnableWindow(cancelButton,busy&&!canceled.load());
}
void CommunityWindow::selection(){
 int index=ListView_GetNextItem(rows,-1,LVNI_SELECTED);if(index<0||index>=static_cast<int>(page.items.size())){SetWindowTextW(details,L"Select a collection to see its author, license and contents.");updateControls();return;}
 const auto& entry=page.items[index];auto text=entry.title+L" — "+entry.authorName+L"\r\n"+entry.summary+L"\r\nLicense: "+entry.license+L" | Keyboard: "+entry.keyboardLayout+L" | Minimum client: "+entry.minimumClientVersion+L"\r\n"+std::to_wstring(entry.shortcuts)+L" shortcuts, "+std::to_wstring(entry.workflows)+L" workflows, "+std::to_wstring(entry.images)+L" images | "+std::to_wstring(entry.votes)+L" votes\r\nSource: "+community::Origin+L" | "+entry.id;
 SetWindowTextW(details,text.c_str());updateControls();
}
void CommunityWindow::cancel(){if(!busy)return;canceled=true;++generation;SetWindowTextW(status,L"Canceling request...");updateControls();}
void CommunityWindow::close(){cancel();ShowWindow(window,SW_HIDE);}
void CommunityWindow::request(bool more){
 if(busy)return;
 if(more){query=shownQuery;query.cursor=page.nextCursor;}
 else {query={};query.search=textOf(search);if(SendMessageW(current,BM_GETCHECK,0,0)==BST_CHECKED){query.executable=app.target.executable;query.hostname=app.target.website;}}
 start(false);
}
void CommunityWindow::preview(){
 if(busy)return;int index=ListView_GetNextItem(rows,-1,LVNI_SELECTED);if(index<0||index>=static_cast<int>(page.items.size()))return;
 prepared={};prepared.source=page.items[index];start(true);
}
void CommunityWindow::start(bool downloading){
 busy=true;download=downloading;canceled=false;succeeded=false;failure.clear();auto requestGeneration=++generation;auto directory=app.file.parent_path();
 SetWindowTextW(status,download?L"Downloading and checking collection...":L"Loading catalog...");updateControls();
 try {worker=std::thread([this,directory,requestGeneration]{
  try {if(download){community::PreparedCollection result;succeeded=community::prepare(prepared.source,version,directory,result,failure,&canceled);if(succeeded)prepared=std::move(result);}
   else {community::Page result;succeeded=community::list(query,directory,result,failure,&canceled);if(succeeded)pendingPage=std::move(result);}}
  catch(const std::exception&){succeeded=false;failure=L"The community request could not finish.";}
  PostMessageW(window,Completed,requestGeneration,0);
 });}catch(const std::exception&){busy=false;SetWindowTextW(status,L"Cannot start the community request.");updateControls();}
}
void CommunityWindow::complete(UINT_PTR requestGeneration){
 if(worker.joinable())worker.join();busy=false;
 if(requestGeneration!=generation||canceled){SetWindowTextW(status,L"Request canceled. Nothing was installed.");prepared={};pendingPage={};updateControls();return;}
 if(!succeeded){SetWindowTextW(status,failure.c_str());updateControls();return;}
 if(download){
  EnableWindow(window,FALSE);app.openEditor(1);
  if(app.editor&&app.editor->window&&app.editor->installCollection(prepared.collection)){
   std::wstring error;if(community::recordInstallation(prepared,app.store,app.file.parent_path(),error))SetWindowTextW(status,L"Collection installed. Its source and original version were recorded.");
   else {SetWindowTextW(status,L"Collection installed; source tracking could not be saved.");app.error(L"The collection was installed, but its source record could not be saved.\n\n"+error,app.popup);}
  }else SetWindowTextW(status,L"Installation canceled or failed. See any displayed error.");
  prepared={};EnableWindow(window,TRUE);
 }else {
  shownQuery=query;page=std::move(pendingPage);ListView_DeleteAllItems(rows);for(size_t i=0;i<page.items.size();++i){auto& entry=page.items[i];LVITEMW item{};item.mask=LVIF_TEXT;item.iItem=static_cast<int>(i);item.pszText=entry.title.data();ListView_InsertItem(rows,&item);
   auto target=entry.hostname.empty()?entry.executable:entry.hostname+L" / "+entry.executable;ListView_SetItemText(rows,item.iItem,1,target.data());ListView_SetItemText(rows,item.iItem,2,entry.language.data());ListView_SetItemText(rows,item.iItem,3,entry.version.data());}
  auto message=(page.fromCache?L"Offline cache — ":L"")+std::to_wstring(page.items.size())+L" collections on this page / "+std::to_wstring(page.total)+L" total.";if(!page.warning.empty())message+=L" "+page.warning;SetWindowTextW(status,message.c_str());selection();
 }
 updateControls();
}
LRESULT CALLBACK CommunityWindow::proc(HWND h,UINT msg,WPARAM w,LPARAM l){
 auto* self=reinterpret_cast<CommunityWindow*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(msg==WM_NCCREATE){self=static_cast<CommunityWindow*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);self->window=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}if(!self)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_CLOSE:self->close();return 0;
 case WM_SIZE:self->layout();return 0;
 case WM_GETMINMAXINFO:{auto* info=reinterpret_cast<MINMAXINFO*>(l);UINT dpi=GetDpiForWindow(h);info->ptMinTrackSize={MulDiv(640,dpi,96),MulDiv(480,dpi,96)};return 0;}
 case WM_DPICHANGED:{auto* rect=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,SWP_NOZORDER|SWP_NOACTIVATE);self->themeChanged();self->layout();return 0;}
 case Completed:self->complete(w);return 0;
 case WM_COMMAND:switch(LOWORD(w)){case IDOK:self->request();break;case IDCANCEL:self->close();break;case Next:self->request(true);break;case Install:self->preview();break;case Cancel:self->cancel();break;}return 0;
 case WM_NOTIFY:if(reinterpret_cast<NMHDR*>(l)->hwndFrom==self->rows&&reinterpret_cast<NMHDR*>(l)->code==LVN_ITEMCHANGED&&!self->busy)self->selection();break;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return self->app.theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_DRAWITEM:if(self->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l)))return TRUE;break;
 case WM_ERASEBKGND:{RECT rect;GetClientRect(h,&rect);FillRect(reinterpret_cast<HDC>(w),&rect,self->app.theme.background);return 1;}
 }return DefWindowProcW(h,msg,w,l);
}
}
