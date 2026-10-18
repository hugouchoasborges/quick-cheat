#pragma once
#include "model.hpp"
#include <commctrl.h>
namespace qc {
struct App;
struct WorkflowEditor {
 App& app;HWND window{},flows{},title{},steps{},stepText{},status{};HFONT font{};UINT dpi{};
 std::wstring executable;Workflow draft;int flowIndex{-1},stepIndex{-1};bool loading{},dirty{},pendingStep{};
 explicit WorkflowEditor(App& value):app(value){};
 ~WorkflowEditor();
 bool show(const std::wstring& key,HWND parent);void layout();void refreshFlows();void refreshSteps();void loadFlow(int index);void selectStep(int index);void save();void remove();void moveStep(int delta);void close();void attachImage();
 bool discard();bool applyStep();
 static LRESULT CALLBACK proc(HWND,UINT,WPARAM,LPARAM);
};
}
