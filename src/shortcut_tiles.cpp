#include "app.hpp"
#include <algorithm>
namespace qc {
int App::selectedShortcut() const{int index=rows?ListView_GetNextItem(rows,-1,LVNI_SELECTED):-1;return shortcutVisible(index)?index:-1;}
void App::selectShortcut(int index){
 if(!shortcutVisible(index))return;
 ListView_SetItemState(rows,-1,0,LVIS_SELECTED|LVIS_FOCUSED);ListView_SetItemState(rows,index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);ListView_EnsureVisible(rows,index,FALSE);
 EnableWindow(GetDlgItem(popup,CmdExecute),!workflowsVisible);updateShortcutDetail();
}
void App::layoutShortcutTiles(){
 if(!rows||workflowsVisible)return;UINT dpi=GetDpiForWindow(popup);auto px=[&](int n){return MulDiv(n,dpi,96);};RECT r;GetClientRect(rows,&r);
 int usable=(std::max)(static_cast<int>(r.right)-px(32)-GetSystemMetricsForDpi(SM_CXVSCROLL,dpi),px(200));tileColumns=(std::max)(1,usable/px(300));int nextWidth=usable/tileColumns;
 if(nextWidth!=tileWidth||tileDpi!=dpi){bool previous=updatingPopup,redraw=IsWindowVisible(rows)&&!previous;updatingPopup=true;if(redraw)SendMessageW(rows,WM_SETREDRAW,FALSE,0);
 // Re-enter tile view after updating geometry to invalidate native item positions.
 ListView_SetView(rows,LV_VIEW_ICON);LVTILEVIEWINFO tiles{};tiles.cbSize=sizeof(tiles);tiles.dwMask=LVTVIM_TILESIZE;tiles.dwFlags=LVTVIF_FIXEDSIZE;tiles.sizeTile={nextWidth,px(92)};ListView_SetTileViewInfo(rows,&tiles);ListView_SetView(rows,LV_VIEW_TILE);tileWidth=nextWidth;tileDpi=dpi;
 updatingPopup=previous;if(redraw)SendMessageW(rows,WM_SETREDRAW,TRUE,0);}
 InvalidateRect(rows,nullptr,FALSE);
}
void App::drawRow(DRAWITEMSTRUCT* item){
 auto* profile=currentProfile();if(!profile||item->itemID>=visibleRows.size())return;const auto& shortcut=profile->shortcuts[visibleRows[item->itemID]];
 HDC dc=item->hDC;UINT dpi=GetDpiForWindow(popup);auto px=[&](int n){return MulDiv(n,dpi,96);};auto& c=theme.colors;RECT body=item->rcItem;InflateRect(&body,-px(5),-px(5));bool selected=(item->itemState&ODS_SELECTED)!=0;
 rounded(dc,body,selected?c.selection:c.panel,selected?c.accent:c.border,px(10));auto original=SelectObject(dc,font);RECT title{body.left+px(14),body.top+px(9),body.right-px(14),body.top+px(45)};paintText(dc,shortcut.title,title,c.text,DT_WORDBREAK|DT_NOPREFIX|DT_END_ELLIPSIS);
 SelectObject(dc,labelFont);auto chord=formatChord(shortcut.chord);size_t begin=0;int x=body.left+px(14),top=body.bottom-px(34);
 while(begin<chord.size()){auto end=chord.find(L'+',begin);auto token=chord.substr(begin,end==chord.npos?end:end-begin);SIZE size{};GetTextExtentPoint32W(dc,token.c_str(),static_cast<int>(token.size()),&size);RECT key{x,top,x+size.cx+px(16),top+px(26)};
 if(key.right>body.right-px(8)){RECT overflow{x,top,body.right-px(8),top+px(26)};paintText(dc,L"…",overflow,c.muted);break;}
 rounded(dc,key,c.key,c.border,px(6));paintText(dc,token,key,c.text,DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);x=key.right+px(5);if(end==chord.npos)break;begin=end+1;
 }
 SelectObject(dc,original);
}
void App::updateShortcutDetail(){
 if(!shortcutDetail)return;auto selected=selectedShortcut();auto* profile=currentProfile();bool show=false;std::wstring text;
 UINT dpi=GetDpiForWindow(popup);auto px=[&](int n){return MulDiv(n,dpi,96);};
 if(!workflowsVisible&&profile&&selected>=0&&selected<static_cast<int>(visibleRows.size())){const auto& shortcut=profile->shortcuts[visibleRows[selected]];HDC dc=GetDC(rows);auto original=SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));RECT title{0,0,(std::max)(tileWidth-px(38),px(80)),0};DrawTextW(dc,shortcut.title.c_str(),-1,&title,DT_CALCRECT|DT_WORDBREAK|DT_NOPREFIX);SelectObject(dc,labelFont?labelFont:GetStockObject(DEFAULT_GUI_FONT));auto chord=formatChord(shortcut.chord);SIZE keys{};GetTextExtentPoint32W(dc,chord.c_str(),static_cast<int>(chord.size()),&keys);SIZE category{};GetTextExtentPoint32W(dc,shortcut.category.c_str(),static_cast<int>(shortcut.category.size()),&category);show=title.bottom>px(36)||keys.cx+px(110)>tileWidth||category.cx>tileWidth-px(38);SelectObject(dc,original);ReleaseDC(rows,dc);text=shortcut.title+L"\r\n"+shortcut.category+L" · "+chord;}
 ShowWindow(shortcutDetail,show?SW_SHOWNA:SW_HIDE);EnableWindow(GetDlgItem(popup,CmdExecute),!workflowsVisible&&selected>=0);if(workflowsVisible)return;
 RECT r;GetClientRect(popup,&r);int width=MulDiv(r.right,96,dpi),height=MulDiv(r.bottom,96,dpi);int top=PopupContentTop+(profile&&!profile->workflows.empty()?PopupTabsHeight:0);bounds(rows,8,top,width-16,(std::max)(height-top-PopupFooterHeight-(show?120:0),60),dpi);
 if(show){if(textOf(shortcutDetail)!=text){SetWindowTextW(shortcutDetail,text.c_str());SendMessageW(shortcutDetail,EM_SETSEL,0,0);SendMessageW(shortcutDetail,EM_SCROLLCARET,0,0);}bounds(shortcutDetail,12,height-PopupFooterHeight-108,width-24,96,dpi);}
}
}
