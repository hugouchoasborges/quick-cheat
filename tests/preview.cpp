#include "storage.hpp"
#include "attachments.hpp"
#include <commctrl.h>
#include <psapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>
namespace {
struct Child {PROCESS_INFORMATION pi{};~Child(){if(pi.hProcess){if(WaitForSingleObject(pi.hProcess,0)==WAIT_TIMEOUT)TerminateProcess(pi.hProcess,1);CloseHandle(pi.hProcess);CloseHandle(pi.hThread);}}void start(std::wstring cmd){STARTUPINFOW si{sizeof(si)};if(!CreateProcessW(nullptr,cmd.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&si,&pi))throw std::runtime_error("Cannot launch child");}};
void pause(int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));}
HWND waitWindow(const wchar_t* cls){for(int i=0;i<100;++i){HWND h=FindWindowW(cls,nullptr);if(h)return h;pause(40);}throw std::runtime_error("Window not found");}
void capture(HWND window,const std::filesystem::path& file){
 RECT rect{};GetWindowRect(window,&rect);int width=rect.right-rect.left,height=rect.bottom-rect.top;
 HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);HBITMAP image=CreateCompatibleBitmap(screen,width,height);auto old=SelectObject(dc,image);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);BOOL ok=PrintWindow(window,dc,2);SelectObject(dc,old);
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
 std::vector<char> pixels(static_cast<size_t>(width)*height*4);ok=ok&&GetDIBits(dc,image,0,height,pixels.data(),&info,DIB_RGB_COLORS)!=0;
 DeleteObject(image);DeleteDC(dc);ReleaseDC(nullptr,screen);if(!ok)throw std::runtime_error("Capture failed");
 BITMAPFILEHEADER header{};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+static_cast<DWORD>(pixels.size());
 std::ofstream out(file,std::ios::binary);out.write(reinterpret_cast<char*>(&header),sizeof(header));out.write(reinterpret_cast<char*>(&info.bmiHeader),sizeof(info.bmiHeader));out.write(pixels.data(),pixels.size());
}
unsigned long long ticks(FILETIME time){ULARGE_INTEGER n{};n.LowPart=time.dwLowDateTime;n.HighPart=time.dwHighDateTime;return n.QuadPart;}
}
int wmain(int argc,wchar_t** argv){
 struct Apartment{HRESULT result=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);~Apartment(){if(SUCCEEDED(result))CoUninitialize();}} apartment;
 if(argc!=2)return 2;if(FindWindowW(L"QuickCheatHost",nullptr)){std::cerr<<"Exit Quick Cheat first\n";return 2;}
 try {
 auto appExe=std::filesystem::absolute(argv[1]);auto output=appExe.parent_path().parent_path()/L"qa";std::filesystem::create_directories(output);
 auto data=output/(L"preview-"+std::to_wstring(GetCurrentProcessId()));qc::Store s;s.trigger.modifiers=7;
 s.profiles.push_back({L"qc_integration.exe",L"Visual Studio Code (demo)",{{L"Save file",L"File",{MOD_CONTROL,'S'}},{L"Open file",L"File",{MOD_CONTROL,'O'}},{L"Find in file",L"Search",{MOD_CONTROL,'F'}},{L"Search across files",L"Search",{MOD_CONTROL|MOD_SHIFT,'F'}},{L"Command palette",L"Navigation",{MOD_CONTROL|MOD_SHIFT,'P'}},{L"Go to file",L"Navigation",{MOD_CONTROL,'P'}},{L"Toggle sidebar",L"View",{MOD_CONTROL,'B'}}}});
 s.profiles.back().workflows={{L"Prepare a change for review",{L"Open the project folder and check the current branch.",L"Review the modified files in Source Control. Keep each change focused on one task.",L"Read the diff, write a clear commit message, and commit the change."}},{L"Find a symbol in the project",{L"Open the command palette.",L"Choose the symbol search command and type the name you want to find.",L"Select a result and review its surrounding code."}}};
 std::wstring demoImage,imageError;auto demoSource=appExe.parent_path().parent_path()/L"demo-markdown"/L"prompt-guide.png";if(std::filesystem::exists(demoSource)&&qc::importWorkflowImage(demoSource,data,demoImage,imageError))s.profiles.back().workflows[0].steps[0]=L"## Write a clear prompt\n\nDefine the **goal**, add *context*, then choose the format.\n\n- Keep the facts\n- Use three short bullets\n\n![Prompt anatomy]("+demoImage+L")\n\nUse `Ctrl+A` to select your draft. [Markdown guide](https://commonmark.org/help/).";
 std::wstring error;if(!qc::saveStore(data/L"shortcuts.qc",s,error))throw std::runtime_error("Cannot save preview data");
 Child app,target;app.start(L"\""+appExe.wstring()+L"\" --data-dir \""+data.wstring()+L"\"");auto host=waitWindow(L"QuickCheatHost");pause(400);
 PROCESS_MEMORY_COUNTERS_EX memory{};memory.cb=sizeof(memory);GetProcessMemoryInfo(app.pi.hProcess,reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory));
 FILETIME created{},exited{},kernel{},user{};GetProcessTimes(app.pi.hProcess,&created,&exited,&kernel,&user);auto before=ticks(kernel)+ticks(user);pause(1000);GetProcessTimes(app.pi.hProcess,&created,&exited,&kernel,&user);
 std::cout<<"Idle working set MiB="<<memory.WorkingSetSize/1048576.0<<" private MiB="<<memory.PrivateUsage/1048576.0<<" CPU ms over 1 second="<<(ticks(kernel)+ticks(user)-before)/10000.0<<'\n';
 target.start(L"\""+(appExe.parent_path()/L"qc_integration.exe").wstring()+L"\" --target");auto targetWindow=waitWindow(L"QuickCheatIntegrationTarget");SetForegroundWindow(targetWindow);pause(200);
 if(GetForegroundWindow()!=targetWindow)throw std::runtime_error("Cannot focus controlled target");
 for(auto mode:{qc::ThemeMode::Dark,qc::ThemeMode::Light,qc::ThemeMode::System}){
 SetForegroundWindow(targetWindow);pause(120);if(GetForegroundWindow()!=targetWindow)throw std::runtime_error("Controlled demo target lost foreground");
 SendMessageW(host,WM_COMMAND,101,0);auto editor=waitWindow(L"QuickCheatEditor");
 SendMessageW(GetDlgItem(editor,214),CB_SETCURSEL,static_cast<UINT>(mode),0);SendMessageW(editor,WM_COMMAND,MAKEWPARAM(214,CBN_SELCHANGE),0);pause(120);
 auto name=mode==qc::ThemeMode::Dark?L"dark":mode==qc::ThemeMode::Light?L"light":L"system";capture(editor,output/(std::wstring(L"editor-")+name+L".bmp"));
 capture(editor,output/(std::wstring(L"settings-general-")+name+L".bmp"));SendMessageW(editor,WM_COMMAND,221,0);pause(120);capture(editor,output/(std::wstring(L"editor-")+name+L".bmp"));
 SendMessageW(editor,WM_COMMAND,108,0);auto workflows=GetDlgItem(editor,230);SendMessageW(GetDlgItem(workflows,402),LB_SETCURSEL,0,0);SendMessageW(workflows,WM_COMMAND,MAKEWPARAM(402,LBN_SELCHANGE),0);pause(120);capture(editor,output/(std::wstring(L"workflow-editor-")+name+L".bmp"));SendMessageW(editor,WM_COMMAND,221,0);
 SendMessageW(host,WM_HOTKEY,1,0);auto popup=waitWindow(L"QuickCheatPopup");pause(120);capture(popup,output/(std::wstring(L"popup-")+name+L".bmp"));
 SendMessageW(popup,WM_COMMAND,110,0);pause(100);capture(popup,output/(std::wstring(L"popup-pinned-")+name+L".bmp"));
 SendMessageW(popup,WM_COMMAND,107,0);auto tree=GetDlgItem(popup,7);auto root=TreeView_GetRoot(tree);auto overview=TreeView_GetNextSibling(tree,root);TreeView_SelectItem(tree,overview?overview:root);pause(120);capture(popup,output/(std::wstring(L"workflow-overview-")+name+L".bmp"));auto step=TreeView_GetChild(tree,root);TreeView_SelectItem(tree,step);pause(120);capture(popup,output/(std::wstring(L"workflow-popup-")+name+L".bmp"));SendMessageW(popup,WM_COMMAND,106,0);SendMessageW(popup,WM_COMMAND,110,0);
 }
 PostMessageW(host,WM_COMMAND,103,0);WaitForSingleObject(app.pi.hProcess,3000);PostMessageW(targetWindow,WM_CLOSE,0,0);WaitForSingleObject(target.pi.hProcess,3000);
 std::wcout<<L"Screenshots: "<<output.wstring()<<L'\n';return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
