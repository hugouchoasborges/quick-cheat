#pragma once
#include <windows.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
namespace qc {
struct BrowserResult { HWND window{};DWORD process{};std::wstring website;std::uint64_t generation{}; };
// All methods except the worker internals are called by the application's UI thread.
class NativeBrowserContext {
 struct State;
 std::shared_ptr<State> state;
public:
 NativeBrowserContext(HWND notifyHost,UINT resultMessage);
 ~NativeBrowserContext();
 NativeBrowserContext(const NativeBrowserContext&)=delete;
 NativeBrowserContext& operator=(const NativeBrowserContext&)=delete;
 std::uint64_t request(HWND window,DWORD process,const std::wstring& executable);
 void clear();
 std::optional<BrowserResult> take();
 static bool supported(const std::wstring& executable);
};
}
