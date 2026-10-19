#include "internal.hpp"
#include <algorithm>
#include <array>
namespace qc::community {
using namespace detail;
namespace {
Bytes jsonBytes(const JsonObject& object){auto text=utf8(std::wstring(object.Stringify()));return Bytes(text.begin(),text.end());}
std::wstring queryPath(const Query& query){require(query.search.size()<=128&&query.executable.size()<=128&&query.hostname.size()<=253&&query.language.size()<=35&&query.keyboardLayout.size()<=64&&query.cursor.size()<=32);return L"/catalog?q="+encode(query.search)+L"&executable="+encode(query.executable)+L"&hostname="+encode(query.hostname)+L"&language="+encode(query.language)+L"&keyboardLayout="+encode(query.keyboardLayout)+L"&cursor="+encode(query.cursor);}
std::wstring key(const std::wstring& value){auto text=utf8(value);return sha256(std::span(reinterpret_cast<const std::uint8_t*>(text.data()),text.size()));}
std::array<unsigned,3> numbers(const std::wstring& text){require(version(text));std::array<unsigned,3> values{};size_t position{};for(unsigned i=0;i<3;++i){unsigned digits{};while(position<text.size()&&text[position]>=L'0'&&text[position]<=L'9'){require(++digits<=4);values[i]=values[i]*10+text[position++]-L'0';}require(digits>0);if(i<2){require(position<text.size()&&text[position]==L'.');++position;}}require(position==text.size()||text[position]==L'-');return values;}
}
bool list(const Query& query,const std::filesystem::path& directory,Page& result,std::wstring& error,const std::atomic_bool* cancel){
 return guarded(error,[&]{Apartment apartment;auto path=queryPath(query),queryHash=key(path);Bytes response;std::wstring networkError;auto cache=directory/L"community"/L"catalog-cache.json";
  if(get(path,256*1024,response,networkError,cancel)){
   auto parsed=object(response);auto current=page(parsed);JsonObject wrapper;wrapper.Insert(L"schemaVersion",JsonValue::CreateNumberValue(1));wrapper.Insert(L"queryHash",JsonValue::CreateStringValue(queryHash));wrapper.Insert(L"page",parsed);
   std::wstring cacheError;if(!guarded(cacheError,[&]{auto data=jsonBytes(wrapper);write(cache,data);}))current.warning=L"Catalog loaded, but offline cache could not be saved.";result=std::move(current);return;
  }
  if(cancel&&cancel->load())fail(L"Community request canceled.");
  std::wstring cacheError;bool loaded=guarded(cacheError,[&]{auto wrapper=object(read(cache,1024*1024));require(number(wrapper,L"schemaVersion",1)==1&&field(wrapper,L"queryHash",64)==queryHash);auto cached=page(wrapper.GetNamedObject(L"page"));cached.fromCache=true;cached.warning=networkError;result=std::move(cached);});
  if(!loaded)throw Failure{networkError};
 });
}
bool prepare(const Entry& source,const std::wstring& clientVersion,const std::filesystem::path& directory,PreparedCollection& prepared,std::wstring& error,const std::atomic_bool* cancel){
 return guarded(error,[&]{Apartment apartment;require(identifier(source.id)&&version(source.version));require(numbers(clientVersion)>=numbers(source.minimumClientVersion),L"This collection requires a newer Quick Cheat version.");Bytes data;std::wstring networkError;
  if(!get(L"/collections/"+encode(source.id)+L"/versions/"+encode(source.version)+L"/download",32*1024*1024,data,networkError,cancel))throw Failure{networkError};require(sha256(data)==source.sha256,L"Community collection hash mismatch.");
  struct Temporary{std::filesystem::path path;bool owned{};~Temporary(){if(owned)DeleteFileW(path.c_str());}} file{temporary(directory/L"community")};write(file.path,data,false);file.owned=true;Collection collection;std::wstring parseError;if(!readCollection(file.path,collection,parseError))throw Failure{parseError};
  require(collection.profile.executable==source.executable&&collection.profile.website==source.hostname,L"Community collection target does not match its listing.");require(collection.profile.shortcuts.size()==source.shortcuts&&collection.profile.workflows.size()==source.workflows&&collection.images.size()==source.images,L"Community collection counts do not match its listing.");
  PreparedCollection result{source,std::move(collection),std::move(data)};prepared=std::move(result);
 });
}
bool recordInstallation(const PreparedCollection& prepared,const Store& current,const std::filesystem::path& directory,std::wstring& error){
 return guarded(error,[&]{Apartment apartment;require(identifier(prepared.source.id)&&version(prepared.source.version)&&!prepared.baseline.empty()&&sha256(prepared.baseline)==prepared.source.sha256);require(findProfile(current,profileKey(prepared.collection.profile))!=nullptr,L"Save the installed profile before recording its origin.");auto installed=directory/L"community"/L"installed";auto baseline=installed/(prepared.source.sha256+L".quickcheat.json");
  if(std::filesystem::exists(baseline))require(sha256(read(baseline,32*1024*1024))==prepared.source.sha256,L"Installed collection baseline is damaged.");else write(baseline,prepared.baseline,false);
  JsonObject metadata;metadata.Insert(L"schemaVersion",JsonValue::CreateNumberValue(1));metadata.Insert(L"origin",JsonValue::CreateStringValue(Origin));metadata.Insert(L"collectionId",JsonValue::CreateStringValue(prepared.source.id));metadata.Insert(L"version",JsonValue::CreateStringValue(prepared.source.version));metadata.Insert(L"sha256",JsonValue::CreateStringValue(prepared.source.sha256));metadata.Insert(L"profileKey",JsonValue::CreateStringValue(profileKey(prepared.collection.profile)));metadata.Insert(L"license",JsonValue::CreateStringValue(prepared.source.license));auto data=jsonBytes(metadata);write(installed/(prepared.source.id+L".json"),data);
 });
}
}
