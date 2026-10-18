#include "app.hpp"
#include "settings.hpp"
#include "workflows.hpp"
#include "dock.hpp"
#include <commdlg.h>
#include <shellapi.h>
#include <algorithm>
#include <set>
namespace qc {
using namespace settings;
Editor::Editor(App& value):app(value) {}
Editor::~Editor(){if(window)DestroyWindow(window);if(font)DeleteObject(font);if(headingFont)DeleteObject(headingFont);if(captionFont)DeleteObject(captionFont);}
void Editor::show() {
 if(!window) {
 WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatEditor";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(app.instance,MAKEINTRESOURCEW(1));RegisterClassW(&cls);
 window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat — Settings",WS_CHILD|WS_CLIPCHILDREN|WS_VSCROLL|WS_HSCROLL,0,0,0,0,app.popup,reinterpret_cast<HMENU>(231),app.instance,this);
 }
 if(app.settingsVisible)return;
 draft=app.store;
 if(!app.target.executable.empty()) {
 auto it=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const Profile& p){return p.executable==app.target.executable&&p.website==app.target.website;});
 if(it==draft.profiles.end()&&draft.profiles.size()<256){draft.profiles.push_back({app.target.executable,app.target.website.empty()?app.target.name:app.target.website,{},{},app.target.website});profileIndex=static_cast<int>(draft.profiles.size()-1);}else if(it!=draft.profiles.end())profileIndex=static_cast<int>(it-draft.profiles.begin());
 }
 refreshProfiles();SetWindowTextW(trigger,formatChord(draft.trigger).c_str());SendMessageW(themeSelect,CB_SETCURSEL,static_cast<UINT>(draft.theme),0);SetWindowTextW(status,L"Stored on this computer. Select a shortcut to edit, or create a new one.");
 updateWorkflowPage();app.theme.style(window);ShowWindow(window,SW_SHOW);layout();
}
void Editor::refreshProfiles() {
 refreshing=true;SendMessageW(profiles,CB_RESETCONTENT,0,0);
 for(const auto& p:draft.profiles){auto name=p.name+L"  ("+p.executable+(p.website.empty()?L"":L" / "+p.website)+L")";SendMessageW(profiles,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));}
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
 auto it=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const Profile& p){return p.executable==exe&&p.website.empty();});
 if(it!=draft.profiles.end())profileIndex=static_cast<int>(it-draft.profiles.begin());else if(draft.profiles.size()<256){draft.profiles.push_back({exe,exe.substr(0,exe.size()-4),{}});profileIndex=static_cast<int>(draft.profiles.size()-1);}else{app.error(L"The maximum of 256 application profiles has been reached.",window);return;}
 if(!persist())draft=app.store;refreshProfiles();
}
bool Editor::persist(){if(!app.commit(draft))return false;SetWindowTextW(status,L"Saved locally.");updateCollectionActions();return true;}
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
void Editor::deleteShortcut(){if(profileIndex<0||shortcutIndex<0)return;if(MessageBoxW(window,L"Delete the selected shortcut?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;auto old=draft;removeDockShortcut(draft.profiles[profileIndex],static_cast<size_t>(shortcutIndex));auto& s=draft.profiles[profileIndex].shortcuts;s.erase(s.begin()+shortcutIndex);if(!persist())draft=std::move(old);refreshList();}
void Editor::deleteProfile(){if(profileIndex<0)return;if(MessageBoxW(window,L"Delete this application profile, all its shortcuts and workflows?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;auto old=draft;draft.profiles.erase(draft.profiles.begin()+profileIndex);if(!persist())draft=std::move(old);profileIndex=-1;refreshProfiles();}
void Editor::applyTrigger(){auto chord=parseChord(textOf(trigger));if(!chord||!chord->modifiers||chord->key==VK_F12){app.error(L"Activation requires a modifier (Ctrl, Alt, Shift or Win) and a key other than F12.",window);return;}auto old=draft.trigger;draft.trigger=*chord;if(!persist())draft.trigger=old;SetWindowTextW(trigger,formatChord(draft.trigger).c_str());}
void Editor::applyAppearance(){auto selected=SendMessageW(themeSelect,CB_GETCURSEL,0,0);if(selected<0||selected>2)return;auto old=draft.theme;draft.theme=static_cast<ThemeMode>(selected);if(!persist())draft.theme=old;SendMessageW(themeSelect,CB_SETCURSEL,static_cast<UINT>(draft.theme),0);}
void Editor::drawEntry(DRAWITEMSTRUCT* item){if(profileIndex<0||item->itemID>=draft.profiles[profileIndex].shortcuts.size())return;auto& s=draft.profiles[profileIndex].shortcuts[item->itemID];auto& c=app.theme.colors;HDC dc=item->hDC;RECT r=item->rcItem;FillRect(dc,&r,app.theme.background);if(item->itemState&ODS_SELECTED){RECT selected=r;InflateRect(&selected,-2,-2);rounded(dc,selected,c.selection,c.selection,8);}auto old=SelectObject(dc,font);std::wstring texts[]={s.title,s.category,formatChord(s.chord)};int x=r.left;for(int i=0;i<3;++i){RECT cell{x+12,r.top,x+ListView_GetColumnWidth(list,i)-8,r.bottom};paintText(dc,texts[i],cell,i==0?c.text:c.muted);x+=ListView_GetColumnWidth(list,i);}SelectObject(dc,old);}
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
 control(h,L"STATIC",L"Settings",0,PageTitle);control(h,L"STATIC",L"Preferences and local libraries.",0,PageSubtitle);control(h,L"STATIC",L"Appearance",0,LabelTheme);
 control(h,L"BUTTON",L"General",WS_TABSTOP,TabGeneral);control(h,L"BUTTON",L"Shortcuts",WS_TABSTOP,TabShortcuts);control(h,L"BUTTON",L"Workflows",WS_TABSTOP,TabWorkflows);control(h,L"BUTTON",L"Dock",WS_TABSTOP,TabDock);
 control(h,L"STATIC",L"Choose a theme, or follow your Windows appearance.",0,ThemeHelp);
 control(h,L"STATIC",L"The keyboard combination that opens Quick Cheat in any application.",0,TriggerHelp);
 control(h,L"STATIC",L"Local data",0,DataLabel);control(h,L"STATIC",e->app.file.parent_path().c_str(),SS_PATHELLIPSIS,DataPath);
 control(h,L"STATIC",L"Everything stays on this computer. Back up this folder to keep your shortcuts, workflows, images and category preferences.",0,DataHelp);control(h,L"BUTTON",L"Open data folder",WS_TABSTOP,OpenData);
 control(h,L"STATIC",L"Shortcut details",0,FormTitle);control(h,L"STATIC",L"Choose an application above, or browse to add one, to create workflows.",SS_CENTER,EmptyFlows);
 for(int id:{LabelTheme,LabelTrigger,DataLabel,FormTitle})SetPropW(GetDlgItem(h,id),L"QcHeading",reinterpret_cast<HANDLE>(1));
 e->themeSelect=control(h,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_TABSTOP,ThemeSelect);for(auto label:{L"System",L"Light",L"Dark"})SendMessageW(e->themeSelect,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(label));
 control(h,L"STATIC",L"Application",0,LabelApp);e->profiles=control(h,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,Profiles);control(h,L"BUTTON",L"Browse application...",WS_TABSTOP,Browse);
 control(h,L"BUTTON",L"Import collection...",WS_TABSTOP,ImportCollection);control(h,L"BUTTON",L"Export collection...",WS_TABSTOP,ExportCollection);
 e->list=control(h,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_OWNERDRAWFIXED|WS_TABSTOP,Entries);ListView_SetExtendedListViewStyle(e->list,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
 const wchar_t* columns[]={L"Description",L"Category",L"Keys"};for(int i=0;i<3;++i){LVCOLUMNW c{};c.mask=LVCF_TEXT|LVCF_WIDTH;c.cx=150;c.pszText=const_cast<wchar_t*>(columns[i]);ListView_InsertColumn(e->list,i,&c);}
 control(h,L"STATIC",L"Description",0,LabelTitle);e->title=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Title);SendMessageW(e->title,EM_SETLIMITTEXT,240,0);
 control(h,L"STATIC",L"Category (choose or type)",0,LabelCategory);e->category=control(h,L"COMBOBOX",L"",CBS_DROPDOWN|CBS_AUTOHSCROLL|WS_VSCROLL|WS_TABSTOP,Category);SendMessageW(e->category,CB_LIMITTEXT,80,0);
 control(h,L"STATIC",L"Keyboard shortcut",0,LabelKeys);e->keys=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Keys);SendMessageW(e->keys,EM_SETLIMITTEXT,100,0);SetWindowSubclass(e->keys,captureProc,1,reinterpret_cast<DWORD_PTR>(e));control(h,L"BUTTON",L"Record",WS_TABSTOP,Record);
 control(h,L"BUTTON",L"Save shortcut",WS_TABSTOP,Save);control(h,L"BUTTON",L"New shortcut",WS_TABSTOP,New);control(h,L"BUTTON",L"Delete",WS_TABSTOP,Delete);control(h,L"BUTTON",L"Remove application",WS_TABSTOP,RemoveProfile);
 control(h,L"STATIC",L"Activation shortcut",0,LabelTrigger);e->trigger=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,Trigger);SendMessageW(e->trigger,EM_SETLIMITTEXT,100,0);control(h,L"BUTTON",L"Apply",WS_TABSTOP,Apply);
 control(h,L"BUTTON",L"Website...",WS_TABSTOP,Website);control(h,L"STATIC",L"Website detection: Edge and Chrome via Windows accessibility. No extension required. Unavailable domains use browser shortcuts only.",0,BrowserHelp);e->status=control(h,L"STATIC",L"",0,Status);return 0;}
 case WM_SIZE:e->layout();return 0;
 case WM_VSCROLL:case WM_HSCROLL:e->scroll(msg,w);return 0;
 case WM_MOUSEWHEEL:e->scroll(WM_VSCROLL,MAKEWPARAM(GET_WHEEL_DELTA_WPARAM(w)>0?SB_PAGEUP:SB_PAGEDOWN,0));return 0;
 case WM_PAINT:e->paint();return 0;
 case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect(reinterpret_cast<HDC>(w),&r,e->app.theme.background);return 1;}
 case WM_CTLCOLORSTATIC:e->app.theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));return reinterpret_cast<LRESULT>(e->app.theme.background);
 case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return e->app.theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_DRAWITEM:{auto* item=reinterpret_cast<DRAWITEMSTRUCT*>(l);if(item->CtlID==Entries)e->drawEntry(item);else e->app.theme.drawControl(item);return TRUE;}
 case WM_MEASUREITEM:reinterpret_cast<MEASUREITEMSTRUCT*>(l)->itemHeight=MulDiv(38,GetDpiForWindow(h),96);return TRUE;
 case WM_GETMINMAXINFO:{auto* info=reinterpret_cast<MINMAXINFO*>(l);UINT dpi=GetDpiForWindow(h);info->ptMinTrackSize={MulDiv(980,dpi,96),MulDiv(780,dpi,96)};return 0;}
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);e->layout();return 0;}
 case WM_NOTIFY:if(reinterpret_cast<NMHDR*>(l)->idFrom==Entries&&reinterpret_cast<NMHDR*>(l)->code==LVN_ITEMCHANGED)e->loadSelection();return 0;
 case WM_COMMAND:switch(LOWORD(w)){
 case CmdManageWorkflows:case TabWorkflows:e->selectPage(2);break;
 case TabDock:e->selectPage(3);break;case Website:if(e->leavePage())e->addWebsite();break;case TabGeneral:e->selectPage(0);break;case TabShortcuts:e->selectPage(1);break;
 case OpenData:ShellExecuteW(h,L"open",e->app.file.parent_path().c_str(),nullptr,nullptr,SW_SHOWNORMAL);break;
 case ImportCollection:e->importApplicationCollection();break;case ExportCollection:e->exportApplicationCollection();break;
 case ThemeSelect:if(HIWORD(w)==CBN_SELCHANGE&&!e->refreshing)e->applyAppearance();break;
 case Profiles:if(HIWORD(w)==CBN_SELCHANGE&&!e->refreshing)e->selectProfile(static_cast<int>(SendMessageW(e->profiles,CB_GETCURSEL,0,0)));break;
 case Browse:if(e->leavePage()){e->addProfile();e->updateWorkflowPage();e->layout();}break;case Save:e->saveShortcut();break;case New:e->clearFields();SetFocus(e->title);break;case Delete:e->deleteShortcut();break;case RemoveProfile:e->deleteProfile();break;case Apply:e->applyTrigger();break;
 case Record:e->recording=true;SetWindowTextW(GetDlgItem(h,Record),L"Press...");SendMessageW(e->keys,EM_SETREADONLY,TRUE,0);SetFocus(e->keys);break;
 case IDOK:if(e->page==0)e->applyTrigger();else if(e->page==1)e->saveShortcut();else if(e->page==2&&e->app.workflowEditor)e->app.workflowEditor->save();else if(e->page==3&&e->dockSettings)e->dockSettings->save();break;case IDCANCEL:e->close();break;}return 0;
 case WM_CLOSE:e->close();return 0;
 }return DefWindowProcW(h,msg,w,l);
}
}
