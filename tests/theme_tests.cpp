#include "model.hpp"
#include <iostream>
int main(){
 using namespace qc;Store s;Store loaded;
 s.theme=ThemeMode::Dark;
 if(!deserialize(serialize(s),loaded)||loaded.theme!=ThemeMode::Dark)return 1;
 s.theme=ThemeMode::Light;if(!deserialize(serialize(s),loaded)||loaded.theme!=ThemeMode::Light)return 2;
 if(!deserialize(L"QC\t1\nH\t3\t32\nZ\n",loaded)||loaded.theme!=ThemeMode::System)return 3;
 if(deserialize(L"QC\t2\nH\t3\t32\nT\t9\nZ\n",loaded))return 4;
 Shortcut shortcut{L"Save document",L"File",{MOD_CONTROL|MOD_SHIFT,'S'}};
 if(!matchesShortcut(shortcut,L"save file"))return 5;
 if(!matchesShortcut(shortcut,L"CTRL shift"))return 6;
 if(matchesShortcut(shortcut,L"save mesh"))return 7;
 if(!matchesShortcut(shortcut,L"  "))return 8;
 std::cout<<"Theme/search tests passed: v1 migration, v2 themes, invalid theme, multi-term search\n";return 0;
}
