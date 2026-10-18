#include "app.hpp"
#include "browser_context.hpp"
#include <algorithm>
namespace qc {
namespace {
struct WebsitePrompt{App& app;std::wstring value;HWND input{};};
INT_PTR CALLBACK websiteProc(HWND h,UINT message,WPARAM w,LPARAM l){
 auto* prompt=reinterpret_cast<WebsitePrompt*>(GetWindowLongPtrW(h,DWLP_USER));
 if(message==WM_INITDIALOG){prompt=reinterpret_cast<WebsitePrompt*>(l);SetWindowLongPtrW(h,DWLP_USER,l);UINT dpi=GetDpiForWindow(h);RECT r{};GetClientRect(h,&r);int width=MulDiv(r.right,96,dpi);
  auto label=control(h,L"STATIC",L"Website hostname or HTTPS URL",0,10);bounds(label,20,16,width-40,24,dpi);
  prompt->input=control(h,L"EDIT",prompt->value.c_str(),WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP,11);bounds(prompt->input,20,46,width-40,30,dpi);SendMessageW(prompt->input,EM_SETLIMITTEXT,2048,0);
  auto hint=control(h,L"STATIC",L"Only the hostname is saved. Subdomains have separate profiles.",0,12);bounds(hint,20,86,width-40,36,dpi);
  auto save=control(h,L"BUTTON",L"Add website",BS_DEFPUSHBUTTON|WS_TABSTOP,IDOK),cancel=control(h,L"BUTTON",L"Cancel",WS_TABSTOP,IDCANCEL);bounds(save,width-248,132,112,32,dpi);bounds(cancel,width-124,132,104,32,dpi);
  for(HWND child=GetWindow(h,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT))SendMessageW(child,WM_SETFONT,reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)),TRUE);
  prompt->app.theme.style(h);SetFocus(prompt->input);return FALSE;
 }
 if(!prompt)return FALSE;
 switch(message){case WM_COMMAND:if(LOWORD(w)==IDCANCEL){EndDialog(h,IDCANCEL);return TRUE;}if(LOWORD(w)==IDOK){auto value=normalizeWebsite(textOf(prompt->input));if(value.empty()){prompt->app.error(L"Enter a valid website hostname or HTTP(S) URL, such as chatgpt.com.",h);return TRUE;}prompt->value=std::move(value);EndDialog(h,IDOK);return TRUE;}break;
 case WM_CLOSE:EndDialog(h,IDCANCEL);return TRUE;
 case WM_ERASEBKGND:{RECT r{};GetClientRect(h,&r);FillRect(reinterpret_cast<HDC>(w),&r,prompt->app.theme.background);return TRUE;}
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:return prompt->app.theme.controlColor(message,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_DRAWITEM:prompt->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;}
 return FALSE;
}
}
void Editor::addWebsite(){
 std::wstring executable=profileIndex>=0?draft.profiles[profileIndex].executable:app.target.executable;
 if(!NativeBrowserContext::supported(executable)){app.error(L"Choose an Edge or Chrome application profile first, then add its website.",window);return;}
 struct alignas(DWORD) Template{DLGTEMPLATE dialog{};WORD menu{},windowClass{};wchar_t title[20]{L"Add website"};} dialog;
 dialog.dialog.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER;dialog.dialog.cx=300;dialog.dialog.cy=112;
 WebsitePrompt prompt{app,app.target.executable==executable?app.target.website:L""};if(DialogBoxIndirectParamW(app.instance,&dialog.dialog,GetAncestor(window,GA_ROOT),websiteProc,reinterpret_cast<LPARAM>(&prompt))!=IDOK)return;
 Profile added{executable,prompt.value,{},{},prompt.value};auto found=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const auto& profile){return sameProfile(profile,added);});
 if(found!=draft.profiles.end()){int index=static_cast<int>(found-draft.profiles.begin());if(selectProfile(index))SendMessageW(profiles,CB_SETCURSEL,index,0);return;}
 auto previous=draft;draft.profiles.push_back(std::move(added));if(!persist()){draft=std::move(previous);return;}profileIndex=static_cast<int>(draft.profiles.size()-1);refreshProfiles();updateWorkflowPage();layout();
}
}
