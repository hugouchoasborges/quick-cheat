#pragma once
namespace qc::settings {
enum {Profiles=200,Browse,Entries,Title,Category,Keys,Record,New,Save,Delete,RemoveProfile,Trigger,Apply,Status,ThemeSelect};
enum {TabGeneral=220,TabShortcuts,TabWorkflows,OpenData,ImportCollection,ExportCollection,Website,TabDock};
inline int tabId(int page){return page==3?TabDock:TabGeneral+page;}
enum {LabelApp=300,LabelTitle,LabelCategory,LabelKeys,LabelTrigger,LabelTheme,PageTitle,PageSubtitle,ThemeHelp,TriggerHelp,DataLabel,DataPath,DataHelp,FormTitle,EmptyFlows,BrowserHelp};
}
