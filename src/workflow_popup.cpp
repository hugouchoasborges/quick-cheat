#include "app.hpp"
#include "markdown_view.hpp"
#include <algorithm>
#include <cwctype>
#include <uxtheme.h>
namespace qc {
namespace {
constexpr int Tree=7,Detail=8;
bool contains(const std::wstring& text,const std::wstring& query){return std::search(text.begin(),text.end(),query.begin(),query.end(),[](wchar_t a,wchar_t b){return std::towlower(a)==std::towlower(b);})!=text.end();}
std::wstring preview(const std::wstring& step){auto end=step.find_first_of(L"\r\n");auto line=step.substr(0,end);auto start=line.find_first_not_of(L" #*`_\t");if(start!=line.npos)line.erase(0,start);auto text=line.substr(0,40);if(line.size()>40)text+=L"…";return text;}
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
 styleWorkflowPopup();
}
void App::styleWorkflowPopup(){
 if(!workflowTree)return;SetWindowTheme(workflowTree,L"",L"");TreeView_SetBkColor(workflowTree,theme.colors.background);TreeView_SetTextColor(workflowTree,theme.colors.text);TreeView_SetLineColor(workflowTree,theme.colors.muted);if(workflowDetail)restyleMarkdown(workflowDetail,theme.colors);
}
void App::layoutWorkflowPopup(){
 auto* profile=currentProfile();bool hasFlows=profile&&!profile->workflows.empty();if(!hasFlows)workflowsVisible=false;
 if(workflowsVisible&&!workflowDetail){workflowDetail=createMarkdownView(popup,Detail);if(workflowDetail){SendMessageW(workflowDetail,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);SetWindowSubclass(workflowDetail,treeProc,1,reinterpret_cast<DWORD_PTR>(this));}}
 UINT dpi=GetDpiForWindow(popup);RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);
 ShowWindow(GetDlgItem(popup,CmdShortcuts),hasFlows?SW_SHOWNA:SW_HIDE);ShowWindow(GetDlgItem(popup,CmdWorkflows),hasFlows?SW_SHOWNA:SW_HIDE);
 bounds(GetDlgItem(popup,CmdShortcuts),16,116,112,28,dpi);bounds(GetDlgItem(popup,CmdWorkflows),136,116,112,28,dpi);
 bounds(rows,8,hasFlows?152:118,width-16,height-(hasFlows?220:186),dpi);
 int contentHeight=(std::max)(height-224,120);if(width>=960){int treeWidth=(width-48)*42/100;bounds(workflowTree,16,152,treeWidth,contentHeight,dpi);bounds(workflowDetail,32+treeWidth,152,width-48-treeWidth,contentHeight,dpi);}
 else{int treeHeight=contentHeight*55/100;bounds(workflowTree,16,152,width-32,treeHeight,dpi);bounds(workflowDetail,16,164+treeHeight,width-32,contentHeight-treeHeight-12,dpi);}
 TreeView_SetItemHeight(workflowTree,MulDiv(30,dpi,96));TreeView_SetIndent(workflowTree,MulDiv(20,dpi,96));
 ShowWindow(workflowTree,workflowsVisible?SW_SHOWNA:SW_HIDE);ShowWindow(workflowDetail,workflowsVisible?SW_SHOWNA:SW_HIDE);
 if(!workflowsVisible&&workflowDetail)clearMarkdown(workflowDetail);
 ShowWindow(GetDlgItem(popup,CmdExecute),workflowsVisible?SW_HIDE:SW_SHOWNA);
 SetPropW(GetDlgItem(popup,CmdShortcuts),L"QcSelected",reinterpret_cast<HANDLE>(!workflowsVisible));SetPropW(GetDlgItem(popup,CmdWorkflows),L"QcSelected",reinterpret_cast<HANDLE>(workflowsVisible));

 if(workflowsVisible){ShowWindow(rows,SW_HIDE);ShowWindow(empty,SW_HIDE);}
}
void App::refreshWorkflowPopup(){
 auto* profile=currentProfile();if(!profile||profile->workflows.empty())workflowsVisible=false;
 layoutWorkflowPopup();if(!workflowsVisible)return;
 refreshingWorkflows=true;SendMessageW(workflowTree,WM_SETREDRAW,FALSE,0);TreeView_DeleteAllItems(workflowTree);clearMarkdown(workflowDetail);auto query=textOf(search);size_t shown=0;HTREEITEM first=nullptr,browserGroup=nullptr,siteGroup=nullptr;
 for(size_t i=0;i<profile->workflows.size();++i){const auto& flow=profile->workflows[i];if(!contains(flow.title,query))continue;++shown;
 HTREEITEM parent=TVI_ROOT;
 if(isBrowserExecutable(target.executable)){
  auto& group=i<browserWorkflowCount?browserGroup:siteGroup;if(!group){auto text=i<browserWorkflowCount?L"Browser":L"Website: "+target.website;TVINSERTSTRUCTW section{};section.hParent=TVI_ROOT;section.hInsertAfter=TVI_LAST;section.item.mask=TVIF_TEXT|TVIF_PARAM;section.item.pszText=text.data();section.item.lParam=0;group=TreeView_InsertItem(workflowTree,&section);TreeView_Expand(workflowTree,group,TVE_EXPAND);}parent=group;
 }
 auto label=preview(flow.title);TVINSERTSTRUCTW entry{};entry.hParent=parent;entry.hInsertAfter=TVI_LAST;entry.item.mask=TVIF_TEXT|TVIF_PARAM;entry.item.pszText=label.data();entry.item.lParam=static_cast<LPARAM>((i+1)<<8);auto root=TreeView_InsertItem(workflowTree,&entry);if(!first)first=root;
 for(size_t j=0;j<flow.steps.size();++j){auto text=std::to_wstring(j+1)+L". "+preview(flow.steps[j]);entry.hParent=root;entry.item.pszText=text.data();entry.item.lParam=static_cast<LPARAM>(((i+1)<<8)|(j+1));TreeView_InsertItem(workflowTree,&entry);}
 }
 refreshingWorkflows=false;if(first){TreeView_Expand(workflowTree,first,TVE_EXPAND);TreeView_SelectItem(workflowTree,first);}else showMarkdown(workflowDetail,L"No matching workflows. Try another title.",file.parent_path(),theme.colors);
 SendMessageW(workflowTree,WM_SETREDRAW,TRUE,0);InvalidateRect(workflowTree,nullptr,TRUE);SetWindowTextW(footer,(std::to_wstring(shown)+L" workflows · Reference only").c_str());EnableWindow(GetDlgItem(popup,CmdExecute),FALSE);
}
void App::selectWorkflowStep(){
 if(refreshingWorkflows)return;auto* profile=currentProfile();if(!profile)return;
 TVITEMW item{};item.mask=TVIF_PARAM;item.hItem=TreeView_GetSelection(workflowTree);if(!item.hItem||!TreeView_GetItem(workflowTree,&item))return;
 if(item.lParam==0){showMarkdown(workflowDetail,L"Select a workflow to read its steps.",file.parent_path(),theme.colors);return;}size_t flow=(static_cast<size_t>(item.lParam)>>8)-1;size_t step=static_cast<size_t>(item.lParam)&255;if(flow>=profile->workflows.size())return;const auto& guide=profile->workflows[flow];
 std::wstring text;if(step&&step<=guide.steps.size()){text=L"**Step "+std::to_wstring(step)+L" of "+std::to_wstring(guide.steps.size())+L"**\n\n"+guide.steps[step-1];}
 else{std::vector<std::wstring> sections;sections.reserve(guide.steps.size());for(size_t i=0;i<guide.steps.size();++i){auto section=L"## Step "+std::to_wstring(i+1)+L" of "+std::to_wstring(guide.steps.size())+L"\n\n"+guide.steps[i];if(i==0)section=L"# "+guide.title+L"\n\n"+section;sections.push_back(std::move(section));}showMarkdownSections(workflowDetail,sections,file.parent_path(),theme.colors);return;}
 showMarkdown(workflowDetail,text,file.parent_path(),theme.colors);
}
}
