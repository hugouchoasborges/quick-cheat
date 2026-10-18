#include "app.hpp"
#include "settings.hpp"
#include "workflows.hpp"
#include <algorithm>
#include <initializer_list>
namespace qc {
using namespace settings;
bool Editor::leavePage(){
 if(page==2&&app.workflowEditor&&(app.workflowEditor->dirty||app.workflowEditor->pendingStep)){if(!app.workflowEditor->discard())return false;app.workflowEditor->loadFlow(app.workflowEditor->flowIndex);}
 if(page==1){bool changed=false;auto description=textOf(title),group=textOf(category),combination=textOf(keys);
  if(profileIndex>=0&&shortcutIndex>=0&&shortcutIndex<static_cast<int>(draft.profiles[profileIndex].shortcuts.size())){const auto& saved=draft.profiles[profileIndex].shortcuts[shortcutIndex];changed=description!=saved.title||group!=saved.category||combination!=formatChord(saved.chord);}
  else changed=!description.empty()||!combination.empty();
  if(changed&&MessageBoxW(window,L"Discard unsaved shortcut changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return false;clearFields();
 }
 if(page==0&&textOf(trigger)!=formatChord(draft.trigger)){if(MessageBoxW(window,L"Discard changes to the activation shortcut?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return false;SetWindowTextW(trigger,formatChord(draft.trigger).c_str());}
 return true;
}
bool Editor::updateWorkflowPage(){
 if(page!=2||profileIndex<0)return true;const auto& key=draft.profiles[profileIndex].executable;
 auto saved=std::find_if(app.store.profiles.begin(),app.store.profiles.end(),[&](const auto& profile){return profile.executable==key;});if(saved==app.store.profiles.end()&&!persist())return false;
 if(!app.workflowEditor)app.workflowEditor=std::make_unique<WorkflowEditor>(app);return app.workflowEditor->show(key,window);
}
bool Editor::selectPage(int next){
 if(next<0||next>2)return false;if(next==page){if(!updateWorkflowPage())return false;layout();return true;}
 if(!leavePage())return false;int previous=page;page=next;if(!updateWorkflowPage()){page=previous;layout();return false;}
 layout();SetFocus(GetDlgItem(window,TabGeneral+page));return true;
}
bool Editor::selectProfile(int next){
 if(next==profileIndex)return true;if(next<0||next>=static_cast<int>(draft.profiles.size())||!leavePage()){SendMessageW(profiles,CB_SETCURSEL,profileIndex,0);return false;}
 int previous=profileIndex;profileIndex=next;if(!updateWorkflowPage()){profileIndex=previous;SendMessageW(profiles,CB_SETCURSEL,profileIndex,0);return false;}
 refreshList();layout();return true;
}
void Editor::openWorkflows(const std::wstring& key){
 auto found=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const auto& profile){return profile.executable==key;});if(found==draft.profiles.end()){app.error(L"Choose an application profile first.",window);return;}
 int index=static_cast<int>(found-draft.profiles.begin());if(selectProfile(index)){SendMessageW(profiles,CB_SETCURSEL,index,0);selectPage(2);}
}
void Editor::close(){if(!leavePage())return;recording=false;ShowWindow(window,SW_HIDE);}
void Editor::layout(){
 if(!window)return;UINT dpi=GetDpiForWindow(window);RECT r;GetClientRect(window,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 auto make=[&](int size,int weight){return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");};HFONT next=make(14,FW_NORMAL),nextHeading=make(25,FW_SEMIBOLD),nextCaption=make(12,FW_NORMAL);
 for(HWND c=GetWindow(window,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 SendMessageW(GetDlgItem(window,PageTitle),WM_SETFONT,reinterpret_cast<WPARAM>(nextHeading),TRUE);
 for(int id:std::initializer_list<int>{PageSubtitle,LabelApp,LabelTitle,LabelCategory,LabelKeys,Status,ThemeHelp,TriggerHelp,DataHelp})SendMessageW(GetDlgItem(window,id),WM_SETFONT,reinterpret_cast<WPARAM>(nextCaption),TRUE);
 if(font)DeleteObject(font);if(headingFont)DeleteObject(headingFont);if(captionFont)DeleteObject(captionFont);font=next;headingFont=nextHeading;captionFont=nextCaption;
 auto place=[&](int id,int x,int y,int w,int h){bounds(GetDlgItem(window,id),x,y,w,h,dpi);};auto visible=[&](int id,bool show){ShowWindow(GetDlgItem(window,id),show?SW_SHOWNA:SW_HIDE);};
 for(int id:std::initializer_list<int>{LabelTheme,ThemeSelect,LabelTrigger,Trigger,Apply,ThemeHelp,TriggerHelp,DataLabel,DataPath,DataHelp,OpenData})visible(id,page==0);
 for(int id:std::initializer_list<int>{LabelApp,Profiles,Browse})visible(id,page!=0);
 for(int id:std::initializer_list<int>{Entries,FormTitle,LabelTitle,Title,LabelCategory,Category,LabelKeys,Keys,Record,Save,New,Delete,RemoveProfile})visible(id,page==1);
 visible(CmdManageWorkflows,false);visible(Status,page!=2);visible(EmptyFlows,page==2&&profileIndex<0);
 SetWindowTextW(GetDlgItem(window,PageSubtitle),page==0?L"Make Quick Cheat feel at home.":page==1?L"Organize the keyboard shortcuts you use in each application.":L"Build reusable step-by-step reference guides for each application.");
 for(int id:std::initializer_list<int>{Title,Category,Keys,Record,Save,New,Delete,RemoveProfile})EnableWindow(GetDlgItem(window,id),profileIndex>=0);
 place(PageTitle,28,22,width-56,36);place(PageSubtitle,30,65,width-60,22);
 for(int i=0;i<3;++i){auto tab=GetDlgItem(window,TabGeneral+i);SetPropW(tab,L"QcSelected",reinterpret_cast<HANDLE>(page==i));place(TabGeneral+i,28+i*144,108,136,34);}
 place(LabelTheme,40,190,280,22);place(ThemeHelp,40,218,360,36);place(ThemeSelect,480,194,220,180);
 place(LabelTrigger,40,296,320,22);place(TriggerHelp,40,324,360,40);place(Trigger,484,300,204,28);place(Apply,704,296,92,36);
 place(DataLabel,40,408,280,22);place(DataPath,40,438,width-300,28);place(DataHelp,40,476,width-80,36);place(OpenData,width-220,430,176,36);
 place(LabelApp,28,178,200,20);place(Profiles,28,204,width-260,300);place(Browse,width-208,200,180,34);
 int left=width-420,right=width-348,fieldWidth=320;
 place(Entries,28,278,left,height-376);place(FormTitle,right,272,fieldWidth,26);
 place(LabelTitle,right,316,fieldWidth,20);place(Title,right+4,344,fieldWidth-8,28);
 place(LabelCategory,right,392,fieldWidth,20);place(Category,right,420,fieldWidth,220);
 place(LabelKeys,right,468,fieldWidth,20);place(Keys,right+4,500,fieldWidth-100,28);place(Record,right+fieldWidth-84,496,84,36);
 place(Save,right,550,fieldWidth,38);place(New,right,602,(fieldWidth-12)/2,32);place(Delete,right+(fieldWidth+12)/2,602,(fieldWidth-12)/2,32);
 place(RemoveProfile,28,height-80,176,30);place(Status,28,height-30,width-56,22);place(EmptyFlows,48,304,width-96,80);
 if(app.workflowEditor&&app.workflowEditor->window){bounds(app.workflowEditor->window,0,252,width,height-260,dpi);ShowWindow(app.workflowEditor->window,page==2&&profileIndex>=0?SW_SHOWNA:SW_HIDE);}
 ListView_SetColumnWidth(list,0,MulDiv(left*48/100,dpi,96));ListView_SetColumnWidth(list,1,MulDiv(left*24/100,dpi,96));ListView_SetColumnWidth(list,2,MulDiv(left*27/100,dpi,96));RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
}
void Editor::paint(){
 PAINTSTRUCT ps;HDC dc=BeginPaint(window,&ps);RECT r;GetClientRect(window,&r);FillRect(dc,&r,app.theme.background);auto px=[&](int n){return MulDiv(n,GetDpiForWindow(window),96);};auto& c=app.theme.colors;
 HPEN pen=CreatePen(PS_SOLID,1,c.border);auto old=SelectObject(dc,pen);auto line=[&](int y,int left,int right){MoveToEx(dc,left,px(y),nullptr);LineTo(dc,right,px(y));};line(154,0,r.right);
 if(page==0){line(270,px(40),r.right-px(40));line(382,px(40),r.right-px(40));}
 else{line(252,px(28),r.right-px(28));if(page==1){int x=r.right-px(376);MoveToEx(dc,x,px(278),nullptr);LineTo(dc,x,r.bottom-px(98));}}
 SelectObject(dc,old);DeleteObject(pen);
 for(auto field:{title,keys,trigger})if(IsWindowVisible(field)){RECT bounds{};GetWindowRect(field,&bounds);MapWindowPoints(nullptr,window,reinterpret_cast<POINT*>(&bounds),2);InflateRect(&bounds,px(4),px(4));rounded(dc,bounds,c.field,GetFocus()==field?c.accent:c.border,px(8));}
 EndPaint(window,&ps);
}
}
