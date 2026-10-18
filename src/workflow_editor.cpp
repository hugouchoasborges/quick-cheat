#include "app.hpp"
#include "workflows.hpp"
#include "attachments.hpp"
#include <commdlg.h>
#include <algorithm>
namespace qc {
namespace {
enum {FlowList=400,FlowTitle,StepList,StepText,NewFlow,SaveFlow,DeleteFlow,NewStep,ApplyStep,DeleteStep,StepUp,StepDown,Close,Status,PageTitle,TitleLabel,StepsLabel,TextLabel,AttachImage};
bool hasText(const std::wstring& text){return text.find_first_not_of(L" \t\r\n")!=text.npos;}
Profile* findProfile(Store& store,const std::wstring& key){for(auto& p:store.profiles)if(p.executable==key)return &p;return nullptr;}
void syncWorkflowDraft(App& app){if(app.editor)for(auto& profile:app.editor->draft.profiles)if(auto* saved=findProfile(app.store,profile.executable))profile.workflows=saved->workflows;}
std::wstring summary(const std::wstring& text){auto line=text.substr(0,96);for(auto& c:line)if(c==L'\n'||c==L'\r'||c==L'\t')c=L' ';return line+(text.size()>96?L"…":L"");}
}
WorkflowEditor::~WorkflowEditor(){if(window)DestroyWindow(window);if(font)DeleteObject(font);}
void App::openWorkflowEditor(const std::wstring& key){
 if(key.empty())return;
 if(!workflowEditor)workflowEditor=std::make_unique<WorkflowEditor>(*this);
 workflowEditor->show(key);
}
void WorkflowEditor::show(const std::wstring& key){
 if(window&&IsWindowVisible(window)){ShowWindow(window,SW_RESTORE);SetForegroundWindow(window);return;}
 auto* profile=findProfile(app.store,key);if(!profile){app.error(L"Save or choose an application profile first.",app.editor?app.editor->window:nullptr);return;}
 executable=key;
 if(!window){WNDCLASSW cls{};cls.hInstance=app.instance;cls.lpfnWndProc=proc;cls.lpszClassName=L"QuickCheatWorkflows";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(app.instance,MAKEINTRESOURCEW(1));RegisterClassW(&cls);
 window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"Quick Cheat — Workflows",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1060,760,app.editor?app.editor->window:nullptr,nullptr,app.instance,this);}
 if(!window)return;
 auto caption=L"Quick Cheat — Workflows — "+profile->name;SetWindowTextW(window,caption.c_str());
 refreshFlows();loadFlow(profile->workflows.empty()?-1:0);app.theme.style(window);layout();
 if(app.editor)EnableWindow(app.editor->window,FALSE);
 ShowWindow(window,SW_SHOW);SetForegroundWindow(window);SetFocus(title);
}
bool WorkflowEditor::discard(){return !(dirty||pendingStep)||MessageBoxW(window,L"Discard unsaved workflow changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)==IDYES;}
void WorkflowEditor::close(){
 if(!discard())return;
 dirty=pendingStep=false;ShowWindow(window,SW_HIDE);
 if(app.editor){syncWorkflowDraft(app);EnableWindow(app.editor->window,TRUE);SetForegroundWindow(app.editor->window);}
}
void WorkflowEditor::layout(){
 if(!window)return;UINT nextDpi=GetDpiForWindow(window);RECT rect;GetClientRect(window,&rect);int width=MulDiv(rect.right,96,nextDpi),height=MulDiv(rect.bottom,96,nextDpi);
 if(!font||dpi!=nextDpi){auto next=CreateFontW(-MulDiv(14,nextDpi,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");for(HWND c=GetWindow(window,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(next),TRUE);if(font)DeleteObject(font);font=next;dpi=nextDpi;}
 auto place=[&](int id,int x,int y,int w,int h){bounds(GetDlgItem(window,id),x,y,w,h,dpi);};int content=width-352;
 place(PageTitle,24,20,width-48,30);place(FlowList,24,64,276,height-192);
 place(NewFlow,24,height-112,132,32);place(DeleteFlow,168,height-112,132,32);
 place(TitleLabel,328,64,content,20);place(FlowTitle,328,90,content,30);
 place(StepsLabel,328,136,content,20);place(StepList,328,164,content,164);
 place(NewStep,328,338,100,30);place(DeleteStep,436,338,100,30);place(StepUp,544,338,100,30);place(StepDown,652,338,108,30);
 place(TextLabel,328,384,content,20);place(StepText,328,412,content,height-546);place(ApplyStep,328,height-124,132,32);
 place(AttachImage,472,height-124,148,32);
 place(SaveFlow,width-300,height-66,164,36);place(Close,width-124,height-66,100,36);place(Status,24,height-24,width-48,20);
 InvalidateRect(window,nullptr,TRUE);
}
void WorkflowEditor::refreshFlows(){
 loading=true;SendMessageW(flows,LB_RESETCONTENT,0,0);if(auto* profile=findProfile(app.store,executable))for(const auto& flow:profile->workflows)SendMessageW(flows,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(flow.title.c_str()));loading=false;
}
void WorkflowEditor::loadFlow(int index){
 loading=true;flowIndex=index;draft={};if(auto* profile=findProfile(app.store,executable);profile&&index>=0&&index<static_cast<int>(profile->workflows.size()))draft=profile->workflows[index];else flowIndex=-1;
 SendMessageW(flows,LB_SETCURSEL,flowIndex,0);SetWindowTextW(title,draft.title.c_str());stepIndex=-1;SetWindowTextW(stepText,L"");refreshSteps();loading=false;dirty=pendingStep=false;
 SetWindowTextW(status,L"Markdown: headings, **bold**, *italic*, lists, `code`, links and attached images. Save workflow to keep changes.");
}
void WorkflowEditor::refreshSteps(){
 SendMessageW(steps,LB_RESETCONTENT,0,0);for(size_t i=0;i<draft.steps.size();++i){auto text=std::to_wstring(i+1)+L". "+summary(draft.steps[i]);SendMessageW(steps,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(text.c_str()));}SendMessageW(steps,LB_SETCURSEL,stepIndex,0);
 EnableWindow(GetDlgItem(window,DeleteStep),stepIndex>=0);EnableWindow(GetDlgItem(window,StepUp),stepIndex>0);EnableWindow(GetDlgItem(window,StepDown),stepIndex>=0&&stepIndex+1<static_cast<int>(draft.steps.size()));
}
bool WorkflowEditor::applyStep(){
 if(!pendingStep)return true;auto text=textOf(stepText);if(!hasText(text)||text.size()>4096){app.error(L"Enter step text (1–4096 characters). To remove a step, use Remove step.",window);return false;}
 if(stepIndex>=0)draft.steps[stepIndex]=std::move(text);else{if(draft.steps.size()>=128){app.error(L"A workflow supports up to 128 steps.",window);return false;}draft.steps.push_back(std::move(text));stepIndex=static_cast<int>(draft.steps.size()-1);}
 dirty=true;pendingStep=false;refreshSteps();return true;
}
void WorkflowEditor::selectStep(int index){
 if(index<0||index>=static_cast<int>(draft.steps.size()))return;
 loading=true;stepIndex=index;SetWindowTextW(stepText,draft.steps[index].c_str());loading=false;pendingStep=false;refreshSteps();
}
void WorkflowEditor::moveStep(int delta){
 if(!applyStep())return;int next=stepIndex+delta;if(stepIndex<0||next<0||next>=static_cast<int>(draft.steps.size()))return;
 std::swap(draft.steps[stepIndex],draft.steps[next]);stepIndex=next;dirty=true;refreshSteps();
}
void WorkflowEditor::attachImage(){
 auto text=textOf(stepText);size_t count=0,at=0;while((at=text.find(L"](attachments/",at))!=text.npos){++count;at+=14;}if(count>=8){app.error(L"A step can display up to eight attached images.",window);return;}
 wchar_t source[32768]{};OPENFILENAMEW dialog{};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=window;dialog.lpstrFilter=L"Images (PNG, JPEG, BMP, GIF)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\0All files\0*.*\0";dialog.lpstrFile=source;dialog.nMaxFile=32768;dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetOpenFileNameW(&dialog))return;std::wstring reference,error;if(!importWorkflowImage(source,app.file.parent_path(),reference,error)){app.error(error,window);return;}
 auto markdown=L"\r\n\r\n![Step image]("+reference+L")\r\n";DWORD begin=0,end=0;SendMessageW(stepText,EM_GETSEL,reinterpret_cast<WPARAM>(&begin),reinterpret_cast<LPARAM>(&end));
 if(textOf(stepText).size()-(end-begin)+markdown.size()>4096){app.error(L"The step text plus attachment reference exceeds 4096 characters. Shorten it first.",window);return;}
 SendMessageW(stepText,EM_REPLACESEL,TRUE,reinterpret_cast<LPARAM>(markdown.c_str()));pendingStep=true;SetFocus(stepText);SetWindowTextW(status,L"Image attached locally. Choose Save workflow to keep the step and its image reference.");
}
void WorkflowEditor::save(){
 if(!applyStep())return;draft.title=textOf(title);if(!hasText(draft.title)||draft.title.size()>240||draft.steps.empty()){app.error(L"Enter a workflow title (up to 240 characters) and at least one step.",window);return;}
 auto candidate=app.store;auto* profile=findProfile(candidate,executable);if(!profile)return;
 if(flowIndex<0)profile->workflows.push_back(draft);else if(flowIndex<static_cast<int>(profile->workflows.size()))profile->workflows[flowIndex]=draft;else return;
 if(!validStore(candidate)){app.error(L"Workflow limits reached: up to 256 workflows per application, 128 steps each, 4096 characters per step, and 8 MiB total storage.",window);return;}
 if(!app.commit(std::move(candidate)))return;
 if(flowIndex<0)flowIndex=static_cast<int>(findProfile(app.store,executable)->workflows.size()-1);
 syncWorkflowDraft(app);refreshFlows();SendMessageW(flows,LB_SETCURSEL,flowIndex,0);dirty=pendingStep=false;SetWindowTextW(status,L"Workflow saved on this computer.");
}
void WorkflowEditor::remove(){
 if(flowIndex<0)return;if(MessageBoxW(window,L"Delete this workflow and all its steps, including unsaved changes?",L"Quick Cheat",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;
 auto candidate=app.store;auto* profile=findProfile(candidate,executable);if(!profile||flowIndex>=static_cast<int>(profile->workflows.size()))return;
 profile->workflows.erase(profile->workflows.begin()+flowIndex);if(!app.commit(std::move(candidate)))return;
 syncWorkflowDraft(app);refreshFlows();loadFlow(-1);
}
LRESULT CALLBACK WorkflowEditor::proc(HWND h,UINT msg,WPARAM w,LPARAM l){
 auto* e=reinterpret_cast<WorkflowEditor*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(msg==WM_NCCREATE){e=static_cast<WorkflowEditor*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);e->window=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(e));}if(!e)return DefWindowProcW(h,msg,w,l);
 switch(msg){
 case WM_CREATE:{
 control(h,L"STATIC",L"Workflows — step-by-step guides for your application",0,PageTitle);
 e->flows=control(h,L"LISTBOX",L"",LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP,FlowList);
 control(h,L"STATIC",L"Workflow title",0,TitleLabel);e->title=control(h,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,FlowTitle);SendMessageW(e->title,EM_SETLIMITTEXT,240,0);
 control(h,L"STATIC",L"Steps — select a step to read or edit it",0,StepsLabel);e->steps=control(h,L"LISTBOX",L"",LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP,StepList);
 control(h,L"STATIC",L"Step text (Markdown)",0,TextLabel);e->stepText=control(h,L"EDIT",L"",ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN|WS_VSCROLL|WS_TABSTOP,StepText);SendMessageW(e->stepText,EM_SETLIMITTEXT,4096,0);
 control(h,L"BUTTON",L"Attach image...",WS_TABSTOP,AttachImage);
 const std::pair<int,const wchar_t*> buttons[]={{NewFlow,L"New workflow"},{SaveFlow,L"Save workflow"},{DeleteFlow,L"Delete workflow"},{NewStep,L"New step"},{ApplyStep,L"Apply step"},{DeleteStep,L"Remove step"},{StepUp,L"Move up"},{StepDown,L"Move down"},{Close,L"Close"}};for(auto [id,label]:buttons)control(h,L"BUTTON",label,WS_TABSTOP,id);
 e->status=control(h,L"STATIC",L"",0,Status);return 0;}
 case WM_SIZE:e->layout();return 0;
 case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(h,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);e->layout();return 0;}
 case WM_GETMINMAXINFO:{UINT dpi=GetDpiForWindow(h);reinterpret_cast<MINMAXINFO*>(l)->ptMinTrackSize={MulDiv(1000,dpi,96),MulDiv(720,dpi,96)};return 0;}
 case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect(reinterpret_cast<HDC>(w),&r,e->app.theme.background);return 1;}
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:return e->app.theme.controlColor(msg,reinterpret_cast<HDC>(w),reinterpret_cast<HWND>(l));
 case WM_DRAWITEM:e->app.theme.drawControl(reinterpret_cast<DRAWITEMSTRUCT*>(l));return TRUE;
 case WM_COMMAND:
 if(e->loading)return 0;
 if(HIWORD(w)==EN_CHANGE){if(LOWORD(w)==FlowTitle)e->dirty=true;else if(LOWORD(w)==StepText)e->pendingStep=true;return 0;}
 switch(LOWORD(w)){
 case FlowList:if(HIWORD(w)==LBN_SELCHANGE){int index=static_cast<int>(SendMessageW(e->flows,LB_GETCURSEL,0,0));if(index!=e->flowIndex&&e->discard())e->loadFlow(index);else SendMessageW(e->flows,LB_SETCURSEL,e->flowIndex,0);}break;
 case StepList:if(HIWORD(w)==LBN_SELCHANGE){int index=static_cast<int>(SendMessageW(e->steps,LB_GETCURSEL,0,0));if(e->applyStep())e->selectStep(index);else SendMessageW(e->steps,LB_SETCURSEL,e->stepIndex,0);}break;
 case NewFlow:if(e->discard()){e->loadFlow(-1);SetFocus(e->title);}break;
 case SaveFlow:e->save();break;case DeleteFlow:e->remove();break;
 case NewStep:if(e->applyStep()){e->loading=true;e->stepIndex=-1;SetWindowTextW(e->stepText,L"");e->loading=false;e->refreshSteps();SetFocus(e->stepText);}break;
 case ApplyStep:e->applyStep();break;
 case AttachImage:e->attachImage();break;
 case DeleteStep:if(e->stepIndex>=0){e->draft.steps.erase(e->draft.steps.begin()+e->stepIndex);e->dirty=true;e->pendingStep=false;e->loading=true;e->stepIndex=-1;SetWindowTextW(e->stepText,L"");e->loading=false;e->refreshSteps();}break;
 case StepUp:e->moveStep(-1);break;case StepDown:e->moveStep(1);break;
 case Close:case IDCANCEL:e->close();break;
 }return 0;
 case WM_CLOSE:e->close();return 0;
 }return DefWindowProcW(h,msg,w,l);
}
}
