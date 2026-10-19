#include "internal.hpp"
#include <algorithm>
#include <cmath>
namespace qc::community::detail {
JsonObject object(const Bytes& bytes){auto text=wide(bytes);unsigned depth{},tokens{};bool quoted{},escape{};for(auto c:text){if(quoted){if(escape)escape=false;else if(c==L'\\')escape=true;else if(c==L'"')quoted=false;continue;}if(c==L'"')quoted=true;else if(c==L'{'||c==L'['){require(++depth<=12&&++tokens<=20000); }else if(c==L'}'||c==L']'){require(depth>0);--depth;}}require(depth==0&&!quoted);return JsonObject::Parse(text);}
std::wstring field(const JsonObject& object,const wchar_t* key,size_t maximum,bool empty){auto value=object.GetNamedValue(key);require(value.ValueType()==JsonValueType::String);auto text=value.GetString();std::wstring out(text.c_str(),text.size());require(out.size()<=maximum&&(empty||!out.empty())&&out.find(L'\0')==out.npos);return out;}
unsigned number(const JsonObject& object,const wchar_t* key,unsigned maximum){auto value=object.GetNamedValue(key);require(value.ValueType()==JsonValueType::Number);double n=value.GetNumber();require(std::isfinite(n)&&n>=0&&n<=maximum&&std::trunc(n)==n);return static_cast<unsigned>(n);}
bool identifier(const std::wstring& value){auto letter=[](wchar_t c){return (c>=L'a'&&c<=L'z')||(c>=L'0'&&c<=L'9');};return !value.empty()&&value.size()<=64&&letter(value.front())&&std::all_of(value.begin(),value.end(),[&](wchar_t c){return letter(c)||c==L'-';});}
bool version(const std::wstring& value){if(value.empty()||value.size()>64||value.front()<L'0'||value.front()>L'9'||value.find(L"..")!=value.npos)return false;return std::all_of(value.begin(),value.end(),[](wchar_t c){return (c>=L'0'&&c<=L'9')||(c>=L'a'&&c<=L'z')||c==L'.'||c==L'-';});}
Page page(const JsonObject& root){
 require(number(root,L"schemaVersion",1)==1);Page out;out.revision=field(root,L"revision",16);require(out.revision.size()==16);out.nextCursor=field(root,L"nextCursor",32,true);out.total=number(root,L"total",500);
 auto items=root.GetNamedArray(L"items");require(items.Size()<=20&&items.Size()<=out.total);
 for(const auto& item:items){auto row=item.GetObject();Entry entry;entry.id=field(row,L"id",64);entry.version=field(row,L"version",64);require(identifier(entry.id)&&version(entry.version));entry.title=field(row,L"title",160);entry.summary=field(row,L"summary",2048,true);
  auto author=row.GetNamedObject(L"author");entry.authorId=field(author,L"id",64);require(identifier(entry.authorId));entry.authorName=field(author,L"displayName",128);
  auto target=row.GetNamedObject(L"target");entry.executable=field(target,L"executable",128);entry.hostname=field(target,L"hostname",253,true);
  auto compatibility=row.GetNamedObject(L"compatibility");require(field(compatibility,L"platform",16)==L"windows"&&field(compatibility,L"architecture",16)==L"x64");entry.minimumClientVersion=field(compatibility,L"minimumClientVersion",64);require(version(entry.minimumClientVersion));entry.language=field(compatibility,L"language",35);entry.keyboardLayout=field(compatibility,L"keyboardLayout",64);
  entry.sha256=field(row,L"sha256",64);require(entry.sha256.size()==64&&std::all_of(entry.sha256.begin(),entry.sha256.end(),[](wchar_t c){return (c>=L'0'&&c<=L'9')||(c>=L'a'&&c<=L'f');}));entry.collectionFormat=number(row,L"collectionFormat",2);require(entry.collectionFormat>=1);
  auto counts=row.GetNamedObject(L"counts");entry.shortcuts=number(counts,L"shortcuts",10000);entry.workflows=number(counts,L"workflows",128);entry.images=number(counts,L"images",128);entry.votes=number(row,L"votes",1000000000);entry.license=field(row,L"license",128);entry.updatedAt=field(row,L"updatedAt",40);
  require(std::none_of(out.items.begin(),out.items.end(),[&](const auto& existing){return existing.id==entry.id;}));out.items.push_back(std::move(entry));
 }
 return out;
}
}
