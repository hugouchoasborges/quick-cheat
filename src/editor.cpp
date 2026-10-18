#include "app.hpp"
#include <commdlg.h>
#include <algorithm>
#include <set>
namespace qc {
namespace {
enum {Profiles=200,Browse,Entries,Title,Category,Keys,Record,New,Save,Delete,RemoveProfile,Trigger,Apply,Status};
enum {LabelApp=300,LabelTitle,LabelCategory,LabelKeys,LabelTrigger};
}
Editor::Editor(App& value):app(value) {}
Editor::~Editor(){if(window)DestroyWindow(window);if(font)DeleteObject(font);}
void Editor::show() {
 if(!window) {
 WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatEditor";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=GetSysColorBrush(COLOR_BTNFACE);RegisterClassW(&cls);
 window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat — Manage shortcuts",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,920,620,nullptr,nullptr,app.instance,this);
 }
 if(IsWindowVisible(window)){ShowWindow(window,SW_RESTORE);SetForegroundWindow(window);return;}
 draft=app.store;
 if(!app.target.executable.empty()) {
 auto it=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const Profile& p){return p.executable==app.target.executable;});
 if(it==draft.profiles.end()&&draft.profiles.size()<256){draft.profiles.push_back({app.target.executable,app.target.name,{}});profileIndex=static_cast<int>(draft.profiles.size()-1);}else if(it!=draft.profiles.end())profileIndex=static_cast<int>(it-draft.profiles.begin());
 }
 refreshProfiles();SetWindowTextW(trigger,formatChord(draft.trigger).c_str());SetWindowTextW(status,L"Choose an application, then add or edit a shortcut. Changes are saved only when you click Save.");
 ShowWindow(window,SW_SHOW);SetForegroundWindow(window);layout();
}
void Editor::layout() {
 if(!window)return;UINT dpi=GetDpiForWindow(window);RECT r;GetClientRect(window,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 HFONT next=CreateFontW(-MulDiv(14,dpi,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
 for(HWND c=GetWindow(window,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 if(font)DeleteObject(font);font=next;int left=(std::max)(320,width-330),right=left+20,fieldWidth=width-right-20;
 bounds(GetDlgItem(window,LabelApp),20,16,120,20,dpi);bounds(profiles,20,40,width-210,300,dpi);bounds(GetDlgItem(window,Browse),width-178,38,158,30,dpi);
 bounds(list,20,90,left-20,height-216,dpi);
 bounds(GetDlgItem(window,LabelTitle),right,90,fieldWidth,20,dpi);bounds(title,right,114,fieldWidth,28,dpi);
 bounds(GetDlgItem(window,LabelCategory),right,156,fieldWidth,20,dpi);bounds(category,right,180,fieldWidth,220,dpi);
 bounds(GetDlgItem(window,LabelKeys),right,222,fieldWidth,20,dpi);bounds(keys,right,246,fieldWidth-80,28,dpi);bounds(GetDlgItem(window,Record),width-92,245,72,30,dpi);
 bounds(GetDlgItem(window,Save),right,288,fieldWidth,34,dpi);bounds(GetDlgItem(window,New),right,330,(fieldWidth-8)/2,30,dpi);bounds(GetDlgItem(window,Delete),right+(fieldWidth+8)/2,330,(fieldWidth-8)/2,30,dpi);
 bounds(GetDlgItem(window,RemoveProfile),20,height-112,190,28,dpi);
 bounds(GetDlgItem(window,LabelTrigger),20,height-74,160,20,dpi);bounds(trigger,186,height-78,220,28,dpi);bounds(GetDlgItem(window,Apply),416,height-78,90,28,dpi);
 bounds(status,20,height-38,width-40,32,dpi);
 ListView_SetColumnWidth(list,0,MulDiv((left-20)*43/100,dpi,96));ListView_SetColumnWidth(list,1,MulDiv((left-20)*27/100,dpi,96));ListView_SetColumnWidth(list,2,MulDiv((left-20)*26/100,dpi,96));
}
void Editor::refreshProfiles() {
 refreshing=true;SendMessageW(profiles,CB_RESETCONTENT,0,0);
 for(const auto& p:draft.profiles){auto name=p.name+L"  ("+p.executable+L")";SendMessageW(profiles,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));}
 if(profileIndex<0||profileIndex>=static_cast<int>(draft.profiles.size()))profileIndex=draft.profiles.empty()?-1:0;
 SendMessageW(profiles,CB_SETCURSEL,profileIndex,0);refreshing=false;refreshList();
}
void Editor::populateCategories(){auto current=textOf(category);SendMessageW(category,CB_RESETCONTENT,0,0);std::set<std::wstring> groups{L"General"};if(profileIndex>=0)for(const auto& s:draft.profiles[profileIndex].shortcuts)if(!s.category.empty())groups.insert(s.category);for(const auto& group:groups)SendMessageW(category,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(group.c_str()));SetWindowTextW(category,current.c_str());}
void Editor::refreshList() {
 refreshing=true;ListView_DeleteAllItems(list);if(profileIndex>=0){int i=0;for(const auto& s:draft.profiles[profileIndex].shortcuts){
 LVITEMW item{};item.mask=LVIF_TEXT;item.iItem=i;item.pszText=const_cast<wchar_t*>(s.title.c_str());ListView_InsertItem(list,&item);
 ListView_SetItemText(list,i,1,const_cast<wchar_t*>(s.category.c_str()));auto chord=formatChord(s.chord);ListView_SetItemText(list,i,2,chord.data());++i;}}
 refreshing=false;clearFields();populateCategories();
}
void Editor::clearFields(){shortcutIndex=-1;recording=false;SetWindowTextW(GetDlgItem(window,Record),L"Record");SendMessageW(keys,EM_SETREADONLY,FALSE,0);SetWindowTextW(title,L"");SetWindowTextW(category,L"General");SetWindowTextW(keys,L"");ListView_SetItemState(list,-1,0,LVIS_SELECTED);}
void Editor::loadSelection() {
 if(refreshing||profileIndex<0)return;int i=ListView_GetNextItem(list,-1,LVNI_SELECTED);if(i<0)return;shortcutIndex=i;
 const auto& s=draft.profiles[profileIndex].shortcuts[i];SetWindowTextW(title,s.title.c_str());SetWindowTextW(category,s.category.c_str());SetWindowTextW(keys,formatChord(s.chord).c_str());
}
void Editor::addProfile() {
 wchar_t file[32768]{};OPENFILENAMEW dialog{sizeof(dialog)};dialog.hwndOwner=window;dialog.lpstrFilter=L"Applications (*.exe)\0*.exe\0\0";dialog.lpstrFile=file;dialog.nMaxFile=32768;dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetOpenFileNameW(&dialog))return;auto exe=appKey(file);
 auto it=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const Profile& p){return p.executable==exe;});
 if(it!=draft.profiles.end())profileIndex=static_cast<int>(it-draft.profiles.begin());else if(draft.profiles.size()<256){draft.profiles.push_back({exe,exe.substr(0,exe.size()-4),{}});profileIndex=static_cast<int>(draft.profiles.size()-1);}else{app.error(L"The maximum of 256 application profiles has been reached.",window);return;}
 if(!persist())draft=app.store;refreshProfiles();
}
bool Editor::persist(){if(!app.commit(draft))return false;SetWindowTextW(status,L"Saved locally. Activate the application and press your activation shortcut to open its popup.");return true;}
void Editor::saveShortcut() {
 if(profileIndex<0){app.error(L"Choose an application executable first.",window);return;}
 auto chord=parseChord(textOf(keys));if(!chord){app.error(L"Enter a combination such as Ctrl+S, Ctrl+Shift+F10 or Alt+Enter. Use Plus for the + key.",window);return;}
 if(chord->key==draft.trigger.key&&chord->modifiers==draft.trigger.modifiers){app.error(L"This combination is reserved for Quick Cheat activation. Choose another shortcut or change activation.",window);return;}
 Shortcut s{textOf(title),textOf(category),*chord};auto original=draft;
 auto& shortcuts=draft.profiles[profileIndex].shortcuts;
 if(shortcutIndex>=0)shortcuts[shortcutIndex]=std::move(s);else shortcuts.push_back(std::move(s));
 if(!validStore(draft)){draft=std::move(original);app.error(L"Add a description (up to 240 characters) and a category (up to 80 characters). The profile supports up to 2048 shortcuts.",window);return;}
 if(!persist()){draft=std::move(original);return;}refreshList();
}
void Editor::deleteShortcut(){if(profileIndex<0||shortcutIndex<0)return;if(MessageBoxW(window,L"Delete the selected shortcut?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;auto old=draft;auto& s=draft.profiles[profileIndex].shortcuts;s.erase(s.begin()+shortcutIndex);if(!persist())draft=std::move(old);refreshList();}
void Editor::deleteProfile(){if(profileIndex<0)return;if(MessageBoxW(window,L"Delete this application profile and all its shortcuts?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;auto old=draft;draft.profiles.erase(draft.profiles.begin()+profileIndex);if(!persist())draft=std::move(old);profileIndex=-1;refreshProfiles();}
void Editor::applyTrigger(){auto chord=parseChord(textOf(trigger));if(!chord||!chord->modifiers||chord->key==VK_F12){app.error(L"Activation requires a modifier (Ctrl, Alt, Shift or Win) and a key other than F12.",window);return;}auto old=draft.trigger;draft.trigger=*chord;if(!persist())draft.trigger=old;SetWindowTextW(trigger,formatChord(draft.trigger).c_str());}
LRESULT CALLBACK Editor::captureProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value) {
 auto* e=reinterpret_cast<Editor*>(value);
 if((msg==WM_CHAR||msg==WM_SYSCHAR||msg==WM_DEADCHAR||msg==WM_SYSDEADCHAR)&&e->suppressChar){e->suppressChar=false;return 0;}
 if(!e->recording&&(msg==WM_KEYDOWN||msg==WM_SYSKEYDOWN))e->suppressChar=false;
 if(e->recording&&(msg==WM_KEYDOWN||msg==WM_SYSKEYDOWN)) {
 if(w==VK_ESCAPE){e->recording=false;SendMessageW(h,EM_SETREADONLY,FALSE,0);SetWindowTextW(GetDlgItem(e->window,Record),L"Record");return 0;}
 Chord chord{};if(GetKeyState(VK_CONTROL)<0)chord.modifiers|=MOD_CONTROL;if(GetKeyState(VK_MENU)<0)chord.modifiers|=MOD_ALT;if(GetKeyState(VK_SHIFT)<0)chord.modifiers|=MOD_SHIFT;if(GetKeyState(VK_LWIN)<0||GetKeyState(VK_RWIN)<0)chord.modifiers|=MOD_WIN;chord.key=static_cast<WORD>(w);
 if(!formatChord(chord).empty()){SetWindowTextW(h,formatChord(chord).c_str());e->recording=false;e->suppressChar=MapVirtualKeyW(static_cast<UINT>(w),MAPVK_VK_TO_CHAR)!=0;SendMessageW(h,EM_SETREADONLY,FALSE,0);SetWindowTextW(GetDlgItem(e->window,Record),L"Record");}return 0;
 }
 if(e->recording&&msg==WM_GETDLGCODE)return DLGC_WANTALLKEYS;
 if(e->recording&&(msg==WM_CHAR||msg==WM_SYSCHAR))return 0;
 return DefSubclassProc(h,msg,w,l);
}
LRESULT CALLBACK Editor::proc(HWND h,UINT msg,WPARAM w,LPARAM l) {
 auto* e=reinterpret_cast<Editor*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){e=static_cast<Editor*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(e));e->window=h;}
 if(!e)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_CREATE:{
 control(h,L"STATIC",L"Application",0,LabelApp);e->profiles=control(h,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,Profiles);control(h,L"BUTTON",L"Browse application...",WS_TABSTOP,Browse);
 e->list=control(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS|WS_TABSTOP,Entries);ListView_SetExtendedListViewStyle(e->list,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
 const wchar_t* columns[]={L"Description",L"Category",L"Keys"};for(int i=0;i<3;++i){LVCOLUMNW c{};c.mask=LVCF_TEXT|LVCF_WIDTH;c.cx=150;c.pszText=const_cast<wchar_t*>(columns[i]);ListView_InsertColumn(e->list,i,&c);}
 control(h,L"STATIC",L"Description",0,LabelTitle);e->title=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Title);SendMessageW(e->title,EM_SETLIMITTEXT,240,0);
 control(h,L"STATIC",L"Category (choose or type)",0,LabelCategory);e->category=control(h,L"COMBOBOX",L"",CBS_DROPDOWN|CBS_AUTOHSCROLL|WS_VSCROLL|WS_TABSTOP,Category);SendMessageW(e->category,CB_LIMITTEXT,80,0);
 control(h,L"STATIC",L"Keyboard shortcut",0,LabelKeys);e->keys=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Keys);SendMessageW(e->keys,EM_SETLIMITTEXT,100,0);SetWindowSubclass(e->keys,captureProc,1,reinterpret_cast<DWORD_PTR>(e));control(h,L"BUTTON",L"Record",WS_TABSTOP,Record);
 control(h,L"BUTTON",L"Save shortcut",WS_TABSTOP|BS_DEFPUSHBUTTON,Save);control(h,L"BUTTON",L"New shortcut",WS_TABSTOP,New);control(h,L"BUTTON",L"Delete",WS_TABSTOP,Delete);control(h,L"BUTTON",L"Remove application",WS_TABSTOP,RemoveProfile);
 control(h,L"STATIC",L"Activation shortcut",0,LabelTrigger);e->trigger=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Trigger);SendMessageW(e->trigger,EM_SETLIMITTEXT,100,0);control(h,L"BUTTON",L"Apply",WS_TABSTOP,Apply);
 e->status=control(h,L"STATIC",L"",0,Status);return 0;}
 case WM_SIZE:e->layout();return 0;
 case WM_GETMINMAXINFO:{auto* info=reinterpret_cast<MINMAXINFO*>(l);UINT dpi=GetDpiForWindow(h);info->ptMinTrackSize={MulDiv(820,dpi,96),MulDiv(540,dpi,96)};return 0;}
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);e->layout();return 0;}
 case WM_NOTIFY:if(reinterpret_cast<NMHDR*>(l)->idFrom==Entries&&reinterpret_cast<NMHDR*>(l)->code==LVN_ITEMCHANGED)e->loadSelection();return 0;
 case WM_COMMAND:switch(LOWORD(w)){
 case Profiles:if(HIWORD(w)==CBN_SELCHANGE&&!e->refreshing){e->profileIndex=static_cast<int>(SendMessageW(e->profiles,CB_GETCURSEL,0,0));e->refreshList();}break;
 case Browse:e->addProfile();break;case Save:e->saveShortcut();break;case New:e->clearFields();SetFocus(e->title);break;case Delete:e->deleteShortcut();break;case RemoveProfile:e->deleteProfile();break;case Apply:e->applyTrigger();break;
 case Record:e->recording=true;SetWindowTextW(GetDlgItem(h,Record),L"Press...");SendMessageW(e->keys,EM_SETREADONLY,TRUE,0);SetFocus(e->keys);break;
 case IDOK:e->saveShortcut();break;case IDCANCEL:ShowWindow(h,SW_HIDE);break;}return 0;
 case WM_CLOSE:e->recording=false;ShowWindow(h,SW_HIDE);return 0;
 }return DefWindowProcW(h,msg,w,l);
}
}
