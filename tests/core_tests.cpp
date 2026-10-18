#include "model.hpp"
#include "storage.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
int main() {
  try {
    using namespace qc;
    auto key = parseChord(L"Ctrl+Shift+S");
    CHECK(key && key->key == 'S' && key->modifiers == (MOD_CONTROL | MOD_SHIFT));
    CHECK(formatChord(*key) == L"Ctrl+Shift+S");
    CHECK(parseChord(L"Win+F11"));
    CHECK(parseChord(L"Alt+Enter"));
    CHECK(parseChord(L"Ctrl+PageDown"));
    CHECK(!parseChord(L"Ctrl+Ctrl+S"));
    CHECK(!parseChord(L"Ctrl+NotAKey"));
    CHECK(!parseChord(L"Ctrl+"));
    CHECK(!parseChord(L"A+B"));
    CHECK(!parseChord(L"F25"));
    CHECK(appKey(L"C:\\Program Files\\Spine\\Spine.EXE") == L"spine.exe");
    Store store;
    store.profiles.push_back({L"spine.exe", L"Spine", {{L"Mesh\tshape\n", L"Création", *key}}});
    auto encoded = serialize(store);
    Store decoded;
    CHECK(deserialize(encoded, decoded));
    CHECK(decoded.profiles.size() == 1);
    CHECK(decoded.profiles[0].shortcuts[0].title == L"Mesh\tshape\n");
    CHECK(decoded.profiles[0].shortcuts[0].category == L"Création");
    CHECK(decoded.profiles[0].shortcuts[0].chord.key == 'S');
    CHECK(!deserialize(L"QC\t99\n", decoded));
    CHECK(!deserialize(encoded.substr(0, encoded.size()-3), decoded));
    CHECK(!deserialize(encoded + L"garbage\n", decoded));
    CHECK(decoded.profiles.size() == 1); // Failure never replaces good data.
    const auto dir = std::filesystem::temp_directory_path() / (L"QuickCheat-test-" + std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(dir);
    const auto file = dir / L"shortcuts.qc";
    std::wstring error;
    CHECK(saveStore(file, store, error));
    Store loaded;
    CHECK(loadStore(file, loaded, error));
    CHECK(loaded.profiles[0].shortcuts[0].category == L"Création");
    store.profiles[0].name = L"Updated";
    CHECK(saveStore(file, store, error));
    CHECK(std::filesystem::exists(file.wstring() + L".bak"));
    CHECK(loadStore(file, loaded, error));
    CHECK(loaded.profiles[0].name == L"Updated");
    CHECK(loadStore(file.wstring() + L".bak", loaded, error));
    CHECK(loaded.profiles[0].name == L"Spine");
    std::filesystem::remove_all(dir);
    std::cout << "Core tests passed: chords, identity, Unicode, schema, corruption, atomic save and backup\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
