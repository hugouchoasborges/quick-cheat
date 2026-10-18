#pragma once
#include "storage.hpp"
#include "theme.hpp"
#include <commctrl.h>
#include <memory>
namespace qc {
constexpr wchar_t version[]=L"0.2.0-alpha.7";
constexpr UINT ForegroundChanged=WM_APP+1, TrayMessage=WM_APP+2, ExecuteRow=WM_APP+3;
constexpr UINT CmdOpen=100,CmdManage=101,CmdData=102,CmdExit=103,CmdNew=104,CmdExecute=105,CmdShortcuts=106,CmdWorkflows=107,CmdManageWorkflows=108,CmdExpand=109,CmdPin=110,CmdPopupClose=111;
struct Target { HWND window{}; DWORD process{}; std::wstring executable,name; };
struct Editor;
struct WorkflowEditor;
struct ShortcutCategory {std::wstring name;bool collapsed{};};
struct App {
 HINSTANCE instance{};HWND host{},popup{},rows{},heading{},empty{},footer{},search{};HFONT font{},labelFont{},bold{};Theme theme;bool updatingPopup{};UINT popupDpi{};
 HWINEVENTHOOK foregroundHook{};Target target;Store store;std::filesystem::path file;
 HWND pressedWindow{};DWORD pressedProcess{};int pressedRow{-1};
 HWND workflowTree{},workflowDetail{},shortcutDetail{};bool workflowsVisible{},refreshingWorkflows{};
 SIZE popupSize{1040,760};bool popupExpanded{},popupSizing{};
 bool popupPinned{};int tileColumns{1},tileWidth{300};UINT tileDpi{};
 std::vector<ShortcutCategory> shortcutCategories;std::wstring categoryProfile;bool categorySearch{},categorySaveWarning{};
 std::unique_ptr<WorkflowEditor> workflowEditor;
 std::unique_ptr<Editor> editor;std::vector<size_t> visibleRows;bool trayAdded{};int hotkeyId{1};UINT taskbarCreated{};
 App();~App();
 bool start(HINSTANCE instance);
 void onForeground(HWND window);
 void togglePopup();void hidePopup(bool restore=true);void refreshPopup(bool clearSearch=false);void createPopup();void layoutPopup();void drawRow(DRAWITEMSTRUCT* item);void applyTheme();
 void openEditor(int page=1);void execute(size_t visibleIndex);void trayMenu();void addTray();
 void positionPopup(bool opening);void togglePopupSize();void rememberPopupSize();void layoutShortcutTiles();void updateShortcutDetail();void populateShortcutTiles();int selectedShortcut() const;void selectShortcut(int index);
 bool shortcutVisible(int index) const;void moveShortcutSelection(int direction);void syncCategoryStates();bool clickCategory(POINT point);void drawCategory(NMLVCUSTOMDRAW* draw);
 void createWorkflowPopup();void refreshWorkflowPopup();void layoutWorkflowPopup();void selectWorkflowStep();void styleWorkflowPopup();void openWorkflowEditor(const std::wstring& executable);
 bool commit(Store candidate);
 Profile* currentProfile();
 void error(const std::wstring& text,HWND owner=nullptr);
 static LRESULT CALLBACK hostProc(HWND,UINT,WPARAM,LPARAM);
 static LRESULT CALLBACK popupProc(HWND,UINT,WPARAM,LPARAM);
};
struct Editor {
 App& app;HWND window{},profiles{},list{},title{},category{},keys{},trigger{},status{},themeSelect{};
 HFONT font{},headingFont{},captionFont{};Store draft;int profileIndex{-1},shortcutIndex{-1},page{1};bool refreshing{},recording{},suppressChar{};
 explicit Editor(App& app);~Editor();
 void show();void layout();void refreshProfiles();void refreshList();void loadSelection();void clearFields();
 void addProfile();void saveShortcut();void deleteShortcut();void deleteProfile();void applyTrigger();void applyAppearance();void paint();void drawEntry(DRAWITEMSTRUCT* item);
 void populateCategories();bool persist();
 bool leavePage();bool selectPage(int next);bool selectProfile(int next);bool updateWorkflowPage();void openWorkflows(const std::wstring& key);void close();
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
 static LRESULT CALLBACK captureProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
};
Target identify(HWND window);
bool targetAlive(const Target& target);
bool sendChord(const Target& target,Chord chord,std::wstring& error);
std::wstring textOf(HWND control);
HWND control(HWND parent,const wchar_t* cls,const wchar_t* text,DWORD style,int id);
void bounds(HWND window,int x,int y,int width,int height,UINT dpi);
}
