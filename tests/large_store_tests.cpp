#include "storage.hpp"
#include <iostream>
int main() {
 using namespace qc;
 Store store;
 for(int i=0;i<8;++i) {
  Profile p{L"app"+std::to_wstring(i)+L".exe",L"Large profile",{}};
  p.shortcuts.assign(2000,Shortcut{std::wstring(240,L't'),std::wstring(80,L'g'),{MOD_CONTROL,'S'}});
  store.profiles.push_back(std::move(p));
 }
 const auto dir=std::filesystem::temp_directory_path()/(L"QuickCheat-large-"+std::to_wstring(GetCurrentProcessId()));
 const auto file=dir/L"shortcuts.qc";
 Store loaded;std::wstring error;
 bool passed=validStore(store)&&serialize(store).size()>4*1024*1024&&saveStore(file,store,error)&&loadStore(file,loaded,error);
 passed=passed&&loaded.profiles.size()==8&&loaded.profiles[7].shortcuts.size()==2000;
 if(passed) {
  for(auto& p:store.profiles)for(auto& s:p.shortcuts){s.title.assign(240,L'語');s.category.assign(80,L'語');}
  // A rejected oversized UTF-8 save must leave the previous readable file intact.
  passed=validStore(store)&&!saveStore(file,store,error)&&loadStore(file,loaded,error)&&loaded.profiles[0].shortcuts[0].title==std::wstring(240,L't');
 }
 std::error_code ec;std::filesystem::remove_all(dir,ec);
 if(!passed){std::wcerr<<L"Large-store roundtrip or oversized-save protection failed: "<<error<<L'\n';return 1;}
 std::cout<<"Large-store tests passed: >4 Mi character roundtrip and oversized UTF-8 save preserves prior data\n";return 0;
}
