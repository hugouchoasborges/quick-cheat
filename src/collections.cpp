#include "collections.hpp"
#include "attachments.hpp"
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <wincrypt.h>
#include <md4c.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <string_view>
namespace qc {
namespace {
using namespace winrt::Windows::Data::Json;
constexpr size_t maxCollectionBytes=32*1024*1024,maxImageBytes=4*1024*1024,maxImagesBytes=8*1024*1024,maxImages=128;
struct Problem {std::wstring message;};
[[noreturn]] void reject(const std::wstring& message){throw Problem{message};}
struct Handle {HANDLE value;~Handle(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);}};
std::wstring lower(std::wstring value){for(auto& c:value)if(c>=L'A'&&c<=L'Z')c+=L'a'-L'A';return value;}
std::vector<std::uint8_t> readBytes(const std::filesystem::path& path,size_t maximum){
 Handle file{CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr)};
 if(file.value==INVALID_HANDLE_VALUE)reject(L"Cannot read the collection or image file: "+path.filename().wstring());
 LARGE_INTEGER size{};if(!GetFileSizeEx(file.value,&size)||size.QuadPart<=0||static_cast<unsigned long long>(size.QuadPart)>maximum)reject(L"The collection or image file is empty or exceeds its size limit.");
 std::vector<std::uint8_t> bytes(static_cast<size_t>(size.QuadPart));DWORD count{};
 if(!ReadFile(file.value,bytes.data(),static_cast<DWORD>(bytes.size()),&count,nullptr)||count!=bytes.size())reject(L"Cannot read the complete collection or image file.");return bytes;
}
std::string utf8(const std::wstring& text){
 int length=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);if(length<=0)reject(L"Invalid Unicode text in the collection.");
 std::string bytes(length,'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),bytes.data(),length,nullptr,nullptr);return bytes;
}
std::wstring wide(const std::vector<std::uint8_t>& bytes){
 auto data=reinterpret_cast<const char*>(bytes.data());int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,data,static_cast<int>(bytes.size()),nullptr,0);if(length<=0)reject(L"The collection must be valid UTF-8 JSON.");
 std::wstring text(length,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,data,static_cast<int>(bytes.size()),text.data(),length);if(!text.empty()&&text.front()==0xfeff)text.erase(0,1);return text;
}
void checkJsonDepth(const std::wstring& text){
 bool quoted=false,escaped=false;int depth=0;size_t tokens=0;
 for(auto c:text){if(quoted){if(escaped)escaped=false;else if(c==L'\\')escaped=true;else if(c==L'"')quoted=false;continue;}if(c==L'"')quoted=true;else if(c==L'{'||c==L'['){if(++depth>12||++tokens>50000)reject(L"The collection JSON is too complex.");}else if(c==L'}'||c==L']')--depth;}
}
std::wstring stringValue(const IJsonValue& value,size_t maximum){
 if(value.ValueType()!=JsonValueType::String)reject(L"Invalid text field in the collection.");auto text=value.GetString();std::wstring result(text.c_str(),text.size());if(result.size()>maximum||result.find(L'\0')!=result.npos)reject(L"A collection text field exceeds its limit or contains a null character.");return result;
}
std::wstring field(const JsonObject& object,const wchar_t* key,size_t maximum){return stringValue(object.GetNamedValue(key),maximum);}
unsigned integer(const IJsonValue& value,unsigned maximum){
 if(value.ValueType()!=JsonValueType::Number)reject(L"Invalid numeric field in the collection.");double number=value.GetNumber();if(!std::isfinite(number)||number<0||number>maximum||std::trunc(number)!=number)reject(L"Invalid numeric value in the collection.");return static_cast<unsigned>(number);
}
int signedInteger(const IJsonValue& value,int minimum,int maximum){
 if(value.ValueType()!=JsonValueType::Number)reject(L"Invalid numeric field in the collection.");double number=value.GetNumber();if(!std::isfinite(number)||number<minimum||number>maximum||std::trunc(number)!=number)reject(L"Invalid signed numeric value in the collection.");return static_cast<int>(number);
}
JsonValue textValue(const std::wstring& text){return JsonValue::CreateStringValue(winrt::hstring(text));}
void validateProfile(const Profile& profile){Store single;single.profiles.push_back(profile);if(!validStore(single)||utf8(serialize(single)).size()>maxStoreBytes)reject(L"The collection exceeds application limits or contains invalid shortcuts or workflows.");}
std::wstring attribute(const MD_ATTRIBUTE& attribute){
 std::wstring value(attribute.text,attribute.size);size_t at=0;
 while((at=value.find(L'&',at))!=value.npos){auto end=value.find(L';',at);if(end==value.npos||end-at>16){++at;continue;}auto entity=value.substr(at,end-at+1);wchar_t decoded=0;
  if(entity==L"&amp;")decoded=L'&';else if(entity==L"&quot;")decoded=L'"';else if(entity==L"&apos;")decoded=L'\'';else if(entity==L"&lt;")decoded=L'<';else if(entity==L"&gt;")decoded=L'>';
  else if(entity.starts_with(L"&#")){try{auto digits=entity.substr(2,entity.size()-3);int base=10;if(digits.starts_with(L"x")||digits.starts_with(L"X")){digits.erase(0,1);base=16;}size_t used{};auto number=std::stoul(digits,&used,base);if(used==digits.size()&&number>0&&number<128)decoded=static_cast<wchar_t>(number);}catch(...){}}
  if(decoded){value.replace(at,entity.size(),1,decoded);++at;}else at=end+1;
 }return value;
}
struct References {std::set<std::wstring> paths;bool invalid{};};
std::set<std::wstring> imageReferences(const Profile& profile){
 References references;MD_PARSER parser{};parser.flags=MD_FLAG_NOHTML|MD_FLAG_STRIKETHROUGH|MD_FLAG_TASKLISTS;
 parser.enter_block=[](MD_BLOCKTYPE,void*,void*){return 0;};parser.leave_block=parser.enter_block;
 parser.leave_span=[](MD_SPANTYPE,void*,void*){return 0;};parser.text=[](MD_TEXTTYPE,const MD_CHAR*,MD_SIZE,void*){return 0;};
 parser.enter_span=[](MD_SPANTYPE type,void* detail,void* context){auto& refs=*static_cast<References*>(context);try{if(type==MD_SPAN_IMG){auto path=attribute(static_cast<MD_SPAN_IMG_DETAIL*>(detail)->src);if(path.starts_with(L"attachments/")){if(!isWorkflowImageReference(path)){refs.invalid=true;return 1;}refs.paths.insert(lower(std::move(path)));}}return 0;}catch(...){refs.invalid=true;return 1;}};
 for(const auto& flow:profile.workflows)for(const auto& step:flow.steps)if(md_parse(step.c_str(),static_cast<MD_SIZE>(step.size()),&parser,&references)!=0)reject(L"Cannot read managed image references in a workflow.");
 if(references.invalid||references.paths.size()>maxImages)reject(L"Invalid attachment reference or more than 128 referenced images.");return references.paths;
}
void checkPng(const std::vector<std::uint8_t>& bytes){constexpr std::uint8_t signature[]={137,80,78,71,13,10,26,10};if(bytes.size()<sizeof(signature)||!std::equal(std::begin(signature),std::end(signature),bytes.begin()))reject(L"Collection images must be PNG files.");}
void checkPlainPath(const std::filesystem::path& path,bool directory){auto attributes=GetFileAttributesW(path.c_str());if(attributes==INVALID_FILE_ATTRIBUTES||(attributes&FILE_ATTRIBUTE_REPARSE_POINT)||bool(attributes&FILE_ATTRIBUTE_DIRECTORY)!=directory)reject(L"The managed attachments path is unavailable or uses a link.");}
std::wstring base64(const std::vector<std::uint8_t>& bytes){
 DWORD count{};if(!CryptBinaryToStringW(bytes.data(),static_cast<DWORD>(bytes.size()),CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF,nullptr,&count))reject(L"Cannot encode an image.");std::wstring result(count,L'\0');if(!CryptBinaryToStringW(bytes.data(),static_cast<DWORD>(bytes.size()),CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF,result.data(),&count))reject(L"Cannot encode an image.");if(!result.empty()&&result.back()==L'\0')result.pop_back();return result;
}
std::vector<std::uint8_t> unbase64(const std::wstring& text){
 DWORD count{};if(!CryptStringToBinaryW(text.data(),static_cast<DWORD>(text.size()),CRYPT_STRING_BASE64|CRYPT_STRING_STRICT,nullptr,&count,nullptr,nullptr)||!count||count>maxImageBytes)reject(L"An embedded image is invalid or exceeds 4 MiB.");std::vector<std::uint8_t> bytes(count);if(!CryptStringToBinaryW(text.data(),static_cast<DWORD>(text.size()),CRYPT_STRING_BASE64|CRYPT_STRING_STRICT,bytes.data(),&count,nullptr,nullptr))reject(L"Invalid Base64 image data.");bytes.resize(count);checkPng(bytes);return bytes;
}
void writeHandle(HANDLE file,const void* bytes,size_t length){DWORD count{};if(!WriteFile(file,bytes,static_cast<DWORD>(length),&count,nullptr)||count!=length||!FlushFileBuffers(file))reject(L"Cannot write the complete collection or image file.");}
void atomicExport(const std::filesystem::path& destination,const std::string& bytes){
 GUID id{};wchar_t suffix[40]{};if(FAILED(CoCreateGuid(&id))||!StringFromGUID2(id,suffix,40))reject(L"Cannot create a temporary collection name.");auto temporary=destination.wstring()+L".tmp."+suffix;bool owned=false;
 try{{Handle file{CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr)};if(file.value==INVALID_HANDLE_VALUE)reject(L"Cannot create the collection file in the chosen folder.");owned=true;writeHandle(file.value,bytes.data(),bytes.size());}if(!MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))reject(L"Cannot replace the chosen collection file.");}catch(...){if(owned)DeleteFileW(temporary.c_str());throw;}
}
template<class Action> bool guarded(std::wstring& error,Action&& action){error.clear();try{action();return true;}catch(const Problem& problem){error=problem.message;}catch(const winrt::hresult_error&){error=L"Invalid or unsupported collection JSON, or a Windows data service is unavailable.";}catch(const std::exception&){error=L"Cannot process this collection. No profile changes were saved.";}return false;}
}
bool exportCollection(const std::filesystem::path& destination,const Profile& profile,const std::filesystem::path& dataDirectory,std::wstring& error){return guarded(error,[&]{
 validateProfile(profile);JsonObject root;root.Insert(L"format",textValue(L"quick-cheat.collection"));root.Insert(L"version",JsonValue::CreateNumberValue(3));JsonObject application;application.Insert(L"executable",textValue(profile.executable));application.Insert(L"name",textValue(profile.name));application.Insert(L"website",textValue(profile.website));root.Insert(L"application",application);
 JsonObject dock;dock.Insert(L"enabled",JsonValue::CreateBooleanValue(profile.dock.enabled));dock.Insert(L"vertical",JsonValue::CreateBooleanValue(profile.dock.vertical));dock.Insert(L"collapsed",JsonValue::CreateBooleanValue(profile.dock.collapsed));dock.Insert(L"scale",JsonValue::CreateNumberValue(profile.dock.scale));dock.Insert(L"opacity",JsonValue::CreateNumberValue(profile.dock.opacity));dock.Insert(L"x",JsonValue::CreateNumberValue(profile.dock.x));dock.Insert(L"y",JsonValue::CreateNumberValue(profile.dock.y));JsonArray dockShortcuts;for(auto index:profile.dock.shortcuts)dockShortcuts.Append(JsonValue::CreateNumberValue(static_cast<double>(index)));dock.Insert(L"shortcuts",dockShortcuts);root.Insert(L"dock",dock);
 JsonArray shortcuts;for(const auto& shortcut:profile.shortcuts){JsonObject item;item.Insert(L"title",textValue(shortcut.title));item.Insert(L"category",textValue(shortcut.category));item.Insert(L"modifiers",JsonValue::CreateNumberValue(shortcut.chord.modifiers));item.Insert(L"key",JsonValue::CreateNumberValue(shortcut.chord.key));item.Insert(L"url",textValue(shortcut.url));item.Insert(L"dockLabel",textValue(shortcut.dockLabel));shortcuts.Append(item);}root.Insert(L"shortcuts",shortcuts);
 JsonArray workflows;for(const auto& flow:profile.workflows){JsonObject item;item.Insert(L"title",textValue(flow.title));JsonArray steps;for(const auto& step:flow.steps)steps.Append(textValue(step));item.Insert(L"steps",steps);workflows.Append(item);}root.Insert(L"workflows",workflows);
 JsonArray images;size_t total=0;auto references=imageReferences(profile);if(!references.empty())checkPlainPath(dataDirectory/L"attachments",true);
 for(const auto& reference:references){auto path=dataDirectory/reference;checkPlainPath(path,false);auto bytes=readBytes(path,maxImageBytes);checkPng(bytes);if((total+=bytes.size())>maxImagesBytes)reject(L"Referenced images exceed the collection limit of 8 MiB.");PreparedImage prepared;std::wstring imageError;if(!loadWorkflowImage(dataDirectory,reference,128,prepared,imageError))reject(imageError);JsonObject item;item.Insert(L"reference",textValue(reference));item.Insert(L"pngBase64",textValue(base64(bytes)));images.Append(item);}root.Insert(L"images",images);
 auto output=utf8(std::wstring(root.Stringify()));if(output.size()>maxCollectionBytes)reject(L"The collection exceeds 32 MiB.");atomicExport(destination,output);
});}
bool readCollection(const std::filesystem::path& source,Collection& collection,std::wstring& error){return guarded(error,[&]{
 auto bytes=readBytes(source,maxCollectionBytes);auto text=wide(bytes);checkJsonDepth(text);auto root=JsonObject::Parse(text);bytes.clear();bytes.shrink_to_fit();text.clear();text.shrink_to_fit();
 auto version=integer(root.GetNamedValue(L"version"),3);if(field(root,L"format",80)!=L"quick-cheat.collection"||(version!=1&&version!=2&&version!=3))reject(L"Unsupported collection format or version.");
 Collection parsed;auto application=root.GetNamedObject(L"application");parsed.profile.executable=field(application,L"executable",260);parsed.profile.name=field(application,L"name",120);
 if(version>=2){parsed.profile.website=field(application,L"website",253);auto dock=root.GetNamedObject(L"dock");auto& settings=parsed.profile.dock;settings.enabled=dock.GetNamedBoolean(L"enabled");settings.vertical=dock.GetNamedBoolean(L"vertical");settings.collapsed=dock.GetNamedBoolean(L"collapsed");settings.scale=integer(dock.GetNamedValue(L"scale"),200);settings.opacity=integer(dock.GetNamedValue(L"opacity"),100);settings.x=signedInteger(dock.GetNamedValue(L"x"),-32768,32767);settings.y=signedInteger(dock.GetNamedValue(L"y"),-32768,32767);auto selected=dock.GetNamedArray(L"shortcuts");if(selected.Size()>64)reject(L"A dock supports up to 64 shortcuts.");for(const auto& index:selected)settings.shortcuts.push_back(integer(index,2047));}
 auto shortcuts=root.GetNamedArray(L"shortcuts");if(shortcuts.Size()>2048)reject(L"A collection supports up to 2048 shortcuts.");for(const auto& value:shortcuts){auto item=value.GetObject();parsed.profile.shortcuts.push_back({field(item,L"title",240),field(item,L"category",80),{integer(item.GetNamedValue(L"modifiers"),0xffff),static_cast<WORD>(integer(item.GetNamedValue(L"key"),0xffff))},version>=3?field(item,L"url",2048):L"",version>=3?field(item,L"dockLabel",120):L""});}
 auto workflows=root.GetNamedArray(L"workflows");if(workflows.Size()>256)reject(L"A collection supports up to 256 workflows.");for(const auto& value:workflows){auto item=value.GetObject();Workflow flow;flow.title=field(item,L"title",240);auto steps=item.GetNamedArray(L"steps");if(steps.Size()>128)reject(L"A workflow supports up to 128 steps.");for(const auto& step:steps)flow.steps.push_back(stringValue(step,4096));parsed.profile.workflows.push_back(std::move(flow));}validateProfile(parsed.profile);
 auto images=root.GetNamedArray(L"images");if(images.Size()>maxImages)reject(L"A collection supports up to 128 images.");size_t total=0;std::set<std::wstring> provided;
 for(const auto& value:images){auto item=value.GetObject();auto reference=lower(field(item,L"reference",100));if(!isWorkflowImageReference(reference)||!provided.insert(reference).second)reject(L"An image reference is unsafe or duplicated.");auto data=unbase64(field(item,L"pngBase64",6*1024*1024));if((total+=data.size())>maxImagesBytes)reject(L"Embedded images exceed 8 MiB.");parsed.images.push_back({std::move(reference),std::move(data)});}
 if(imageReferences(parsed.profile)!=provided)reject(L"The collection has missing or unreferenced image attachments.");collection=std::move(parsed);
});}
bool mergeCollection(const Store& current,const Collection& collection,bool replace,Store& candidate,CollectionImportCounts& counts,std::wstring& error){return guarded(error,[&]{
 validateProfile(collection.profile);candidate=current;counts={};auto found=std::find_if(candidate.profiles.begin(),candidate.profiles.end(),[&](const auto& profile){return sameProfile(profile,collection.profile);});
 if(found==candidate.profiles.end()){candidate.profiles.push_back(collection.profile);counts.shortcuts=collection.profile.shortcuts.size();counts.workflows=collection.profile.workflows.size();}
 else if(replace){*found=collection.profile;counts.shortcuts=collection.profile.shortcuts.size();counts.workflows=collection.profile.workflows.size();}
 else{std::vector<size_t> mapped;for(const auto& shortcut:collection.profile.shortcuts){auto duplicate=std::find_if(found->shortcuts.begin(),found->shortcuts.end(),[&](const auto& saved){return saved.title==shortcut.title&&saved.category==shortcut.category&&saved.chord.modifiers==shortcut.chord.modifiers&&saved.chord.key==shortcut.chord.key&&saved.url==shortcut.url&&saved.dockLabel==shortcut.dockLabel;});if(duplicate!=found->shortcuts.end()){mapped.push_back(static_cast<size_t>(duplicate-found->shortcuts.begin()));++counts.duplicates;}else{mapped.push_back(found->shortcuts.size());found->shortcuts.push_back(shortcut);++counts.shortcuts;}}
  for(auto source:collection.profile.dock.shortcuts){auto index=mapped[source];if(std::find(found->dock.shortcuts.begin(),found->dock.shortcuts.end(),index)==found->dock.shortcuts.end())found->dock.shortcuts.push_back(index);}
  for(const auto& flow:collection.profile.workflows){bool duplicate=std::any_of(found->workflows.begin(),found->workflows.end(),[&](const auto& saved){return saved.title==flow.title&&saved.steps==flow.steps;});if(duplicate)++counts.duplicates;else{found->workflows.push_back(flow);++counts.workflows;}}}
 if(!validStore(candidate)||utf8(serialize(candidate)).size()>maxStoreBytes)reject(L"Import would exceed your local library limits. No changes were saved.");
});}
CollectionImageInstall::~CollectionImageInstall(){if(!committed){for(const auto& path:created)DeleteFileW(path.c_str());if(createdDirectory)RemoveDirectoryW(directory.c_str());}}
bool CollectionImageInstall::install(const Collection& collection,const std::filesystem::path& dataDirectory,std::wstring& error){return guarded(error,[&]{
 if(collection.images.empty())return;directory=std::filesystem::absolute(dataDirectory)/L"attachments";std::error_code ec;createdDirectory=std::filesystem::create_directories(directory,ec);if(ec)reject(L"Cannot create the managed attachments folder.");checkPlainPath(directory,true);created.reserve(collection.images.size());
 for(const auto& image:collection.images){if(!isWorkflowImageReference(image.reference))reject(L"Unsafe image reference.");checkPng(image.bytes);auto destination=directory/image.reference.substr(12);auto attributes=GetFileAttributesW(destination.c_str());
  if(attributes!=INVALID_FILE_ATTRIBUTES){checkPlainPath(destination,false);if(readBytes(destination,maxImageBytes)!=image.bytes)reject(L"An attachment already exists with different content. Import canceled without overwriting it.");}
  else{created.push_back(destination);Handle file{CreateFileW(destination.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr)};if(file.value==INVALID_HANDLE_VALUE){created.pop_back();reject(L"Cannot create an imported attachment.");}writeHandle(file.value,image.bytes.data(),image.bytes.size());}
  PreparedImage prepared;std::wstring imageError;if(!loadWorkflowImage(directory.parent_path(),image.reference,128,prepared,imageError))reject(imageError);
 }
});}
}
