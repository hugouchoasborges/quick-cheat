#include "markdown_view.hpp"
#include "attachments.hpp"
#include <md4c.h>
#include <commctrl.h>
#include <shellapi.h>
#include <algorithm>
#include <vector>
namespace qc {
namespace {
constexpr wchar_t property[]=L"QcMarkdown";
struct Link {LONG begin{},end{};std::wstring url;};
struct Reader {std::vector<std::wstring> sections;std::filesystem::path directory;Palette colors{};std::vector<Link> links;int width{-1};UINT dpi{};bool rendering{};};
Reader* readerState(HWND h){return static_cast<Reader*>(GetPropW(h,property));}
bool sameColors(const Palette& a,const Palette& b){return a.background==b.background&&a.field==b.field&&a.panel==b.panel&&a.text==b.text&&a.muted==b.muted&&a.accent==b.accent;}
bool safeLink(const std::wstring& url){return url.size()<=2048&&url.find_first_of(L"\r\n\t") ==url.npos&&(url.starts_with(L"https://")||url.starts_with(L"http://")||url.starts_with(L"mailto:"));}
std::wstring entity(std::wstring text){
 if(text==L"&amp;")return L"&";if(text==L"&lt;")return L"<";if(text==L"&gt;")return L">";if(text==L"&quot;")return L"\"";if(text==L"&apos;")return L"'";if(text==L"&nbsp;")return L"\u00a0";
 if(text.size()>3&&text[0]==L'&'&&text[1]==L'#'&&text.back()==L';'){try{auto digits=text.substr(2,text.size()-3);size_t used=0;int base=10;if(digits.starts_with(L"x")||digits.starts_with(L"X")){digits.erase(0,1);base=16;}auto value=std::stoul(digits,&used,base);if(used==digits.size()&&value&&value<=0x10ffff&&!(value>=0xd800&&value<=0xdfff)){if(value<0x10000)return std::wstring(1,static_cast<wchar_t>(value));value-=0x10000;return {static_cast<wchar_t>(0xd800+(value>>10)),static_cast<wchar_t>(0xdc00+(value&1023))};}}catch(...){} }
 return text;
}
std::wstring attribute(const MD_ATTRIBUTE& value){std::wstring result(value.text,value.size);size_t at=0;while((at=result.find(L'&',at))!=result.npos){auto end=result.find(L';',at);if(end==result.npos||end-at>32){++at;continue;}auto original=result.substr(at,end-at+1),decoded=entity(original);result.replace(at,original.size(),decoded);at+=decoded.size();}return result;}
struct Style {bool bold{},italic{},code{},strike{},link{},quote{};LONG size{220};};
struct List {bool ordered{};unsigned next{1};};
struct Renderer {
 HWND window;Reader& state;Style style;std::vector<Style> blocks,spans;std::vector<List> lists;std::vector<Link> openLinks;bool lineStart{true};unsigned imageDepth{},images{};std::wstring imageSource,imageAlt;
 LONG position(){CHARRANGE range{};SendMessageW(window,EM_EXGETSEL,0,reinterpret_cast<LPARAM>(&range));return range.cpMax;}
 void format(){CHARFORMAT2W f{};f.cbSize=sizeof(f);f.dwMask=CFM_BOLD|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE|CFM_LINK|CFM_FACE|CFM_SIZE|CFM_COLOR|CFM_BACKCOLOR;f.yHeight=style.size;f.crTextColor=style.link?state.colors.accent:style.quote?state.colors.muted:state.colors.text;f.crBackColor=style.code?state.colors.panel:state.colors.field;f.dwEffects=(style.bold?CFE_BOLD:0)|(style.italic?CFE_ITALIC:0)|(style.strike?CFE_STRIKEOUT:0)|(style.link?CFE_LINK|CFE_UNDERLINE:0);wcscpy_s(f.szFaceName,style.code?L"Consolas":L"Segoe UI");SendMessageW(window,EM_SETCHARFORMAT,SCF_SELECTION,reinterpret_cast<LPARAM>(&f));}
 void paragraph(){PARAFORMAT2 p{};p.cbSize=sizeof(p);p.dwMask=PFM_STARTINDENT|PFM_OFFSET|PFM_SPACEAFTER;p.dxStartIndent=static_cast<LONG>(lists.size())*240+(style.quote?200:0);p.dySpaceAfter=100;SendMessageW(window,EM_SETPARAFORMAT,0,reinterpret_cast<LPARAM>(&p));}
 void append(const std::wstring& text){if(text.empty())return;format();SendMessageW(window,EM_REPLACESEL,FALSE,reinterpret_cast<LPARAM>(text.c_str()));lineStart=text.back()==L'\r'||text.back()==L'\n';}
 void newline(){if(!lineStart)append(L"\r");}
 void rule(){
  newline();auto previous=style;style=Style{};style.quote=true;style.size=180;paragraph();append(L"────────────────────────────────");newline();style=previous;paragraph();
 }
 void image(){
 newline();std::wstring error;PreparedImage prepared;RECT area;GetClientRect(window,&area);UINT dpi=GetDpiForWindow(window);auto width=static_cast<UINT>((std::max)(static_cast<int>(area.right)-MulDiv(32,dpi,96),80));
 bool limited=images++>=8;bool ok=!limited&&loadWorkflowImage(state.directory,imageSource,width,prepared,error);if(ok){format();RICHEDIT_IMAGE_PARAMETERS p{};p.xWidth=MulDiv(prepared.width,2540,dpi);p.yHeight=MulDiv(prepared.height,2540,dpi);p.Type=TA_BASELINE;p.pwszAlternateText=imageAlt.c_str();p.pIStream=prepared.stream.Get();LONG before=position();auto status=SendMessageW(window,EM_INSERTIMAGE,0,reinterpret_cast<LPARAM>(&p));ok=SUCCEEDED(static_cast<HRESULT>(status));if(ok){CHARRANGE after{before+1,before+1};SendMessageW(window,EM_EXSETSEL,0,reinterpret_cast<LPARAM>(&after));}lineStart=false;}
 if(!ok)append(limited&&state.sections.size()>1?L"[Image preview limit reached. Select this step to view its images.]":L"[Image unavailable: "+(imageAlt.empty()?L"attachment":imageAlt)+L"]");newline();
 if(ok&&!imageAlt.empty()){auto previous=style;style.quote=true;style.size=180;append(imageAlt);newline();style=previous;}
 }
 static int enterBlock(MD_BLOCKTYPE type,void* detail,void* context){auto& r=*static_cast<Renderer*>(context);r.blocks.push_back(r.style);
 switch(type){case MD_BLOCK_H:r.newline();r.style.bold=true;r.style.size=(std::max)(240L,400L-static_cast<LONG>(static_cast<MD_BLOCK_H_DETAIL*>(detail)->level)*30);break;
 case MD_BLOCK_CODE:r.newline();r.style.code=true;r.style.size=200;break;case MD_BLOCK_QUOTE:r.newline();r.style.quote=true;break;
 case MD_BLOCK_UL:r.newline();r.lists.push_back({false,1});break;case MD_BLOCK_OL:r.newline();r.lists.push_back({true,static_cast<MD_BLOCK_OL_DETAIL*>(detail)->start});break;
 case MD_BLOCK_LI:{r.newline();r.paragraph();auto* item=static_cast<MD_BLOCK_LI_DETAIL*>(detail);if(item->is_task)r.append(item->task_mark==L' '?L"☐ ":L"☑ ");else if(!r.lists.empty()&&r.lists.back().ordered)r.append(std::to_wstring(r.lists.back().next++)+L". ");else r.append(L"• ");break;}
 case MD_BLOCK_HR:r.rule();break;default:break;}r.paragraph();return 0;}
 static int leaveBlock(MD_BLOCKTYPE type,void*,void* context){auto& r=*static_cast<Renderer*>(context);if(type==MD_BLOCK_P||type==MD_BLOCK_H||type==MD_BLOCK_CODE||type==MD_BLOCK_LI||type==MD_BLOCK_QUOTE)r.newline();if((type==MD_BLOCK_UL||type==MD_BLOCK_OL)&&!r.lists.empty())r.lists.pop_back();if(!r.blocks.empty()){r.style=r.blocks.back();r.blocks.pop_back();}return 0;}
 static int enterSpan(MD_SPANTYPE type,void* detail,void* context){auto& r=*static_cast<Renderer*>(context);r.spans.push_back(r.style);
 switch(type){case MD_SPAN_STRONG:r.style.bold=true;break;case MD_SPAN_EM:r.style.italic=true;break;case MD_SPAN_CODE:r.style.code=true;r.style.size=200;break;case MD_SPAN_DEL:r.style.strike=true;break;
 case MD_SPAN_A:{auto* link=static_cast<MD_SPAN_A_DETAIL*>(detail);r.openLinks.push_back({r.position(),0,attribute(link->href)});r.style.link=safeLink(r.openLinks.back().url);break;}
 case MD_SPAN_IMG:{auto* image=static_cast<MD_SPAN_IMG_DETAIL*>(detail);if(!r.imageDepth){r.imageSource=attribute(image->src);r.imageAlt.clear();}++r.imageDepth;break;}default:break;}return 0;}
 static int leaveSpan(MD_SPANTYPE type,void*,void* context){auto& r=*static_cast<Renderer*>(context);if(type==MD_SPAN_IMG&&r.imageDepth&&!--r.imageDepth)r.image();if(type==MD_SPAN_A&&!r.openLinks.empty()){auto link=std::move(r.openLinks.back());r.openLinks.pop_back();link.end=r.position();if(safeLink(link.url)){CHARRANGE range{link.begin,link.end};SendMessageW(r.window,EM_EXSETSEL,0,reinterpret_cast<LPARAM>(&range));CHARFORMAT2W f{};f.cbSize=sizeof(f);f.dwMask=CFM_LINK|CFM_UNDERLINE|CFM_COLOR;f.dwEffects=CFE_LINK|CFE_UNDERLINE;f.crTextColor=r.state.colors.accent;SendMessageW(r.window,EM_SETCHARFORMAT,SCF_SELECTION,reinterpret_cast<LPARAM>(&f));range={link.end,link.end};SendMessageW(r.window,EM_EXSETSEL,0,reinterpret_cast<LPARAM>(&range));r.state.links.push_back(std::move(link));}}if(!r.spans.empty()){r.style=r.spans.back();r.spans.pop_back();}return 0;}
 static int text(MD_TEXTTYPE type,const MD_CHAR* value,MD_SIZE length,void* context){auto& r=*static_cast<Renderer*>(context);std::wstring text(value,length);if(type==MD_TEXT_ENTITY)text=entity(std::move(text));if(type==MD_TEXT_NULLCHAR)text=L"\ufffd";if(r.imageDepth){r.imageAlt+=text;return 0;}if(type==MD_TEXT_BR)r.append(L"\r");else if(type==MD_TEXT_SOFTBR)r.append(L" ");else r.append(text);return 0;}
};
void render(HWND window,Reader& state){
 if(state.rendering)return;state.rendering=true;RECT rect;GetClientRect(window,&rect);state.width=rect.right;state.dpi=GetDpiForWindow(window);state.links.clear();
 SendMessageW(window,WM_SETREDRAW,FALSE,0);SendMessageW(window,EM_SETREADONLY,FALSE,0);SetWindowTextW(window,L"");SendMessageW(window,EM_SETBKGNDCOLOR,0,state.colors.field);
 MD_PARSER parser{};parser.flags=MD_FLAG_NOHTML|MD_FLAG_STRIKETHROUGH|MD_FLAG_TASKLISTS;parser.enter_block=Renderer::enterBlock;parser.leave_block=Renderer::leaveBlock;parser.enter_span=Renderer::enterSpan;parser.leave_span=Renderer::leaveSpan;parser.text=Renderer::text;
 unsigned images=0;for(size_t i=0;i<state.sections.size();++i){Renderer output{window,state};output.images=images;output.paragraph();if(i)output.rule();const auto& source=state.sections[i];if(md_parse(source.c_str(),static_cast<MD_SIZE>(source.size()),&parser,&output)!=0){std::wstring plain;for(const auto& part:state.sections)plain+=part+L"\r\n\r\n";SetWindowTextW(window,plain.c_str());state.links.clear();break;}images=output.images;output.newline();}
 SendMessageW(window,EM_SETREADONLY,TRUE,0);SendMessageW(window,EM_EMPTYUNDOBUFFER,0,0);SendMessageW(window,EM_SETSEL,0,0);SendMessageW(window,EM_SCROLLCARET,0,0);SendMessageW(window,WM_SETREDRAW,TRUE,0);RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME);state.rendering=false;
}
LRESULT CALLBACK readerProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR){auto* state=readerState(h);
 if(msg==WM_SIZE&&state&&!state->rendering&&!state->sections.empty()){RECT r;GetClientRect(h,&r);if(r.right!=state->width)SetTimer(h,1,140,nullptr);}
 if(msg==WM_TIMER&&w==1){KillTimer(h,1);if(state&&!state->sections.empty())render(h,*state);return 0;}
 if(msg==WM_NCDESTROY){KillTimer(h,1);RemoveWindowSubclass(h,readerProc,104);RemovePropW(h,property);delete state;}
 return DefSubclassProc(h,msg,w,l);
}
}
HWND createMarkdownView(HWND parent,int id){
 static HMODULE module=LoadLibraryExW(L"Msftedit.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!module)return nullptr;
 HWND h=CreateWindowExW(0,MSFTEDIT_CLASS,L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|ES_MULTILINE|ES_READONLY,0,0,0,0,parent,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);if(!h)return nullptr;
 auto* state=new Reader;SetPropW(h,property,state);SetWindowSubclass(h,readerProc,104,0);SendMessageW(h,EM_SETUNDOLIMIT,0,0);SendMessageW(h,EM_EXLIMITTEXT,0,1048576);SendMessageW(h,EM_SETEVENTMASK,0,ENM_LINK);SendMessageW(h,EM_AUTOURLDETECT,FALSE,0);SendMessageW(h,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,MAKELPARAM(10,10));return h;
}
void showMarkdown(HWND reader,const std::wstring& markdown,const std::filesystem::path& directory,const Palette& colors){showMarkdownSections(reader,{markdown},directory,colors);}
void showMarkdownSections(HWND reader,const std::vector<std::wstring>& sections,const std::filesystem::path& directory,const Palette& colors){
 auto* state=readerState(reader);if(!state)return;RECT rect;GetClientRect(reader,&rect);UINT dpi=GetDpiForWindow(reader);if(state->sections==sections&&state->directory==directory&&sameColors(state->colors,colors)&&state->width==rect.right&&state->dpi==dpi)return;
 state->sections=sections;state->directory=directory;state->colors=colors;render(reader,*state);
}
void restyleMarkdown(HWND reader,const Palette& colors){auto* state=readerState(reader);if(state&&!sameColors(state->colors,colors)){state->colors=colors;if(!state->sections.empty())render(reader,*state);else SendMessageW(reader,EM_SETBKGNDCOLOR,0,colors.field);}}
void clearMarkdown(HWND reader){auto* state=readerState(reader);if(!state||state->rendering)return;KillTimer(reader,1);state->sections.clear();state->links.clear();state->width=-1;SetWindowTextW(reader,L"");SendMessageW(reader,EM_EMPTYUNDOBUFFER,0,0);}
bool handleMarkdownLink(ENLINK* event){
 if(event->msg!=WM_LBUTTONUP)return false;auto* state=readerState(event->nmhdr.hwndFrom);if(!state)return false;POINTL point{static_cast<short>(LOWORD(event->lParam)),static_cast<short>(HIWORD(event->lParam))};LONG position=static_cast<LONG>(SendMessageW(event->nmhdr.hwndFrom,EM_CHARFROMPOS,0,reinterpret_cast<LPARAM>(&point)));std::wstring url;for(const auto& link:state->links)if(link.begin<=position&&position<link.end){url=link.url;break;}if(!safeLink(url))return false;ShellExecuteW(event->nmhdr.hwndFrom,L"open",url.c_str(),nullptr,nullptr,SW_SHOWNORMAL);return true;
}
}
