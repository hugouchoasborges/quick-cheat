#pragma once
#include <windows.h>
#include <string>
namespace qc {
struct AppOriginEvidence {
 bool complete{},addressPresent{},externalNavigation{},appToolbar{};
 unsigned headerCount{},externalBarCount{},tabs{};
 std::wstring header;
};
std::wstring installedAppStartHost(const std::wstring& description);
std::wstring manifestBrowserAppHost(const std::wstring& xml,const std::wstring& applicationId);
std::wstring registeredBrowserAppHost(HWND window);
bool hasPackagedAppIdentity(HWND window);
std::wstring verifiedAppOrigin(const AppOriginEvidence& evidence,const std::wstring& registeredHost);
}
