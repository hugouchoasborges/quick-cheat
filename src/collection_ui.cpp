#include "app.hpp"
#include "settings.hpp"
#include "workflows.hpp"
#include "collections.hpp"
#include <commdlg.h>
#include <algorithm>
namespace qc {
namespace {
constexpr int AddCollection=1001,ReplaceCollection=1002;
int previewCollection(HWND owner,const Collection& collection,const Profile* existing){
 auto& profile=collection.profile;size_t bytes=0;for(const auto& image:collection.images)bytes+=image.bytes.size();
 auto heading=L"Import collection for "+profile.name;
 auto content=profile.executable+(profile.website.empty()?L"":L" / "+profile.website)+L"\n\n"+std::to_wstring(profile.shortcuts.size())+L" shortcuts\n"+std::to_wstring(profile.workflows.size())+L" workflows\n"+std::to_wstring(collection.images.size())+L" images ("+std::to_wstring((bytes+1023)/1024)+L" KiB)";
 if(existing)content+=L"\n\nCurrent application: "+std::to_wstring(existing->shortcuts.size())+L" shortcuts and "+std::to_wstring(existing->workflows.size())+L" workflows.";
 TASKDIALOG_BUTTON buttons[]={{AddCollection,existing?L"Add to this application\nKeep existing content and skip exact duplicates.":L"Add this application\nImport its shortcuts, workflows and images."},{ReplaceCollection,L"Replace this application's content\nReplace its shortcuts and workflows with this collection."}};
 TASKDIALOGCONFIG dialog{};dialog.cbSize=sizeof(dialog);dialog.hwndParent=owner;dialog.dwFlags=TDF_USE_COMMAND_LINKS|TDF_ALLOW_DIALOG_CANCELLATION|TDF_SIZE_TO_CONTENT;dialog.dwCommonButtons=TDCBF_CANCEL_BUTTON;dialog.pszWindowTitle=L"Quick Cheat — Import collection";dialog.pszMainInstruction=heading.c_str();dialog.pszContent=content.c_str();dialog.cButtons=existing?2:1;dialog.pButtons=buttons;dialog.nDefaultButton=AddCollection;dialog.pszFooter=L"Other applications and global settings stay unchanged. Images are checked before saving.";
 int choice=IDCANCEL;if(FAILED(TaskDialogIndirect(&dialog,&choice,nullptr,nullptr)))return IDCANCEL;return choice;
}
}
void Editor::exportApplicationCollection(){
 if(profileIndex<0)return;if(pendingPageEdits()){app.error(L"Save your current edits before exporting the collection.",window);return;}
 const auto& profile=draft.profiles[profileIndex];wchar_t destination[32768]{};auto initial=profile.executable.substr(0,profile.executable.size()-4)+(profile.website.empty()?L"":L"-"+profile.website)+L".quickcheat.json";wcsncpy_s(destination,initial.c_str(),_TRUNCATE);
 OPENFILENAMEW dialog{sizeof(dialog)};dialog.hwndOwner=window;dialog.lpstrTitle=L"Export saved shortcuts, workflows and images";dialog.lpstrFilter=L"Quick Cheat collections (*.quickcheat.json)\0*.quickcheat.json\0\0";dialog.lpstrDefExt=L"quickcheat.json";dialog.lpstrFile=destination;dialog.nMaxFile=32768;dialog.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetSaveFileNameW(&dialog))return;std::wstring filename=std::filesystem::path(destination).filename().wstring();std::transform(filename.begin(),filename.end(),filename.begin(),[](wchar_t c){return static_cast<wchar_t>(towlower(c));});if(!filename.ends_with(L".quickcheat.json")){app.error(L"Use a filename ending in .quickcheat.json.",window);return;}
 std::wstring error;if(!exportCollection(destination,profile,app.file.parent_path(),error)){app.error(error,window);return;}
 auto message=L"Exported "+std::to_wstring(profile.shortcuts.size())+L" shortcuts and "+std::to_wstring(profile.workflows.size())+L" workflows with their referenced images.";SetWindowTextW(status,message.c_str());if(page==2&&app.workflowEditor)SetWindowTextW(app.workflowEditor->status,message.c_str());
}
void Editor::importApplicationCollection(){
 wchar_t source[32768]{};OPENFILENAMEW dialog{sizeof(dialog)};dialog.hwndOwner=window;dialog.lpstrTitle=L"Import application collection";dialog.lpstrFilter=L"Quick Cheat collections (*.quickcheat.json)\0*.quickcheat.json\0JSON files (*.json)\0*.json\0\0";dialog.lpstrFile=source;dialog.nMaxFile=32768;dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetOpenFileNameW(&dialog))return;Collection collection;std::wstring error;if(!readCollection(source,collection,error)){app.error(error,window);return;}
 auto existing=std::find_if(app.store.profiles.begin(),app.store.profiles.end(),[&](const auto& profile){return sameProfile(profile,collection.profile);});int choice=previewCollection(window,collection,existing==app.store.profiles.end()?nullptr:&*existing);if(choice!=AddCollection&&choice!=ReplaceCollection)return;
 bool replace=choice==ReplaceCollection;if(replace&&MessageBoxW(window,L"Replace all shortcuts and workflows for this application? The previous library save will be kept as a backup.",L"Quick Cheat",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)!=IDYES)return;
 Store candidate;CollectionImportCounts counts;if(!mergeCollection(app.store,collection,replace,candidate,counts,error)){app.error(error,window);return;}
 if(!leavePage())return;CollectionImageInstall images;if(!images.install(collection,app.file.parent_path(),error)){app.error(error,window);return;}if(!app.commit(std::move(candidate)))return;images.keep();
 draft=app.store;auto imported=std::find_if(draft.profiles.begin(),draft.profiles.end(),[&](const auto& profile){return sameProfile(profile,collection.profile);});profileIndex=static_cast<int>(imported-draft.profiles.begin());if(app.workflowEditor&&app.workflowEditor->window)ShowWindow(app.workflowEditor->window,SW_HIDE);refreshProfiles();updateWorkflowPage();layout();
 auto message=L"Imported "+std::to_wstring(counts.shortcuts)+L" shortcuts and "+std::to_wstring(counts.workflows)+L" workflows; skipped "+std::to_wstring(counts.duplicates)+L" exact duplicates.";SetWindowTextW(status,message.c_str());if(page==2&&app.workflowEditor)SetWindowTextW(app.workflowEditor->status,message.c_str());
}
}
