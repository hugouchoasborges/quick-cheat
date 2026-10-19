#pragma once
#include "model.hpp"
#include <commctrl.h>
#include <memory>
namespace qc {
constexpr UINT ThemeChanged=WM_APP+10;
struct Palette { COLORREF background{},panel{},field{},selection{},border{},text{},muted{},accent{},accentText{},key{}; };
struct SystemThemeObserver;
struct Theme {
 Palette colors{};bool dark{},highContrast{};HBRUSH background{},field{},panel{};
 std::unique_ptr<SystemThemeObserver> observer;
 Theme();~Theme();
 void observe(HWND host);void stop();void resolve(ThemeMode mode);void style(HWND window);
 LRESULT controlColor(UINT message,HDC dc,HWND control);
 bool drawControl(DRAWITEMSTRUCT* item);
 LRESULT drawCheckbox(NMCUSTOMDRAW* draw);
 static LRESULT CALLBACK widgetProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
 static LRESULT CALLBACK headerProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
};
void rounded(HDC dc,RECT rect,COLORREF fill,COLORREF border,int radius);
void paintText(HDC dc,const std::wstring& text,RECT rect,COLORREF color,UINT flags=DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
}
