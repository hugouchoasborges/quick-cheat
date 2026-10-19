#pragma once
#include "app.hpp"
#include "community/catalog.hpp"
#include <thread>
namespace qc {
class CommunityWindow {
 App& app;HWND window{},search{},current{},rows{},details{},status{},searchButton{},next{},install{},cancelButton{};
 HFONT font{};community::Page page,pendingPage;community::Query query,shownQuery;community::PreparedCollection prepared;
 std::thread worker;std::atomic_bool canceled{};bool busy{},download{},succeeded{};UINT_PTR generation{};std::wstring failure;
 void layout();void updateControls();void selection();void request(bool more=false);void preview();void complete(UINT_PTR);
 void start(bool downloading);void cancel();void close();
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
public:
 explicit CommunityWindow(App& value):app(value){}
 ~CommunityWindow();
 void show();void themeChanged();bool dialogMessage(MSG& message);
};
}
