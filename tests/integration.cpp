#include "storage.hpp"
#include <commctrl.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <stdexcept>
namespace {
struct Child {
 PROCESS_INFORMATION info{};
 ~Child(){if(info.hProcess){if(WaitForSingleObject(info.hProcess,0)==WAIT_TIMEOUT)TerminateProcess(info.hProcess,1);CloseHandle(info.hProcess);CloseHandle(info.hThread);}}
 void start(std::wstring command){STARTUPINFOW si{sizeof(si)};if(!CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&si,&info))throw std::runtime_error("Cannot launch child");}
 void wait(){if(WaitForSingleObject(info.hProcess,5000)!=WAIT_OBJECT_0)throw std::runtime_error("Child did not exit");}
};
bool waitFor(std::function<bool()> predicate){for(int i=0;i<150;++i){if(predicate())return true;std::this_thread::sleep_for(std::chrono::milliseconds(20));}return false;}
void check(bool condition,const char* message){if(!condition){EnumWindows([](HWND h,LPARAM)->BOOL{wchar_t title[512]{},cls[128]{};GetWindowTextW(h,title,512);GetClassNameW(h,cls,128);if(wcsstr(cls,L"QuickCheat")||wcscmp(title,L"Quick Cheat")==0){std::wcerr<<L"Window: "<<cls<<L" / "<<title<<L" visible="<<IsWindowVisible(h)<<L'\n';EnumChildWindows(h,[](HWND c,LPARAM)->BOOL{wchar_t t[512]{};GetWindowTextW(c,t,512);if(*t)std::wcerr<<L"  "<<t<<L'\n';return TRUE;},0);}return TRUE;},0);throw std::runtime_error(message);}}
std::wstring text(HWND h){wchar_t s[512]{};GetWindowTextW(h,s,512);return s;}
void activate(HWND h){ShowWindow(h,SW_RESTORE);HWND current=GetForegroundWindow();wchar_t cls[128]{};GetClassNameW(current,cls,128);if(wcsstr(cls,L"QuickCheatIntegrationTarget"))SendMessageW(current,WM_APP+10,0,reinterpret_cast<LPARAM>(h));else SetForegroundWindow(h);check(waitFor([&]{return GetForegroundWindow()==h;}),"Cannot focus controlled target");std::this_thread::sleep_for(std::chrono::milliseconds(120));}
void toggle(HWND target){check(GetForegroundWindow()==target,"Controlled target is not foreground");check(PostMessageW(FindWindowW(L"QuickCheatHost",nullptr),WM_HOTKEY,1,MAKELPARAM(7,VK_SPACE))!=FALSE,"Cannot dispatch hotkey message");}
LRESULT CALLBACK targetProc(HWND h,UINT msg,WPARAM w,LPARAM l){if(msg==WM_APP+10){SetForegroundWindow(reinterpret_cast<HWND>(l));return 0;}if(msg==WM_KEYDOWN&&w==VK_F6){SetWindowTextW(h,L"Shortcut received");return 0;}if(msg==WM_CLOSE){DestroyWindow(h);PostQuitMessage(0);return 0;}return DefWindowProcW(h,msg,w,l);}
int targetMain(const wchar_t* cls){WNDCLASSW wc{};wc.lpfnWndProc=targetProc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=cls;wc.hbrBackground=GetSysColorBrush(COLOR_WINDOW);RegisterClassW(&wc);HWND h=CreateWindowW(cls,L"Quick Cheat controlled target",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,460,240,nullptr,nullptr,wc.hInstance,nullptr);ShowWindow(h,SW_SHOW);SetForegroundWindow(h);MSG m{};while(GetMessageW(&m,nullptr,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;}
}
int wmain(int argc,wchar_t** argv) {
 if(argc==2&&wcscmp(argv[1],L"--target")==0)return targetMain(L"QuickCheatIntegrationTarget");
 if(argc==2&&wcscmp(argv[1],L"--target2")==0)return targetMain(L"QuickCheatIntegrationTarget2");
 if(argc!=2){std::cerr<<"Usage: qc_integration.exe <QuickCheat.exe>\n";return 2;}
 if(FindWindowW(L"QuickCheatHost",nullptr)){std::cerr<<"Exit Quick Cheat before running integration checks.\n";return 2;}
 const auto dir=std::filesystem::temp_directory_path()/(L"QuickCheat-integration-"+std::to_wstring(GetCurrentProcessId()));std::filesystem::create_directories(dir);
 int result=0;
 qc::Store initial;initial.trigger.modifiers=MOD_CONTROL|MOD_ALT|MOD_SHIFT;std::wstring initialError;
 if(!qc::saveStore(dir/L"shortcuts.qc",initial,initialError)){std::cerr<<"Cannot create isolated test data\n";return 1;}
 try {
 Child app,target,other;auto appExe=std::filesystem::absolute(argv[1]);wchar_t self[32768]{};GetModuleFileNameW(nullptr,self,32768);
 const auto command=L"\""+appExe.wstring()+L"\" --data-dir \""+dir.wstring()+L"\"";
 app.start(command);HWND host{};check(waitFor([&]{host=FindWindowW(L"QuickCheatHost",nullptr);return host!=nullptr;}),"Host missing");
 target.start(L"\""+std::wstring(self)+L"\" --target");HWND targetWindow{};check(waitFor([&]{targetWindow=FindWindowW(L"QuickCheatIntegrationTarget",nullptr);return targetWindow!=nullptr;}),"Target missing");activate(targetWindow);
 toggle(targetWindow);HWND popup{};check(waitFor([&]{popup=FindWindowW(L"QuickCheatPopup",nullptr);return popup&&IsWindowVisible(popup);}),"Activation hotkey failed");
 check(GetForegroundWindow()==popup,"Popup did not focus search");check(ListView_GetItemCount(GetDlgItem(popup,2))==0,"Expected empty popup");
 check(!IsWindowEnabled(GetDlgItem(popup,105)),"Execute should be disabled for an empty list");
 PostMessageW(host,WM_COMMAND,101,0);HWND editor{};check(waitFor([&]{editor=FindWindowW(L"QuickCheatEditor",nullptr);return editor&&IsWindowVisible(editor);}),"Editor missing");
 SendMessageW(GetDlgItem(editor,203),WM_SETTEXT,0,reinterpret_cast<LPARAM>(L"Integration shortcut"));
 SendMessageW(GetDlgItem(editor,204),WM_SETTEXT,0,reinterpret_cast<LPARAM>(L"Editing"));
 SendMessageW(GetDlgItem(editor,205),WM_SETTEXT,0,reinterpret_cast<LPARAM>(L"F6"));
 SendMessageW(editor,WM_COMMAND,208,0);
 qc::Store saved;std::wstring error;check(qc::loadStore(dir/L"shortcuts.qc",saved,error)&&saved.profiles.size()==1&&saved.profiles[0].shortcuts.size()==1,"Editor did not persist shortcut");
 for(auto mode:{qc::ThemeMode::Dark,qc::ThemeMode::Light,qc::ThemeMode::System}){SendMessageW(GetDlgItem(editor,214),CB_SETCURSEL,static_cast<UINT>(mode),0);SendMessageW(editor,WM_COMMAND,MAKEWPARAM(214,CBN_SELCHANGE),0);check(qc::loadStore(dir/L"shortcuts.qc",saved,error)&&saved.theme==mode,"Theme did not persist");}
 PostMessageW(editor,WM_CLOSE,0,0);activate(targetWindow);toggle(targetWindow);
 check(waitFor([&]{return IsWindowVisible(popup)&&ListView_GetItemCount(GetDlgItem(popup,2))==1;}),"Saved shortcut missing from popup");
 check(GetForegroundWindow()==popup,"Popup search did not receive focus after editing");
 SendMessageW(GetDlgItem(popup,6),WM_SETTEXT,0,reinterpret_cast<LPARAM>(L"no-match"));check(ListView_GetItemCount(GetDlgItem(popup,2))==0&&!IsWindowEnabled(GetDlgItem(popup,105)),"Search empty state failed");
 SendMessageW(GetDlgItem(popup,6),WM_SETTEXT,0,reinterpret_cast<LPARAM>(L"integration"));check(ListView_GetItemCount(GetDlgItem(popup,2))==1,"Search matching failed");
 PostMessageW(GetDlgItem(popup,2),WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(40,30));PostMessageW(GetDlgItem(popup,2),WM_LBUTTONUP,0,MAKELPARAM(40,30));
 std::this_thread::sleep_for(std::chrono::milliseconds(100));check(text(targetWindow)!=L"Shortcut received"&&IsWindowVisible(popup),"Selecting a row executed its shortcut");
 SendMessageW(GetDlgItem(popup,105),BM_CLICK,0,0);
 check(waitFor([&]{return text(targetWindow)==L"Shortcut received";}),"Execute button did not deliver F6 to target");
 check(waitFor([&]{return !IsWindowVisible(popup);}),"Popup did not hide after execution");
 toggle(targetWindow);check(waitFor([&]{return IsWindowVisible(popup);}),"Popup failed to reopen");SendMessageW(popup,WM_COMMAND,110,0);
 auto secondExe=dir/L"qc_other_target.exe";std::filesystem::copy_file(self,secondExe);
 other.start(L"\""+secondExe.wstring()+L"\" --target2");HWND otherWindow{};check(waitFor([&]{otherWindow=FindWindowW(L"QuickCheatIntegrationTarget2",nullptr);return otherWindow!=nullptr;}),"Second target missing");activate(otherWindow);
 check(waitFor([&]{return text(GetDlgItem(popup,1))==L"qc_other_target"&&ListView_GetItemCount(GetDlgItem(popup,2))==0;}),"Foreground change did not update popup");
 PostMessageW(host,WM_COMMAND,103,0);app.wait();
 Child restarted;restarted.start(command);check(waitFor([&]{host=FindWindowW(L"QuickCheatHost",nullptr);return host!=nullptr;}),"Restart failed");
 activate(targetWindow);toggle(targetWindow);check(waitFor([&]{popup=FindWindowW(L"QuickCheatPopup",nullptr);return popup&&IsWindowVisible(popup)&&ListView_GetItemCount(GetDlgItem(popup,2))==1;}),"Shortcut did not survive restart");
 PostMessageW(host,WM_COMMAND,103,0);restarted.wait();PostMessageW(targetWindow,WM_CLOSE,0,0);target.wait();PostMessageW(otherWindow,WM_CLOSE,0,0);other.wait();
 std::cout<<"Integration passed: search focus/filter, theme persistence, selection without execution, explicit Execute, target restoration, foreground switch, restart\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';result=1;}
 std::error_code ec;if(result==0)std::filesystem::remove_all(dir,ec);else std::wcerr<<L"Diagnostic directory: "<<dir.wstring()<<L'\n';return result;
}
