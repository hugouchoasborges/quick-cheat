#pragma once
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include "model.hpp"
namespace qc {
struct App;
class Dock {
 App& app;HWND window{},tips{};HFONT font{};std::vector<HWND> buttons;std::wstring key,renderKey;
 bool saving{},dragging{};POINT dragStart{};RECT dragRect{};HWND targetWindow{};DWORD targetProcess{};UINT dpi{96};size_t page{},pageCount{1};
 void rebuild(const Profile& profile);void savePosition();void toggleCollapsed();void execute(size_t index);
 static LRESULT CALLBACK buttonProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
 public:
 explicit Dock(App& app);~Dock();void refresh();void themeChanged();
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
};
class DockSettings {
 App& app;HWND window{},parent{},enabled{},orientation{},collapsed{},scale{},opacity{},list{},status{};HFONT font{},headingFont{},captionFont{};std::wstring key;std::vector<size_t> order;bool loading{},changed{};
 void populate();void apply();void move(int direction);void mark();void updateActions();
 public:
 DockSettings(App& app,HWND parent);~DockSettings();bool show(const std::wstring& key);void hide();void layout(int x,int y,int width,int height,UINT dpi);bool discard();bool pending()const{return changed;}void save(){apply();}void themeChanged();
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
};
}
