#pragma once
#include "storage.hpp"
#include "theme.hpp"
#include <commctrl.h>
#include <memory>
namespace qc {
constexpr wchar_t version[]=L"0.6.0-alpha.4";
constexpr UINT ForegroundChanged=WM_APP+1, TrayMessage=WM_APP+2, ExecuteRow=WM_APP+3,BrowserChanged=WM_APP+4,BrowserDirty=WM_APP+5;
constexpr int PopupContentTop=50,PopupTabsHeight=34,PopupFooterHeight=38;
constexpr UINT CmdOpen=100,CmdManage=101,CmdData=102,CmdExit=103,CmdNew=104,CmdExecute=105,CmdShortcuts=106,CmdWorkflows=107,CmdManageWorkflows=108,CmdExpand=109,CmdPin=110,CmdPopupClose=111,CmdBack=112,CmdCommunity=113;
struct Target { HWND window{}; DWORD process{}; std::wstring executable,name,website; };
struct Editor;
struct Collection;class CommunityWindow;
struct WorkflowEditor;
class Dock;class DockSettings;class NativeBrowserContext;
struct ShortcutCategory {std::wstring name;bool collapsed{};};
struct App {
 HINSTANCE instance{};HWND host{},popup{},rows{},heading{},empty{},footer{},search{};HFONT font{},labelFont{},bold{};Theme theme;bool updatingPopup{};UINT popupDpi{};
 HWINEVENTHOOK foregroundHook{},browserNameHook{},browserValueHook{},locationHook{};Target target;Store store;std::filesystem::path file;
 std::unique_ptr<NativeBrowserContext> browser;std::unique_ptr<Dock> dock;
 std::unique_ptr<CommunityWindow> community;
 Profile contextProfile;bool contextDirty{true},hasContextProfile{};size_t browserShortcutCount{},browserWorkflowCount{};
 bool pendingExecution{},pendingPopupOpen{},browserTimerPending{};Shortcut pendingShortcut;Target pendingTarget;unsigned long long pendingGeneration{},openingGeneration{};
 HWND pressedWindow{};DWORD pressedProcess{};int pressedRow{-1};
 HWND workflowTree{},workflowDetail{},shortcutDetail{};bool workflowsVisible{},refreshingWorkflows{};
 SIZE popupSize{1040,760};bool popupExpanded{},popupSizing{};
 bool popupPinned{},settingsVisible{};int tileColumns{1},tileWidth{300};UINT tileDpi{};
 std::vector<ShortcutCategory> shortcutCategories;std::wstring categoryProfile;bool categorySearch{},categorySaveWarning{};
 std::unique_ptr<WorkflowEditor> workflowEditor;
 std::unique_ptr<Editor> editor;std::vector<size_t> visibleRows;bool trayAdded{};int hotkeyId{1};UINT taskbarCreated{};
 App();~App();
 bool start(HINSTANCE instance);
 void onForeground(HWND window);void requestWebsite();void receiveWebsite();void runShortcut(const Shortcut& shortcut,const std::wstring& expectedWebsite={});void showPopupNow();
 void togglePopup();void hidePopup(bool restore=true);void refreshPopup(bool clearSearch=false);void createPopup();void layoutPopup();void drawRow(DRAWITEMSTRUCT* item);void applyTheme();
 void openEditor(int page=1);void closeSettings();void execute(size_t visibleIndex);void trayMenu();void addTray();
 void positionPopup(bool opening);void togglePopupSize();void rememberPopupSize();void layoutShortcutTiles();void updateShortcutDetail();void populateShortcutTiles();int selectedShortcut() const;void selectShortcut(int index);
 bool shortcutVisible(int index) const;void moveShortcutSelection(int direction);void syncCategoryStates();bool clickCategory(POINT point);void drawCategory(NMLVCUSTOMDRAW* draw);
 void createWorkflowPopup();void refreshWorkflowPopup();void layoutWorkflowPopup();void selectWorkflowStep();void styleWorkflowPopup();void openWorkflowEditor(const std::wstring& executable);
 bool commit(Store candidate);
 Profile* currentProfile();std::wstring contextName();
 void error(const std::wstring& text,HWND owner=nullptr);
 static LRESULT CALLBACK hostProc(HWND,UINT,WPARAM,LPARAM);
 static LRESULT CALLBACK popupProc(HWND,UINT,WPARAM,LPARAM);
};
struct Editor {
 App& app;HWND window{},profiles{},list{},title{},category{},keys{},trigger{},status{},themeSelect{};
 HFONT font{},headingFont{},captionFont{};Store draft;int profileIndex{-1},shortcutIndex{-1},page{1},scrollX{},scrollY{};bool refreshing{},recording{},suppressChar{};
 explicit Editor(App& app);~Editor();
 std::unique_ptr<DockSettings> dockSettings;
 void show();void layout();void scroll(UINT message,WPARAM value);void revealFocus();void refreshProfiles();void refreshList();void loadSelection();void clearFields();
 void addProfile();void addWebsite();void saveShortcut();void deleteShortcut();void deleteProfile();void applyTrigger();void applyAppearance();void paint();void drawEntry(DRAWITEMSTRUCT* item);
 void populateCategories();bool persist();
 void updateShortcutAction(bool clear=false);
 bool leavePage();bool selectPage(int next);bool selectProfile(int next);bool updateWorkflowPage();void openWorkflows(const std::wstring& key);void close();
 bool pendingPageEdits() const;void importApplicationCollection();void exportApplicationCollection();void updateCollectionActions();
 bool installCollection(const Collection& collection);
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
 static LRESULT CALLBACK captureProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
};
Target identify(HWND window);
bool targetAlive(const Target& target);
bool sendChord(const Target& target,Chord chord,std::wstring& error);
bool sendWebsiteUrl(const Target& target,const std::wstring& url,std::wstring& error);
std::wstring textOf(HWND control);
HWND control(HWND parent,const wchar_t* cls,const wchar_t* text,DWORD style,int id);
void bounds(HWND window,int x,int y,int width,int height,UINT dpi);
}
