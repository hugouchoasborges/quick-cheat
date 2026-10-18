#include "app.hpp"
#include <array>
namespace qc {
std::wstring textOf(HWND control) {
 int n=GetWindowTextLengthW(control);std::wstring text(static_cast<size_t>(n)+1,L'\0');GetWindowTextW(control,text.data(),n+1);text.resize(n);return text;
}
HWND control(HWND parent,const wchar_t* cls,const wchar_t* text,DWORD style,int id) {
 return CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,0,0,0,0,parent,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
}
void bounds(HWND window,int x,int y,int width,int height,UINT dpi) { MoveWindow(window,MulDiv(x,dpi,96),MulDiv(y,dpi,96),MulDiv(width,dpi,96),MulDiv(height,dpi,96),TRUE); }
Target identify(HWND window) {
 Target target;if(!window||!IsWindowVisible(window))return target;GetWindowThreadProcessId(window,&target.process);
 if(!target.process||target.process==GetCurrentProcessId())return {};
 HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,target.process);if(!process)return {};
 std::wstring path(32768,L'\0');DWORD size=static_cast<DWORD>(path.size());BOOL ok=QueryFullProcessImageNameW(process,0,path.data(),&size);CloseHandle(process);
 if(!ok)return {};path.resize(size);target.executable=appKey(path);target.window=window;target.name=target.executable;
 if(target.name.ends_with(L".exe"))target.name.resize(target.name.size()-4);return target;
}
bool targetAlive(const Target& target) {
 if(!target.window||!IsWindow(target.window))return false;
 auto now=identify(target.window);return now.process==target.process&&now.executable==target.executable;
}
bool sendChord(const Target& target,Chord chord,std::wstring& error) {
 if(!targetAlive(target)){error=L"The application window is no longer available. Activate it and try again.";return false;}
 constexpr WORD physical[]={VK_CONTROL,VK_MENU,VK_SHIFT,VK_LWIN,VK_RWIN};
 for(auto key:physical)if(GetAsyncKeyState(key)&0x8000){error=L"Release Ctrl, Alt, Shift and Windows keys before clicking a shortcut.";return false;}
 if(GetAsyncKeyState(chord.key)&0x8000){error=L"Release the shortcut key before clicking it.";return false;}
 if(GetForegroundWindow()!=target.window)SetForegroundWindow(target.window);
 if(GetForegroundWindow()!=target.window){error=L"Windows could not focus the target application. Activate it and try again.";return false;}
 std::array<INPUT,10> input{};UINT count=0;
 auto add=[&](WORD key,bool up) { auto& i=input[count++];i.type=INPUT_KEYBOARD;i.ki.wVk=key;
 i.ki.dwFlags=up?KEYEVENTF_KEYUP:0;
 if(key==VK_LWIN||key==VK_RWIN||key==VK_INSERT||key==VK_DELETE||key==VK_HOME||key==VK_END||key==VK_PRIOR||key==VK_NEXT||(key>=VK_LEFT&&key<=VK_DOWN))i.ki.dwFlags|=KEYEVENTF_EXTENDEDKEY;
 i.ki.dwExtraInfo=0x51434854; };
 struct Modifier {UINT first;WORD second;};
 const Modifier mods[]={{MOD_CONTROL,VK_CONTROL},{MOD_ALT,VK_MENU},{MOD_SHIFT,VK_SHIFT},{MOD_WIN,VK_LWIN}};
 for(auto [flag,key]:mods)if(chord.modifiers&flag)add(key,false);add(chord.key,false);add(chord.key,true);
 for(int i=3;i>=0;--i)if(chord.modifiers&mods[i].first)add(mods[i].second,true);
 UINT sent=SendInput(count,input.data(),sizeof(INPUT));
 if(sent!=count) {
 // Release only keys for which our key-down was actually inserted.
 std::array<INPUT,5> release{};UINT releaseCount=0;
 for(UINT i=0;i<sent;++i)if(!(input[i].ki.dwFlags&KEYEVENTF_KEYUP)) {
 bool ended=false;for(UINT j=i+1;j<sent;++j)if(input[j].ki.wVk==input[i].ki.wVk&&(input[j].ki.dwFlags&KEYEVENTF_KEYUP))ended=true;
 if(!ended){release[releaseCount]=input[i];release[releaseCount++].ki.dwFlags|=KEYEVENTF_KEYUP;}}
 if(releaseCount)SendInput(releaseCount,release.data(),sizeof(INPUT));
 error=L"Windows blocked some or all of the shortcut. Elevated applications and protected windows may reject input.";return false;
 }return true;
}
}
