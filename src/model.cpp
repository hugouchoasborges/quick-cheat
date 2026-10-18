#include "model.hpp"
#include <algorithm>
#include <cwctype>
#include <sstream>
#include <set>
#include <winnls.h>
namespace qc {
namespace {
struct NamedKey { const wchar_t* name; WORD key; };
constexpr NamedKey names[] = {{L"Space",VK_SPACE},{L"Enter",VK_RETURN},{L"Tab",VK_TAB},
 {L"Escape",VK_ESCAPE},{L"Backspace",VK_BACK},{L"Delete",VK_DELETE},{L"Insert",VK_INSERT},
 {L"Home",VK_HOME},{L"End",VK_END},{L"PageUp",VK_PRIOR},{L"PageDown",VK_NEXT},
 {L"Left",VK_LEFT},{L"Right",VK_RIGHT},{L"Up",VK_UP},{L"Down",VK_DOWN},
 {L"Plus",VK_OEM_PLUS},{L"Minus",VK_OEM_MINUS},{L"Comma",VK_OEM_COMMA},
 {L"Period",VK_OEM_PERIOD},{L"Slash",VK_OEM_2},{L"Backslash",VK_OEM_5},
 {L"Semicolon",VK_OEM_1},{L"Quote",VK_OEM_7},{L"BracketLeft",VK_OEM_4},
 {L"BracketRight",VK_OEM_6},{L"Backtick",VK_OEM_3}};
std::wstring trim(std::wstring s) {
 const auto first=s.find_first_not_of(L" \r\n\t");
 return first==s.npos ? L"" : s.substr(first,s.find_last_not_of(L" \r\n\t")-first+1);
}
bool same(const std::wstring& a,const wchar_t* b) { return _wcsicmp(a.c_str(),b)==0; }
std::vector<std::wstring> split(const std::wstring& s,wchar_t delimiter) {
 std::vector<std::wstring> out; size_t begin=0;
 for(size_t pos=0;;begin=pos+1) { pos=s.find(delimiter,begin);out.push_back(s.substr(begin,pos==s.npos?pos:pos-begin));if(pos==s.npos)break; }
 return out;
}
std::wstring escape(const std::wstring& s) {
 std::wstring r; for(auto c:s) { switch(c) { case L'\\':r+=L"\\\\";break;case L'\t':r+=L"\\t";break;case L'\n':r+=L"\\n";break;case L'\r':r+=L"\\r";break;default:r+=c; } } return r;
}
bool unescape(const std::wstring& s,std::wstring& out) {
 out.clear();for(size_t i=0;i<s.size();++i) { wchar_t c=s[i];if(c==L'\\') {
 if(++i==s.size())return false;switch(s[i]) {case L'\\':c=L'\\';break;case L't':c=L'\t';break;case L'n':c=L'\n';break;case L'r':c=L'\r';break;default:return false;}}
 if(c==0)return false;out+=c;}return true;
}
bool number(const std::wstring& s,UINT& n) { if(s.empty()||s.size()>5)return false;n=0;for(auto c:s) { if(c<L'0'||c>L'9')return false;n=n*10+c-L'0'; }return n<=65535; }
bool validChord(Chord c) { return !(c.modifiers & ~15u) && !formatChord(c).empty(); }
}
std::optional<Chord> parseChord(const std::wstring& text) {
 Chord chord;bool hasKey=false;
 for(auto token:split(text,L'+')) {
 token=trim(token);if(token.empty())return {};
 UINT modifier=0;
 if(same(token,L"Ctrl"))modifier=MOD_CONTROL;else if(same(token,L"Alt"))modifier=MOD_ALT;
 else if(same(token,L"Shift"))modifier=MOD_SHIFT;else if(same(token,L"Win"))modifier=MOD_WIN;
 if(modifier) { if(hasKey||(chord.modifiers&modifier))return {};chord.modifiers|=modifier;continue; }
 if(hasKey)return {};hasKey=true;
 if(token.size()==1&&iswalnum(token[0])&&token[0]<128)chord.key=static_cast<WORD>(towupper(token[0]));
 else if((token[0]==L'F'||token[0]==L'f')&&token.size()<=3) { UINT n=0;if(!number(token.substr(1),n)||n<1||n>24)return {};chord.key=static_cast<WORD>(VK_F1+n-1); }
 else { for(const auto& entry:names)if(same(token,entry.name))chord.key=entry.key;if(!chord.key)return {}; }
 }
 return hasKey ? std::optional<Chord>(chord) : std::nullopt;
}
std::wstring formatChord(Chord c) {
 std::wstring key;
 if((c.key>='A'&&c.key<='Z')||(c.key>='0'&&c.key<='9'))key=static_cast<wchar_t>(c.key);
 else if(c.key>=VK_F1&&c.key<=VK_F24)key=L"F"+std::to_wstring(c.key-VK_F1+1);
 else for(const auto& entry:names)if(c.key==entry.key)key=entry.name;
 if(key.empty()||(c.modifiers&~15u))return L"";
 std::wstring result;if(c.modifiers&MOD_CONTROL)result+=L"Ctrl+";if(c.modifiers&MOD_ALT)result+=L"Alt+";
 if(c.modifiers&MOD_SHIFT)result+=L"Shift+";if(c.modifiers&MOD_WIN)result+=L"Win+";return result+key;
}
std::wstring appKey(const std::wstring& path) {
 auto result=path.substr(path.find_last_of(L"\\/")==path.npos?0:path.find_last_of(L"\\/")+1);
 std::transform(result.begin(),result.end(),result.begin(),[](wchar_t c){return static_cast<wchar_t>(towlower(c));});return result;
}
bool isBrowserExecutable(const std::wstring& executable) { auto key=appKey(executable);return key==L"chrome.exe"||key==L"msedge.exe"||key==L"firefox.exe"; }
std::wstring normalizeWebsite(const std::wstring& input) {
 auto host=trim(input);if(host.empty()||host.size()>2048||host.find(L'\0')!=host.npos)return {};
 auto scheme=host.find(L"://");if(scheme!=host.npos){auto prefix=host.substr(0,scheme);if(_wcsicmp(prefix.c_str(),L"https")&&_wcsicmp(prefix.c_str(),L"http"))return {};host.erase(0,scheme+3);}
 else if(host.find_first_of(L"/?#")!=host.npos)return {};
 host=host.substr(0,host.find_first_of(L"/?#"));if(host.find(L'@')!=host.npos)return {};
 auto colon=host.find(L':');if(colon!=host.npos){auto port=host.substr(colon+1);UINT value{};if(!number(port,value)||value==0)return {};host.resize(colon);}
 if(!host.empty()&&host.back()==L'.')host.pop_back();if(host.empty()||host.size()>253)return {};
 int needed=IdnToAscii(IDN_USE_STD3_ASCII_RULES,host.data(),static_cast<int>(host.size()),nullptr,0);if(needed<=0||needed>253)return {};
 std::wstring ascii(needed,L'\0');if(!IdnToAscii(IDN_USE_STD3_ASCII_RULES,host.data(),static_cast<int>(host.size()),ascii.data(),needed))return {};
 for(auto& c:ascii)c=static_cast<wchar_t>(towlower(c));for(const auto& label:split(ascii,L'.')){if(label.empty()||label.size()>63||label.front()==L'-'||label.back()==L'-')return {};for(auto c:label)if(!((c>=L'a'&&c<=L'z')||(c>=L'0'&&c<=L'9')||c==L'-'))return {};}
 return ascii;
}
std::wstring profileKey(const Profile& profile){return profile.website.empty()?profile.executable:profile.executable+L"|"+profile.website;}
bool sameProfile(const Profile& first,const Profile& second){return first.executable==second.executable&&first.website==second.website;}
Profile* findProfile(Store& store,const std::wstring& key){auto found=std::find_if(store.profiles.begin(),store.profiles.end(),[&](const auto& profile){return profileKey(profile)==key;});return found==store.profiles.end()?nullptr:&*found;}
const Profile* findProfile(const Store& store,const std::wstring& key){auto found=std::find_if(store.profiles.begin(),store.profiles.end(),[&](const auto& profile){return profileKey(profile)==key;});return found==store.profiles.end()?nullptr:&*found;}
void removeDockShortcut(Profile& profile,size_t erasedIndex){auto& indices=profile.dock.shortcuts;std::erase(indices,erasedIndex);for(auto& index:indices)if(index>erasedIndex)--index;}
bool matchesShortcut(const Shortcut& shortcut,const std::wstring& query) {
 auto normalized=[](std::wstring s){for(auto& c:s)c=c==L'+'?L' ':static_cast<wchar_t>(towlower(c));return s;};
 auto haystack=normalized(shortcut.title+L" "+shortcut.category+L" "+formatChord(shortcut.chord));
 std::wistringstream words(normalized(query));std::wstring word;
 while(words>>word)if(haystack.find(word)==haystack.npos)return false;return true;
}
bool validStore(const Store& s) {
 if(static_cast<UINT>(s.theme)>2||s.profiles.size()>256||!validChord(s.trigger)||!s.trigger.modifiers||s.trigger.key==VK_F12)return false;
 std::set<std::wstring> apps;size_t count=0,workflowCount=0,stepCount=0;
 for(const auto& p:s.profiles) {
 if(p.executable.empty()||p.executable.size()>260||appKey(p.executable)!=p.executable||!p.executable.ends_with(L".exe")||!apps.insert(profileKey(p)).second||trim(p.name).empty()||p.name.size()>120||p.shortcuts.size()>2048)return false;
 if(!p.website.empty()&&(!isBrowserExecutable(p.executable)||normalizeWebsite(p.website)!=p.website))return false;
 const auto& dock=p.dock;if(dock.scale<50||dock.scale>200||dock.opacity<20||dock.opacity>100||dock.x<-32768||dock.x>32767||dock.y<-32768||dock.y>32767||dock.shortcuts.size()>64)return false;
 std::set<size_t> indices;for(auto index:dock.shortcuts)if(index>=p.shortcuts.size()||!indices.insert(index).second)return false;
 for(const auto& k:p.shortcuts)if(trim(k.title).empty()||k.title.size()>240||k.category.size()>80||!validChord(k.chord)||++count>20000)return false;
 if(p.workflows.size()>256)return false;
 for(const auto& flow:p.workflows){if(trim(flow.title).empty()||flow.title.size()>240||flow.steps.empty()||flow.steps.size()>128||++workflowCount>4096)return false;for(const auto& step:flow.steps)if(trim(step).empty()||step.size()>4096||++stepCount>20000)return false;}
 }return true;
}
std::wstring serialize(const Store& s) {
 std::wostringstream out;out<<L"QC\t4\nH\t"<<s.trigger.modifiers<<L'\t'<<s.trigger.key<<L"\nT\t"<<static_cast<UINT>(s.theme)<<L'\n';
 for(const auto& p:s.profiles) {
 out<<L"P\t"<<escape(p.executable)<<L'\t'<<escape(p.name)<<L'\t'<<escape(p.website)<<L'\n';
 const auto& dock=p.dock;out<<L"O\t"<<dock.enabled<<L'\t'<<dock.vertical<<L'\t'<<dock.collapsed<<L'\t'<<dock.scale<<L'\t'<<dock.opacity<<L'\t'<<dock.x<<L'\t'<<dock.y<<L'\n';for(auto index:dock.shortcuts)out<<L"K\t"<<index<<L'\n';
 for(const auto& k:p.shortcuts)out<<L"S\t"<<escape(k.title)<<L'\t'<<escape(k.category)<<L'\t'<<k.chord.modifiers<<L'\t'<<k.chord.key<<L'\n';
 for(const auto& flow:p.workflows){out<<L"W\t"<<escape(flow.title)<<L'\n';for(const auto& step:flow.steps)out<<L"D\t"<<escape(step)<<L'\n';out<<L"F\n";}
 out<<L"E\n";
 }out<<L"Z\n";return out.str();
}
bool deserialize(const std::wstring& data,Store& target) {
 // Valid UTF-8 never decodes into more UTF-16 units than its byte count.
 if(data.size()>maxStoreBytes||data.find(L'\0')!=data.npos)return false;
 auto lines=split(data,L'\n');if(lines.size()<4||(lines[0]!=L"QC\t1"&&lines[0]!=L"QC\t2"&&lines[0]!=L"QC\t3"&&lines[0]!=L"QC\t4")||!lines.back().empty()||lines[lines.size()-2]!=L"Z")return false;
 Store s;auto header=split(lines[1],L'\t');UINT key=0;
 if(header.size()!=3||header[0]!=L"H"||!number(header[1],s.trigger.modifiers)||!number(header[2],key))return false;s.trigger.key=static_cast<WORD>(key);
 size_t start=2;
 if(lines[0]!=L"QC\t1"){auto theme=split(lines[2],L'\t');UINT mode=0;if(theme.size()!=2||theme[0]!=L"T"||!number(theme[1],mode)||mode>2)return false;s.theme=static_cast<ThemeMode>(mode);start=3;}
 bool inProfile=false,inWorkflow=false,dockSeen=false;const bool version4=lines[0]==L"QC\t4";
 for(size_t i=start;i+2<lines.size();++i) {
 auto row=split(lines[i],L'\t');
 if(row[0]==L"P"&&row.size()==(version4?4:3)&&!inProfile) {
 Profile p;if(!unescape(row[1],p.executable)||!unescape(row[2],p.name)||(version4&&!unescape(row[3],p.website)))return false;s.profiles.push_back(std::move(p));inProfile=true;dockSeen=false;
 }else if(row[0]==L"O"&&row.size()==8&&version4&&inProfile&&!inWorkflow&&!dockSeen){
 auto& dock=s.profiles.back().dock;UINT enabled{},vertical{},collapsed{};auto signedValue=[](const std::wstring& value,int& out){bool negative=!value.empty()&&value[0]==L'-';UINT n{};if(!number(negative?value.substr(1):value,n)||n>(negative?32768u:32767u))return false;out=negative?-static_cast<int>(n):static_cast<int>(n);return true;};
 if(!number(row[1],enabled)||enabled>1||!number(row[2],vertical)||vertical>1||!number(row[3],collapsed)||collapsed>1||!number(row[4],dock.scale)||!number(row[5],dock.opacity)||!signedValue(row[6],dock.x)||!signedValue(row[7],dock.y))return false;dock.enabled=enabled!=0;dock.vertical=vertical!=0;dock.collapsed=collapsed!=0;dockSeen=true;
 }else if(row[0]==L"K"&&row.size()==2&&version4&&inProfile&&!inWorkflow&&dockSeen){UINT index{};if(!number(row[1],index)||s.profiles.back().dock.shortcuts.size()>=64)return false;s.profiles.back().dock.shortcuts.push_back(index);
 }else if(row[0]==L"S"&&row.size()==5&&inProfile&&!inWorkflow) {
 Shortcut k;if(!unescape(row[1],k.title)||!unescape(row[2],k.category)||!number(row[3],k.chord.modifiers)||!number(row[4],key))return false;
 k.chord.key=static_cast<WORD>(key);s.profiles.back().shortcuts.push_back(std::move(k));
 }else if(row[0]==L"W"&&row.size()==2&&inProfile&&!inWorkflow&&(lines[0]==L"QC\t3"||version4)){
 Workflow flow;if(!unescape(row[1],flow.title)||s.profiles.back().workflows.size()>=256)return false;s.profiles.back().workflows.push_back(std::move(flow));inWorkflow=true;
 }else if(row[0]==L"D"&&row.size()==2&&inWorkflow){std::wstring step;if(!unescape(row[1],step)||s.profiles.back().workflows.back().steps.size()>=128)return false;s.profiles.back().workflows.back().steps.push_back(std::move(step));
 }else if(row[0]==L"F"&&row.size()==1&&inWorkflow)inWorkflow=false;
 else if(row[0]==L"E"&&row.size()==1&&inProfile&&!inWorkflow&&(!version4||dockSeen))inProfile=false;else return false;
 }if(inProfile||inWorkflow||!validStore(s))return false;target=std::move(s);return true;
}
}
