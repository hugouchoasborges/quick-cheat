#include "app.hpp"
#include <algorithm>
#include <cwctype>
#include <uxtheme.h>
namespace qc {
namespace {
constexpr int Tree=7,Detail=8;
bool contains(const std::wstring& text,const std::wstring& query){return std::search(text.begin(),text.end(),query.begin(),query.end(),[](wchar_t a,wchar_t b){return std::towlower(a)==std::towlower(b);})!=text.end();}
std::wstring preview(const std::wstring& step){auto text=step.substr(0,40);for(auto& c:text)if(c==L'\r'||c==L'\n'||c==L'\t')c=L' ';if(step.size()>40)text+=L"…";return text;}
LRESULT CALLBACK treeProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR value){
 auto* app=reinterpret_cast<App*>(value);
 if(msg==WM_GETDLGCODE&&l){auto* key=reinterpret_cast<MSG*>(l);if(key->wParam==VK_RETURN||key->wParam==VK_ESCAPE)return DefSubclassProc(h,msg,w,l)|DLGC_WANTALLKEYS;}
 if(msg==WM_KEYDOWN&&w==VK_ESCAPE){app->hidePopup();return 0;}
 if(msg==WM_KEYDOWN&&w==VK_RETURN){auto item=TreeView_GetSelection(h);if(item)TreeView_Expand(h,item,TVE_TOGGLE);return 0;}
 return DefSubclassProc(h,msg,w,l);
}
}
void App::createWorkflowPopup(){
 control(popup,L"BUTTON",L"Shortcuts",WS_TABSTOP,CmdShortcuts);control(popup,L"BUTTON",L"Workflows",WS_TABSTOP,CmdWorkflows);
 workflowTree=control(popup,WC_TREEVIEWW,L"",TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_NOHSCROLL|WS_TABSTOP,Tree);
 SetWindowSubclass(workflowTree,treeProc,1,reinterpret_cast<DWORD_PTR>(this));
 workflowDetail=control(popup,L"EDIT",L"",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP,Detail);
 SetWindowSubclass(workflowDetail,treeProc,1,reinterpret_cast<DWORD_PTR>(this));
 SendMessageW(workflowDetail,EM_SETLIMITTEXT,8192,0);styleWorkflowPopup();
}
void App::styleWorkflowPopup(){
 if(!workflowTree)return;SetWindowTheme(workflowTree,L"",L"");TreeView_SetBkColor(workflowTree,theme.colors.background);TreeView_SetTextColor(workflowTree,theme.colors.text);TreeView_SetLineColor(workflowTree,theme.colors.muted);
}
void App::layoutWorkflowPopup(){
 auto* profile=currentProfile();bool hasFlows=profile&&!profile->workflows.empty();if(!hasFlows)workflowsVisible=false;
 UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 ShowWindow(GetDlgItem(popup,CmdShortcuts),hasFlows?SW_SHOWNA:SW_HIDE);ShowWindow(GetDlgItem(popup,CmdWorkflows),hasFlows?SW_SHOWNA:SW_HIDE);
 bounds(GetDlgItem(popup,CmdShortcuts),16,76,112,28,dpi);bounds(GetDlgItem(popup,CmdWorkflows),136,76,112,28,dpi);
 bounds(rows,8,hasFlows?112:78,width-16,height-(hasFlows?180:146),dpi);
 int contentHeight=(std::max)(height-184,120);if(width>=960){int treeWidth=(width-48)*42/100;bounds(workflowTree,16,112,treeWidth,contentHeight,dpi);bounds(workflowDetail,32+treeWidth,112,width-48-treeWidth,contentHeight,dpi);}
 else{int treeHeight=contentHeight*55/100;bounds(workflowTree,16,112,width-32,treeHeight,dpi);bounds(workflowDetail,16,124+treeHeight,width-32,contentHeight-treeHeight-12,dpi);}
 TreeView_SetItemHeight(workflowTree,MulDiv(30,dpi,96));TreeView_SetIndent(workflowTree,MulDiv(20,dpi,96));
 ShowWindow(workflowTree,workflowsVisible?SW_SHOWNA:SW_HIDE);ShowWindow(workflowDetail,workflowsVisible?SW_SHOWNA:SW_HIDE);
 ShowWindow(GetDlgItem(popup,CmdExecute),workflowsVisible?SW_HIDE:SW_SHOWNA);
 SetPropW(GetDlgItem(popup,CmdShortcuts),L"QcSelected",reinterpret_cast<HANDLE>(!workflowsVisible));SetPropW(GetDlgItem(popup,CmdWorkflows),L"QcSelected",reinterpret_cast<HANDLE>(workflowsVisible));
 SetWindowTextW(GetDlgItem(popup,CmdManage),workflowsVisible?L"Manage workflows":L"Manage shortcuts");
 if(workflowsVisible){ShowWindow(rows,SW_HIDE);ShowWindow(empty,SW_HIDE);}
}
void App::refreshWorkflowPopup(){
 auto* profile=currentProfile();if(!profile||profile->workflows.empty())workflowsVisible=false;
 layoutWorkflowPopup();if(!workflowsVisible)return;
 refreshingWorkflows=true;SendMessageW(workflowTree,WM_SETREDRAW,FALSE,0);TreeView_DeleteAllItems(workflowTree);SetWindowTextW(workflowDetail,L"");auto query=textOf(search);size_t shown=0;HTREEITEM first=nullptr;
 for(size_t i=0;i<profile->workflows.size();++i){const auto& flow=profile->workflows[i];if(!contains(flow.title,query))continue;++shown;
 auto label=preview(flow.title);TVINSERTSTRUCTW entry{};entry.hParent=TVI_ROOT;entry.hInsertAfter=TVI_LAST;entry.item.mask=TVIF_TEXT|TVIF_PARAM;entry.item.pszText=label.data();entry.item.lParam=static_cast<LPARAM>((i+1)<<8);auto root=TreeView_InsertItem(workflowTree,&entry);if(!first)first=root;
 for(size_t j=0;j<flow.steps.size();++j){auto text=std::to_wstring(j+1)+L". "+preview(flow.steps[j]);entry.hParent=root;entry.item.pszText=text.data();entry.item.lParam=static_cast<LPARAM>(((i+1)<<8)|(j+1));TreeView_InsertItem(workflowTree,&entry);}
 }
 refreshingWorkflows=false;if(first){TreeView_Expand(workflowTree,first,TVE_EXPAND);TreeView_SelectItem(workflowTree,first);}else SetWindowTextW(workflowDetail,L"No matching workflows. Try another title.");
 SendMessageW(workflowTree,WM_SETREDRAW,TRUE,0);InvalidateRect(workflowTree,nullptr,TRUE);SetWindowTextW(footer,(std::to_wstring(shown)+L" workflows · Reference only").c_str());EnableWindow(GetDlgItem(popup,CmdExecute),FALSE);
}
void App::selectWorkflowStep(){
 if(refreshingWorkflows)return;auto* profile=currentProfile();if(!profile)return;
 TVITEMW item{};item.mask=TVIF_PARAM;item.hItem=TreeView_GetSelection(workflowTree);if(!item.hItem||!TreeView_GetItem(workflowTree,&item))return;
 size_t flow=(static_cast<size_t>(item.lParam)>>8)-1;size_t step=static_cast<size_t>(item.lParam)&255;if(flow>=profile->workflows.size())return;const auto& guide=profile->workflows[flow];
 std::wstring text;if(step&&step<=guide.steps.size()){text=L"Step "+std::to_wstring(step)+L" of "+std::to_wstring(guide.steps.size())+L"\r\n\r\n"+guide.steps[step-1];}
 else text=guide.title+L"\r\n\r\n"+std::to_wstring(guide.steps.size())+L" steps. Expand this workflow and select a step to read it. This guide does not execute commands.";
 SetWindowTextW(workflowDetail,text.c_str());SendMessageW(workflowDetail,EM_SETSEL,0,0);SendMessageW(workflowDetail,EM_SCROLLCARET,0,0);
}
}
