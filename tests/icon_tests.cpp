#include <windows.h>
#include <filesystem>
#include <iostream>
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;
 auto binary=std::filesystem::absolute(argv[1]);HMODULE module=LoadLibraryExW(binary.c_str(),nullptr,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
 if(!module){std::cerr<<"Cannot read executable resources\n";return 1;}
 bool valid=true;
 for(int size:{16,20,24,32,40,48,64,96,128,256}){
  auto icon=static_cast<HICON>(LoadImageW(module,MAKEINTRESOURCEW(1),IMAGE_ICON,size,size,0));ICONINFO info{};
  if(!icon||!GetIconInfo(icon,&info)){valid=false;if(icon)DestroyIcon(icon);break;}
  BITMAP bitmap{};if(!GetObjectW(info.hbmColor,sizeof(bitmap),&bitmap)||bitmap.bmWidth!=size||bitmap.bmHeight!=size)valid=false;
  if(info.hbmColor)DeleteObject(info.hbmColor);if(info.hbmMask)DeleteObject(info.hbmMask);DestroyIcon(icon);
 }
 FreeLibrary(module);if(!valid){std::cerr<<"Missing or invalid embedded icon sizes\n";return 1;}
 std::cout<<"Embedded icon loads at all 10 Windows sizes\n";return 0;
}
