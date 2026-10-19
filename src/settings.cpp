#include "app.hpp"
#include "settings.hpp"
#include "workflows.hpp"
#include "dock.hpp"
#include "browser_context.hpp"
#include <algorithm>
#include <initializer_list>
namespace qc {
using namespace settings;
bool Editor::pendingPageEdits() const{
 if(page==3)return dockSettings&&dockSettings->pending();
 if(page==2)return app.workflowEditor&&(app.workflowEditor->dirty||app.workflowEditor->pendingStep);
 if(page==0)return textOf(trigger)!=formatChord(draft.trigger);
 auto description=textOf(title),group=textOf(category),combination=textOf(keys),dockLabel=textOf(GetDlgItem(window,DockLabel));bool url=SendMessageW(GetDlgItem(window,ActionType),CB_GETCURSEL,0,0)==1;
 if(profileIndex>=0&&shortcutIndex>=0&&shortcutIndex<static_cast<int>(draft.profiles[profileIndex].shortcuts.size())){const auto& saved=draft.profiles[profileIndex].shortcuts[shortcutIndex];return description!=saved.title||group!=saved.category||combination!=(saved.url.empty()?formatChord(saved.chord):saved.url)||url!=(!saved.url.empty())||dockLabel!=saved.dockLabel;}
 return !description.empty()||!combination.empty()||!dockLabel.empty()||url;
}
bool Editor::leavePage(){
 if(page==3&&dockSettings&&!dockSettings->discard())return false;
 if(page==2&&app.workflowEditor&&(app.workflowEditor->dirty||app.workflowEditor->pendingStep)){if(!app.workflowEditor->discard())return false;app.workflowEditor->loadFlow(app.workflowEditor->flowIndex);}
 if(page==1){
  if(pendingPageEdits()&&MessageBoxW(window,L"Discard unsaved shortcut changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return false;clearFields();
 }
 if(page==0&&textOf(trigger)!=formatChord(draft.trigger)){if(MessageBoxW(window,L"Discard changes to the activation shortcut?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return false;SetWindowTextW(trigger,formatChord(draft.trigger).c_str());}
 return true;
}
bool Editor::updateWorkflowPage(){
 if((page!=2&&page!=3)||profileIndex<0)return true;auto key=profileKey(draft.profiles[profileIndex]);
 if(!findProfile(app.store,key)&&!persist())return false;
 if(page==3){if(!dockSettings)dockSettings=std::make_unique<DockSettings>(app,window);return dockSettings->show(key);}
 if(!app.workflowEditor)app.workflowEditor=std::make_unique<WorkflowEditor>(app);return app.workflowEditor->show(key,window);
}
bool Editor::selectPage(int next){
 if(next<0||next>3)return false;if(next==page){if(!updateWorkflowPage())return false;layout();return true;}
 if(!leavePage())return false;int previous=page;page=next;if(!updateWorkflowPage()){page=previous;layout();return false;}
 layout();SetFocus(GetDlgItem(window,tabId(page)));return true;
}
bool Editor::selectProfile(int next){
 if(next==profileIndex)return true;if(next<0||next>=static_cast<int>(draft.profiles.size())||!leavePage()){SendMessageW(profiles,CB_SETCURSEL,profileIndex,0);return false;}
 int previous=profileIndex;profileIndex=next;if(!updateWorkflowPage()){profileIndex=previous;SendMessageW(profiles,CB_SETCURSEL,profileIndex,0);return false;}
 refreshList();layout();return true;
}
void Editor::openWorkflows(const std::wstring& key){
 auto found=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const auto& profile){return profileKey(profile)==key;});if(found==draft.profiles.end()){app.error(L"Choose an application profile first.",window);return;}
 int index=static_cast<int>(found-draft.profiles.begin());if(selectProfile(index)){SendMessageW(profiles,CB_SETCURSEL,index,0);selectPage(2);}
}
void Editor::close(){app.hidePopup();}
void Editor::layout(){
 if(!window)return;UINT dpi=GetDpiForWindow(window);RECT r;GetClientRect(window,&r);int viewWidth=MulDiv(r.right,96,dpi),viewHeight=MulDiv(r.bottom,96,dpi);
 int width=(std::max)(viewWidth,980),height=(std::max)(viewHeight,page==1?810:714)+66;
 scrollX=std::clamp(scrollX,0,(std::max)(0,width-viewWidth));scrollY=std::clamp(scrollY,0,(std::max)(0,height-66-viewHeight));
 SCROLLINFO horizontal{sizeof(horizontal),SIF_RANGE|SIF_PAGE|SIF_POS,0,width-1,static_cast<UINT>(viewWidth),scrollX};SetScrollInfo(window,SB_HORZ,&horizontal,TRUE);
 SCROLLINFO vertical{sizeof(vertical),SIF_RANGE|SIF_PAGE|SIF_POS,0,height-67,static_cast<UINT>(viewHeight),scrollY};SetScrollInfo(window,SB_VERT,&vertical,TRUE);
 auto make=[&](int size,int weight){return CreateFontW(-MulDiv(size,dpi,96),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");};HFONT next=make(14,FW_NORMAL),nextHeading=make(25,FW_SEMIBOLD),nextCaption=make(12,FW_NORMAL);
 for(HWND c=GetWindow(window,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);
 SendMessageW(GetDlgItem(window,PageTitle),WM_SETFONT,reinterpret_cast<WPARAM>(nextHeading),TRUE);
 for(int id:std::initializer_list<int>{PageSubtitle,LabelApp,LabelTitle,LabelCategory,LabelKeys,Status,ThemeHelp,TriggerHelp,DataHelp,LabelActionType,LabelDockLabel})SendMessageW(GetDlgItem(window,id),WM_SETFONT,reinterpret_cast<WPARAM>(nextCaption),TRUE);
 if(font)DeleteObject(font);if(headingFont)DeleteObject(headingFont);if(captionFont)DeleteObject(captionFont);font=next;headingFont=nextHeading;captionFont=nextCaption;
 auto place=[&](int id,int x,int y,int w,int h){bounds(GetDlgItem(window,id),x-scrollX,y-66-scrollY,w,h,dpi);};auto visible=[&](int id,bool show){ShowWindow(GetDlgItem(window,id),show?SW_SHOWNA:SW_HIDE);};
 visible(PageTitle,false);visible(PageSubtitle,false);
 for(int id:std::initializer_list<int>{LabelTheme,ThemeSelect,LabelTrigger,Trigger,Apply,ThemeHelp,TriggerHelp,DataLabel,DataPath,DataHelp,OpenData,BrowserHelp})visible(id,page==0);
 for(int id:std::initializer_list<int>{LabelApp,Profiles,Browse,ImportCollection,ExportCollection,Website})visible(id,page!=0);
 for(int id:std::initializer_list<int>{Entries,FormTitle,LabelTitle,Title,LabelCategory,Category,LabelKeys,Keys,Record,Save,New,Delete,RemoveProfile})visible(id,page==1);
 visible(Status,page!=2&&page!=3);visible(EmptyFlows,(page==2||page==3)&&profileIndex<0);
 SetWindowTextW(GetDlgItem(window,PageSubtitle),page==0?L"Make Quick Cheat feel at home.":page==1?L"Organize the keyboard shortcuts you use in each application.":L"Build reusable step-by-step reference guides for each application.");
 for(int id:std::initializer_list<int>{Title,Category,Keys,Record,Save,New,Delete,RemoveProfile})EnableWindow(GetDlgItem(window,id),profileIndex>=0);
 updateCollectionActions();
 place(PageTitle,28,22,width-56,36);place(PageSubtitle,30,65,width-60,22);
 for(int i=0;i<4;++i){auto tab=GetDlgItem(window,tabId(i));SetPropW(tab,L"QcSelected",reinterpret_cast<HANDLE>(page==i));place(tabId(i),28+i*144,108,136,34);}
 place(LabelTheme,40,190,280,22);place(ThemeHelp,40,218,360,36);place(ThemeSelect,480,194,220,180);
 place(LabelTrigger,40,296,320,22);place(TriggerHelp,40,324,360,40);place(Trigger,484,300,204,28);place(Apply,704,296,92,36);
 place(DataLabel,40,408,280,22);place(DataPath,40,438,width-300,28);place(DataHelp,40,476,width-80,36);place(OpenData,width-220,430,176,36);
 place(LabelApp,28,178,width-240,20);place(Website,width-184,158,156,30);EnableWindow(GetDlgItem(window,Website),profileIndex>=0&&NativeBrowserContext::supported(draft.profiles[profileIndex].executable));place(BrowserHelp,40,550,width-80,60);place(Profiles,28,204,width-576,300);place(Browse,width-520,200,160,34);place(ImportCollection,width-352,200,160,34);place(ExportCollection,width-184,200,156,34);
 int left=width-420,right=width-348,fieldWidth=320;
 place(Entries,28,278,left,height-376);place(FormTitle,right,272,fieldWidth,26);
 place(LabelTitle,right,316,fieldWidth,20);place(Title,right+4,344,fieldWidth-8,28);
 place(LabelCategory,right,392,fieldWidth,20);place(Category,right,420,fieldWidth,220);
 place(LabelActionType,right,462,fieldWidth,20);place(ActionType,right,486,fieldWidth,180);
 bool url=SendMessageW(GetDlgItem(window,ActionType),CB_GETCURSEL,0,0)==1;
 place(LabelKeys,right,530,fieldWidth,20);place(Keys,right+4,558,fieldWidth-(url?8:100),28);place(Record,right+fieldWidth-84,554,84,36);visible(Record,page==1&&!url);
 place(LabelDockLabel,right,602,fieldWidth,20);place(DockLabel,right+4,628,fieldWidth-8,28);
 place(Save,right,678,fieldWidth,38);place(New,right,730,(fieldWidth-12)/2,32);place(Delete,right+(fieldWidth+12)/2,730,(fieldWidth-12)/2,32);
 for(int id:{LabelActionType,ActionType,LabelDockLabel,DockLabel})visible(id,page==1);EnableWindow(GetDlgItem(window,ActionType),profileIndex>=0&&!draft.profiles[profileIndex].website.empty());EnableWindow(GetDlgItem(window,DockLabel),profileIndex>=0);
 place(RemoveProfile,28,height-80,176,30);place(Status,28,height-30,width-56,22);place(EmptyFlows,48,304,width-96,80);
 if(app.workflowEditor&&app.workflowEditor->window){bounds(app.workflowEditor->window,-scrollX,186-scrollY,width,height-260,dpi);ShowWindow(app.workflowEditor->window,page==2&&profileIndex>=0?SW_SHOWNA:SW_HIDE);}
 if(dockSettings){if(page==3&&profileIndex>=0)dockSettings->layout(MulDiv(28-scrollX,dpi,96),MulDiv(186-scrollY,dpi,96),MulDiv(width-56,dpi,96),MulDiv(height-260,dpi,96),dpi);else dockSettings->hide();}
 ListView_SetColumnWidth(list,0,MulDiv(left*48/100,dpi,96));ListView_SetColumnWidth(list,1,MulDiv(left*24/100,dpi,96));ListView_SetColumnWidth(list,2,MulDiv(left*27/100,dpi,96));RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
}
void Editor::paint(){
 PAINTSTRUCT ps;HDC dc=BeginPaint(window,&ps);RECT r;GetClientRect(window,&r);FillRect(dc,&r,app.theme.background);auto px=[&](int n){return MulDiv(n,GetDpiForWindow(window),96);};auto& c=app.theme.colors;
 SetViewportOrgEx(dc,-px(scrollX),-px(scrollY+66),nullptr);r.right=px((std::max)(MulDiv(r.right,96,GetDpiForWindow(window)),980));r.bottom=px((std::max)(MulDiv(r.bottom,96,GetDpiForWindow(window)),714)+66);
 HPEN pen=CreatePen(PS_SOLID,1,c.border);auto old=SelectObject(dc,pen);auto line=[&](int y,int left,int right){MoveToEx(dc,left,px(y),nullptr);LineTo(dc,right,px(y));};line(154,0,r.right);
 if(page==0){line(270,px(40),r.right-px(40));line(382,px(40),r.right-px(40));}
 else{line(252,px(28),r.right-px(28));if(page==1){int x=r.right-px(376);MoveToEx(dc,x,px(278),nullptr);LineTo(dc,x,r.bottom-px(98));}}
 SelectObject(dc,old);DeleteObject(pen);
 for(auto field:{title,keys,trigger,GetDlgItem(window,DockLabel)})if(IsWindowVisible(field)){RECT bounds{};GetWindowRect(field,&bounds);MapWindowPoints(nullptr,window,reinterpret_cast<POINT*>(&bounds),2);OffsetRect(&bounds,px(scrollX),px(scrollY+66));InflateRect(&bounds,px(4),px(4));rounded(dc,bounds,c.field,GetFocus()==field?c.accent:c.border,px(8));}
 EndPaint(window,&ps);
}
void Editor::scroll(UINT message,WPARAM value){
 int bar=message==WM_HSCROLL?SB_HORZ:SB_VERT;SCROLLINFO info{sizeof(info),SIF_ALL};GetScrollInfo(window,bar,&info);int position=info.nPos;
 switch(LOWORD(value)){case SB_LINEUP:position-=32;break;case SB_LINEDOWN:position+=32;break;case SB_PAGEUP:position-=static_cast<int>(info.nPage);break;case SB_PAGEDOWN:position+=static_cast<int>(info.nPage);break;case SB_THUMBTRACK:case SB_THUMBPOSITION:position=info.nTrackPos;break;case SB_TOP:position=0;break;case SB_BOTTOM:position=info.nMax;break;default:return;}
 (bar==SB_HORZ?scrollX:scrollY)=std::clamp(position,0,(std::max)(0,info.nMax-static_cast<int>(info.nPage)+1));layout();
}
void Editor::revealFocus(){
 HWND focus=GetFocus();if(!focus||!IsChild(window,focus))return;RECT field{},view{};GetWindowRect(focus,&field);MapWindowPoints(nullptr,window,reinterpret_cast<POINT*>(&field),2);GetClientRect(window,&view);UINT dpi=GetDpiForWindow(window);
 int dx=field.left<0?field.left:field.right>view.right?field.right-view.right:0,dy=field.top<0?field.top:field.bottom>view.bottom?field.bottom-view.bottom:0;
 if(dx||dy){scrollX+=MulDiv(dx,96,dpi);scrollY+=MulDiv(dy,96,dpi);layout();}
}
}

void qc::Editor::updateCollectionActions(){bool available=profileIndex>=0&&static_cast<size_t>(profileIndex)<draft.profiles.size()&&(!draft.profiles[profileIndex].shortcuts.empty()||!draft.profiles[profileIndex].workflows.empty());if(window)EnableWindow(GetDlgItem(window,settings::ExportCollection),available);}
