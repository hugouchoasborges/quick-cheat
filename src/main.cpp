#include "app.hpp"
#include "workflows.hpp"
#include "markdown_view.hpp"
#include "dock.hpp"
#include "browser_context.hpp"
#include "settings.hpp"
#include <shellapi.h>
#include <objbase.h>
#include <roapi.h>
#include <new>
namespace qc {
namespace {
App* active{};
void CALLBACK foregroundEvent(HWINEVENTHOOK,DWORD,HWND window,LONG object,LONG,DWORD,DWORD) {
 if(active&&object==OBJID_WINDOW)PostMessageW(active->host,ForegroundChanged,0,reinterpret_cast<LPARAM>(window));
}
}
void CALLBACK contextEvent(HWINEVENTHOOK,DWORD event,HWND window,LONG object,LONG child,DWORD,DWORD){
 if(!active||!window||!active->target.window)return;if(window!=active->target.window&&GetAncestor(window,GA_ROOT)!=active->target.window)return;
 if(event==EVENT_OBJECT_LOCATIONCHANGE){if(window==active->target.window&&object==OBJID_WINDOW)PostMessageW(active->host,BrowserDirty,event,0);}
 else if(NativeBrowserContext::supported(active->target.executable)){bool addressValue=event==EVENT_OBJECT_VALUECHANGE&&object==OBJID_CLIENT&&child==CHILDID_SELF;bool caption=event==EVENT_OBJECT_NAMECHANGE&&window==active->target.window&&object==OBJID_WINDOW;if(addressValue||caption)PostMessageW(active->host,BrowserDirty,event,0);}
}
App::App()=default;
App::~App() {
 syncCategoryStates();
 theme.stop();
 if(foregroundHook)UnhookWinEvent(foregroundHook);if(browserNameHook)UnhookWinEvent(browserNameHook);if(browserValueHook)UnhookWinEvent(browserValueHook);if(locationHook)UnhookWinEvent(locationHook);browser.reset();dock.reset();
 if(host){UnregisterHotKey(host,hotkeyId);NOTIFYICONDATAW n{sizeof(n)};n.hWnd=host;n.uID=1;Shell_NotifyIconW(NIM_DELETE,&n);}
 workflowEditor.reset();editor.reset();if(popup)DestroyWindow(popup);if(host)DestroyWindow(host);
 if(font)DeleteObject(font);if(labelFont)DeleteObject(labelFont);if(bold)DeleteObject(bold);active=nullptr;
}
void App::error(const std::wstring& text,HWND owner) { MessageBoxW(owner?owner:host,text.c_str(),L"Quick Cheat",MB_OK|MB_ICONWARNING); }
bool App::start(HINSTANCE value) {
 instance=value;active=this;
 if(file.empty())file=dataPath();std::wstring message;
 if(file.empty()){error(L"Cannot locate your local application data folder.");return false;}
 if(!loadStore(file,store,message)){error(message+L"\n\n"+file.wstring());return false;}
 WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=hostProc;cls.lpszClassName=L"QuickCheatHost";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));RegisterClassW(&cls);
 host=CreateWindowExW(WS_EX_TOOLWINDOW,cls.lpszClassName,L"Quick Cheat",WS_OVERLAPPED,0,0,0,0,nullptr,nullptr,instance,this);if(!host)return false;
 taskbarCreated=RegisterWindowMessageW(L"TaskbarCreated");
 theme.observe(host);applyTheme();
 bool registered=RegisterHotKey(host,hotkeyId,store.trigger.modifiers|MOD_NOREPEAT,store.trigger.key)!=FALSE;
 bool fallback=false;
 if(!registered&&GetFileAttributesW(file.c_str())==INVALID_FILE_ATTRIBUTES){
 Chord alternative{MOD_CONTROL|MOD_ALT|MOD_SHIFT,VK_SPACE};
 if(RegisterHotKey(host,hotkeyId,alternative.modifiers|MOD_NOREPEAT,alternative.key)){store.trigger=alternative;registered=true;fallback=true;}}
 foregroundHook=SetWinEventHook(EVENT_SYSTEM_FOREGROUND,EVENT_SYSTEM_FOREGROUND,nullptr,foregroundEvent,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
 if(!foregroundHook){error(L"Cannot monitor foreground changes.");return false;}
 browser=std::make_unique<NativeBrowserContext>(host,BrowserChanged);dock=std::make_unique<Dock>(*this);
 browserNameHook=SetWinEventHook(EVENT_OBJECT_NAMECHANGE,EVENT_OBJECT_NAMECHANGE,nullptr,contextEvent,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);browserValueHook=SetWinEventHook(EVENT_OBJECT_VALUECHANGE,EVENT_OBJECT_VALUECHANGE,nullptr,contextEvent,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);locationHook=SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE,EVENT_OBJECT_LOCATIONCHANGE,nullptr,contextEvent,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
 onForeground(GetForegroundWindow());addTray();
 if(!trayAdded){error(L"Cannot add the notification icon. Quick Cheat will close; please try again.");return false;}
 if(!registered||fallback){NOTIFYICONDATAW n{sizeof(n)};n.hWnd=host;n.uID=1;n.uFlags=NIF_INFO;n.dwInfoFlags=NIIF_INFO;
 wcscpy_s(n.szInfoTitle,L"Quick Cheat activation");auto info=registered?L"The default shortcut is in use. Using "+formatChord(store.trigger)+L". Change it in Settings.":L"Your activation shortcut is in use. Open Settings from the tray to change it.";
 wcsncpy_s(n.szInfo,info.c_str(),_TRUNCATE);Shell_NotifyIconW(NIM_MODIFY,&n);}
 return true;
}
void App::addTray() {
 NOTIFYICONDATAW n{sizeof(n)};n.hWnd=host;n.uID=1;n.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;n.uCallbackMessage=TrayMessage;n.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));
 if(!n.hIcon)n.hIcon=LoadIconW(nullptr,IDI_APPLICATION);
 wcscpy_s(n.szTip,L"Quick Cheat — Settings");trayAdded=Shell_NotifyIconW(NIM_ADD,&n)!=FALSE;
}
void App::trayMenu() {
 HMENU menu=CreatePopupMenu();AppendMenuW(menu,MF_STRING,CmdOpen,L"Show shortcuts");AppendMenuW(menu,MF_STRING,CmdManage,L"Settings...");
 AppendMenuW(menu,MF_STRING,CmdData,L"Open data folder");AppendMenuW(menu,MF_SEPARATOR,0,nullptr);AppendMenuW(menu,MF_STRING,CmdExit,L"Exit Quick Cheat");
 POINT p;GetCursorPos(&p);SetForegroundWindow(host);int command=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,p.x,p.y,0,host,nullptr);DestroyMenu(menu);
 if(command)PostMessageW(host,WM_COMMAND,command,0);PostMessageW(host,WM_NULL,0,0);
}
void App::onForeground(HWND window) {
 // Events may already be stale by the time our message queue receives them.
 if(window!=GetForegroundWindow())return;
 if(dock)dock->refresh();DWORD pid=0;GetWindowThreadProcessId(window,&pid);if(!pid||pid==GetCurrentProcessId())return;
 // Tray/menu activation and transient failures are not a new application context.
 wchar_t cls[96]{};GetClassNameW(window,cls,96);
 if(wcscmp(cls,L"Shell_TrayWnd")==0||wcscmp(cls,L"Shell_SecondaryTrayWnd")==0||wcscmp(cls,L"NotifyIconOverflowWindow")==0||wcscmp(cls,L"TopLevelWindowForOverflowXamlIsland")==0||wcscmp(cls,L"#32768")==0||(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_NOACTIVATE))return;
 auto next=identify(window);if(!next.window||next.executable.empty())return;
 if(next.window==target.window&&next.process==target.process&&next.executable==target.executable){requestWebsite();return;}
 syncCategoryStates();pendingExecution=false;pendingPopupOpen=false;if(browser)browser->clear();target=std::move(next);contextDirty=true;requestWebsite();if(popup&&IsWindowVisible(popup))refreshPopup(true);if(dock)dock->refresh();
}

void App::togglePopup(){
 if(pendingExecution){pendingExecution=false;if(browser)browser->clear();}
 if(pendingPopupOpen){pendingPopupOpen=false;if(browser)browser->clear();return;}
 if(popup&&IsWindowVisible(popup)){if(popupPinned){SetForegroundWindow(popup);SetFocus(settingsVisible&&editor?GetDlgItem(editor->window,settings::tabId(editor->page)):search);}else hidePopup();return;}
 onForeground(GetForegroundWindow());if(!targetAlive(target)){error(L"Activate an application window first, then press the activation shortcut.");return;}
 if(browser&&NativeBrowserContext::supported(target.executable)&&GetForegroundWindow()==target.window){pendingPopupOpen=true;openingGeneration=browser->request(target.window,target.process,target.executable);return;}
 showPopupNow();
}
void App::showPopupNow(){
 if(!popup)createPopup();refreshPopup(true);positionPopup(true);layoutPopup();SetForegroundWindow(popup);SetFocus(settingsVisible&&editor?GetDlgItem(editor->window,settings::tabId(editor->page)):search);
}
void App::hidePopup(bool restore){pendingPopupOpen=false;if(pendingExecution){pendingExecution=false;if(browser)browser->clear();}if(!popup)return;syncCategoryStates();bool ownFocus=GetForegroundWindow()==popup;ShowWindow(popup,SW_HIDE);if(workflowDetail)clearMarkdown(workflowDetail);if(restore&&ownFocus&&targetAlive(target))SetForegroundWindow(target.window);if(dock)dock->refresh();}
void App::applyTheme(){theme.resolve(store.theme);theme.style(popup);styleWorkflowPopup();if(editor)theme.style(editor->window);if(workflowEditor)theme.style(workflowEditor->window);if(editor&&editor->dockSettings)editor->dockSettings->themeChanged();if(dock)dock->themeChanged();}
void App::execute(size_t index) {
 if(workflowsVisible||!shortcutVisible(static_cast<int>(index)))return;
 // Re-read actual foreground before consulting a potentially stale event.
 HWND foreground=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(foreground,&pid);
 if(pid!=GetCurrentProcessId()&&foreground!=target.window){onForeground(foreground);return;}
 auto* p=currentProfile();if(!p||index>=visibleRows.size()||visibleRows[index]>=p->shortcuts.size())return;
 auto chord=p->shortcuts[visibleRows[index]].chord;
 if(chord.key==store.trigger.key&&chord.modifiers==store.trigger.modifiers){error(L"This shortcut is also the Quick Cheat activation shortcut. Change the activation shortcut before running it.");return;}
 runShortcut(chord,!target.website.empty()&&visibleRows[index]>=browserShortcutCount?target.website:L"");
}
bool App::commit(Store candidate) {
 bool changed=candidate.trigger.key!=store.trigger.key||candidate.trigger.modifiers!=store.trigger.modifiers;int next=hotkeyId==1?2:1;
 if(changed&&!RegisterHotKey(host,next,candidate.trigger.modifiers|MOD_NOREPEAT,candidate.trigger.key)){error(L"That activation shortcut is unavailable. Choose another combination.",editor?editor->window:host);return false;}
 std::wstring message;if(!saveStore(file,candidate,message)){if(changed)UnregisterHotKey(host,next);error(message,editor?editor->window:host);return false;}
 if(changed){UnregisterHotKey(host,hotkeyId);hotkeyId=next;}bool appearanceChanged=candidate.theme!=store.theme;store=std::move(candidate);contextDirty=true;if(appearanceChanged)applyTheme();if(popup)refreshPopup();if(dock)dock->refresh();return true;
}
void App::openEditor(int page) {
 if(!popup){createPopup();refreshPopup(true);}if(!editor)editor=std::make_unique<Editor>(*this);
 if(!settingsVisible)editor->show();settingsVisible=true;editor->selectPage(page);
 if(!IsWindowVisible(popup))positionPopup(true);layoutPopup();SetForegroundWindow(popup);SetFocus(GetDlgItem(editor->window,settings::tabId(editor->page)));
}
void App::closeSettings(){
 if(!settingsVisible||!editor||!editor->leavePage())return;
 editor->recording=false;ShowWindow(editor->window,SW_HIDE);settingsVisible=false;refreshPopup();layoutPopup();SetFocus(search);
}
LRESULT CALLBACK App::hostProc(HWND h,UINT msg,WPARAM w,LPARAM l) {
 auto* app=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){app=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}
 if(!app)return DefWindowProcW(h,msg,w,l);
 if(msg==app->taskbarCreated&&app->taskbarCreated){app->addTray();return 0;}
 switch(msg) {
 case WM_HOTKEY:app->togglePopup();return 0;
 case ForegroundChanged:app->onForeground(reinterpret_cast<HWND>(l));return 0;
 case BrowserChanged:app->receiveWebsite();return 0;
 case BrowserDirty:if(w==EVENT_OBJECT_LOCATIONCHANGE){if(app->dock)app->dock->refresh();}else if(app->browser&&!app->pendingExecution&&!app->pendingPopupOpen&&!app->browserTimerPending){app->browserTimerPending=true;SetTimer(h,41,100,nullptr);}return 0;
 case WM_TIMER:if(w==41){KillTimer(h,41);app->browserTimerPending=false;app->requestWebsite();}return 0;
 case ThemeChanged:case WM_SETTINGCHANGE:case WM_SYSCOLORCHANGE:app->applyTheme();return 0;
 case TrayMessage:if(l==WM_RBUTTONUP||l==WM_CONTEXTMENU)app->trayMenu();else if(l==WM_LBUTTONDBLCLK)app->openEditor();return 0;
 case WM_COMMAND:switch(LOWORD(w)){case CmdOpen:app->togglePopup();break;case CmdManage:app->openEditor(0);break;
 case CmdData:ShellExecuteW(h,L"open",app->file.parent_path().c_str(),nullptr,nullptr,SW_SHOWNORMAL);break;case CmdExit:if(app->editor&&app->settingsVisible&&!app->editor->leavePage())break;PostQuitMessage(0);break;}return 0;
 case WM_CLOSE:if(!app->editor||!app->settingsVisible||app->editor->leavePage())PostQuitMessage(0);return 0;
 }return DefWindowProcW(h,msg,w,l);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int) {
 HANDLE singleton=CreateMutexW(nullptr,FALSE,L"Local\\QuickCheat.Desktop");if(!singleton)return 1;
 if(GetLastError()==ERROR_ALREADY_EXISTS){MessageBoxW(nullptr,L"Quick Cheat is already running in the system tray.",L"Quick Cheat",MB_OK);CloseHandle(singleton);return 0;}
 INITCOMMONCONTROLSEX controls{sizeof(controls),ICC_STANDARD_CLASSES|ICC_LISTVIEW_CLASSES|ICC_TREEVIEW_CLASSES};InitCommonControlsEx(&controls);HRESULT apartment=RoInitialize(RO_INIT_SINGLETHREADED);
 HRESULT ole=OleInitialize(nullptr);int result=0;
 try { qc::App app;int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);
 if(argv){if(argc==3&&wcscmp(argv[1],L"--data-dir")==0)app.file=std::filesystem::path(argv[2])/L"shortcuts.qc";LocalFree(argv);}
 if(!app.start(instance))result=1;else {MSG message{};BOOL status=0;while((status=GetMessageW(&message,nullptr,0,0))>0) {
 if(app.settingsVisible&&app.editor&&IsWindowVisible(app.popup)&&IsWindowEnabled(app.editor->window)&&IsDialogMessageW(app.popup,&message)){if(message.message==WM_KEYDOWN&&message.wParam==VK_TAB&&app.editor)app.editor->revealFocus();continue;}
 if(app.popup&&IsWindowVisible(app.popup)&&GetForegroundWindow()==app.popup&&IsDialogMessageW(app.popup,&message)){if(message.message==WM_KEYDOWN&&message.wParam==VK_TAB&&app.editor)app.editor->revealFocus();continue;}
 TranslateMessage(&message);DispatchMessageW(&message);
 }if(status<0)result=1;}}
 catch(const std::exception&){MessageBoxW(nullptr,L"Quick Cheat could not continue. Saved shortcuts were not changed.",L"Quick Cheat",MB_OK|MB_ICONERROR);result=1;}
 if(SUCCEEDED(ole))OleUninitialize();if(SUCCEEDED(apartment))RoUninitialize();CloseHandle(singleton);return result;
}
