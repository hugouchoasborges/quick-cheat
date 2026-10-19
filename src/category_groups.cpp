#include "app.hpp"
#include <algorithm>
#include <cwctype>
namespace qc {
namespace {
std::wstring preferenceKey(const std::wstring& value){
 constexpr wchar_t digits[]=L"0123456789abcdef";std::wstring key;
 for(auto character:value){auto c=static_cast<unsigned>(towlower(character));for(int shift=12;shift>=0;shift-=4)key+=digits[(c>>shift)&15];}return key;
}
std::filesystem::path preferencesPath(const std::filesystem::path& file){return std::filesystem::absolute(file).parent_path()/L"ui-state.ini";}
}
bool App::shortcutVisible(int index) const{
 if(index<0||index>=static_cast<int>(visibleRows.size()))return false;
 LVITEMW item{};item.mask=LVIF_GROUPID;item.iItem=index;if(!ListView_GetItem(rows,&item))return false;
 return !(ListView_GetGroupState(rows,item.iGroupId,LVGS_COLLAPSED)&LVGS_COLLAPSED);
}
void App::moveShortcutSelection(int direction){
 int selected=selectedShortcut(),count=static_cast<int>(visibleRows.size());int next=selected<0?(direction>0?0:count-1):selected+direction;
 for(;next>=0&&next<count;next+=direction)if(shortcutVisible(next)){selectShortcut(next);return;}
}
void App::populateShortcutTiles(){
 auto* profile=currentProfile();shortcutCategories.clear();categoryProfile=profile?profile->executable:L"";categorySearch=!textOf(search).empty();if(!profile)return;
 ListView_EnableGroupView(rows,TRUE);tileDpi=0;auto prefs=preferencesPath(file);auto section=preferenceKey(categoryProfile);
 for(size_t i=0;i<visibleRows.size();++i){const auto& shortcut=profile->shortcuts[visibleRows[i]];auto name=shortcut.category.empty()?L"General":shortcut.category;
 auto found=std::find_if(shortcutCategories.begin(),shortcutCategories.end(),[&](const auto& category){return _wcsicmp(category.name.c_str(),name.c_str())==0;});int groupId=static_cast<int>(found-shortcutCategories.begin());
 if(found==shortcutCategories.end()){
  bool collapsed=!categorySearch&&GetPrivateProfileIntW(section.c_str(),preferenceKey(name).c_str(),0,prefs.c_str())!=0;shortcutCategories.push_back({name,collapsed});
  LVGROUP group{sizeof(group)};group.mask=LVGF_HEADER|LVGF_GROUPID|LVGF_STATE|LVGF_ALIGN;group.pszHeader=name.data();group.iGroupId=groupId;group.uAlign=LVGA_HEADER_LEFT;group.stateMask=LVGS_COLLAPSIBLE|LVGS_COLLAPSED;group.state=LVGS_COLLAPSIBLE|(collapsed?LVGS_COLLAPSED:0);ListView_InsertGroup(rows,-1,&group);
 }
 auto label=shortcut.title+L" — "+shortcutActionText(shortcut)+L" ("+name+L")";LVITEMW item{};item.mask=LVIF_TEXT|LVIF_GROUPID;item.iItem=static_cast<int>(i);item.iGroupId=groupId;item.pszText=label.data();ListView_InsertItem(rows,&item);
 }
}
void App::syncCategoryStates(){
 if(!rows||updatingPopup||shortcutCategories.empty())return;
 auto prefs=preferencesPath(file);auto section=preferenceKey(categoryProfile);bool failed=false,changed=false;
 for(size_t i=0;i<shortcutCategories.size();++i){auto& category=shortcutCategories[i];bool collapsed=(ListView_GetGroupState(rows,static_cast<int>(i),LVGS_COLLAPSED)&LVGS_COLLAPSED)!=0;
  if(collapsed==category.collapsed)continue;category.collapsed=collapsed;pressedRow=-1;changed=true;
  if(!categorySearch&&!WritePrivateProfileStringW(section.c_str(),preferenceKey(category.name).c_str(),collapsed?L"1":L"0",prefs.c_str()))failed=true;
 }
 auto* profile=currentProfile();if(changed&&profile&&profile->executable==categoryProfile){int selected=ListView_GetNextItem(rows,-1,LVNI_SELECTED);if(selected>=0&&!shortcutVisible(selected))ListView_SetItemState(rows,selected,0,LVIS_SELECTED|LVIS_FOCUSED);updateShortcutDetail();}
 if(failed&&!categorySaveWarning){categorySaveWarning=true;error(L"Cannot save category expansion preferences. Check access to the data folder.\n\n"+prefs.wstring(),popup);}
}
bool App::clickCategory(POINT point){
 for(size_t i=0;i<shortcutCategories.size();++i){RECT rect{};if(!ListView_GetGroupRect(rows,static_cast<int>(i),LVGGR_HEADER,&rect)||!PtInRect(&rect,point))continue;
  pressedRow=-1;ListView_SetItemState(rows,-1,0,LVIS_SELECTED|LVIS_FOCUSED);
  LVGROUP group{sizeof(group)};group.mask=LVGF_STATE;group.stateMask=LVGS_COLLAPSED|LVGS_FOCUSED;group.state=LVGS_FOCUSED|((ListView_GetGroupState(rows,static_cast<int>(i),LVGS_COLLAPSED)&LVGS_COLLAPSED)?0:LVGS_COLLAPSED);
  ListView_SetGroupInfo(rows,static_cast<int>(i),&group);SetFocus(rows);syncCategoryStates();InvalidateRect(rows,nullptr,FALSE);return true;
 }return false;
}
void App::drawCategory(NMLVCUSTOMDRAW* draw){
 int id=static_cast<int>(draw->nmcd.dwItemSpec);if(id<0||id>=static_cast<int>(shortcutCategories.size()))return;auto dc=draw->nmcd.hdc;RECT rect{};if(!ListView_GetGroupRect(rows,id,LVGGR_HEADER,&rect))return;
 RECT client{},visible{};GetClientRect(rows,&client);if(!IntersectRect(&visible,&rect,&client))return;
 auto px=[&](int n){return MulDiv(n,GetDpiForWindow(rows),96);};auto state=ListView_GetGroupState(rows,id,LVGS_COLLAPSED|LVGS_FOCUSED);FillRect(dc,&rect,theme.background);
 int center=(rect.top+rect.bottom)/2,x=rect.left+px(12);HPEN pen=CreatePen(PS_SOLID,(std::max)(1,px(1)),theme.colors.muted);auto oldPen=SelectObject(dc,pen);
 if(state&LVGS_COLLAPSED){MoveToEx(dc,x,center-px(4),nullptr);LineTo(dc,x+px(4),center);LineTo(dc,x,center+px(4));}
 else{MoveToEx(dc,x-px(2),center-px(2),nullptr);LineTo(dc,x+px(2),center+px(2));LineTo(dc,x+px(6),center-px(2));}
 auto oldFont=SelectObject(dc,font);auto& label=shortcutCategories[id].name;RECT text=rect;text.left=x+px(16);text.right-=px(16);paintText(dc,label,text,theme.colors.text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
 SIZE size{};GetTextExtentPoint32W(dc,label.c_str(),static_cast<int>(label.size()),&size);int start=text.left+size.cx+px(14);if(start<rect.right-px(16)){MoveToEx(dc,start,center,nullptr);LineTo(dc,rect.right-px(12),center);}
 SelectObject(dc,oldFont);SelectObject(dc,oldPen);DeleteObject(pen);if((state&LVGS_FOCUSED)&&GetFocus()==rows){InflateRect(&rect,-px(2),-px(2));DrawFocusRect(dc,&rect);}
}
}
