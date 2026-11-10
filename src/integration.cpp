#include "app.h"
#include <vector>

namespace mo {
int hotkeyId{};

bool applyHotkey(UINT modifiers, UINT key) {
    if (!(modifiers & (MOD_CONTROL | MOD_ALT | MOD_SHIFT)) || (modifiers & ~7U) ||
        !key || key > 255 || key == VK_CONTROL || key == VK_SHIFT || key == VK_MENU || key == VK_LWIN || key == VK_RWIN) {
        status(L"Choose Ctrl, Alt or Shift plus a letter, number or function key.");
        return false;
    }
    if (hotkeyId && modifiers == config.hotkeyModifiers && key == config.hotkeyKey) {
        status(L"Hotkey unchanged."); return true;
    }
    const int next = hotkeyId == 1 ? 2 : 1;
    if (!RegisterHotKey(owner, next, modifiers | MOD_NOREPEAT, key)) {
        status(L"Hotkey unavailable. Choose another; the previous hotkey is kept.");
        return false;
    }
    if (hotkeyId) UnregisterHotKey(owner, hotkeyId);
    hotkeyId = next;
    config.hotkeyModifiers = modifiers; config.hotkeyKey = key;
    status(L"Hotkey registered.");
    return true;
}

void shutdownHotkey() { if (hotkeyId) { UnregisterHotKey(owner, hotkeyId); hotkeyId = 0; } }

bool setStartup(bool value) {
    constexpr wchar_t runPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    HKEY key{};
    LSTATUS result = value
        ? RegCreateKeyExW(HKEY_CURRENT_USER, runPath, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr)
        : RegOpenKeyExW(HKEY_CURRENT_USER, runPath, 0, KEY_SET_VALUE, &key);
    if (!value && result == ERROR_FILE_NOT_FOUND) { config.startup = false; return true; }
    if (result == ERROR_SUCCESS) {
        if (value) {
            wchar_t executable[32768]{};
            if (!GetModuleFileNameW(nullptr, executable, 32768)) result = GetLastError();
            else {
                const std::wstring command = L"\"" + std::wstring(executable) + L"\" --background";
                result = RegSetValueExW(key, L"MouseOptions", 0, REG_SZ,
                    reinterpret_cast<const BYTE*>(command.c_str()), static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
            }
        } else {
            result = RegDeleteValueW(key, L"MouseOptions");
            if (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
        }
        RegCloseKey(key);
    }
    if (result != ERROR_SUCCESS) {
        status(L"Cannot change Windows startup registration. Setting was not changed.");
        return false;
    }
    config.startup = value;
    status(value ? L"Will start in the tray when you sign in." : L"Automatic startup is off.");
    return true;
}

bool changeStartupPreference(bool value) {
    constexpr wchar_t path[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    HKEY key{};
    DWORD type{}, length{};
    bool existed = false;
    std::vector<BYTE> previous;
    LSTATUS result = RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_QUERY_VALUE, &key);
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(key, L"MouseOptions", nullptr, &type, nullptr, &length);
        if (result == ERROR_SUCCESS && length <= 65536) {
            previous.resize(length);
            result = RegQueryValueExW(key, L"MouseOptions", nullptr, &type, previous.data(), &length);
            existed = result == ERROR_SUCCESS;
        } else if (result == ERROR_SUCCESS) result = ERROR_MORE_DATA;
        RegCloseKey(key);
    }
    if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND) {
        status(L"Cannot read Windows startup registration. Setting was not changed."); return false;
    }
    const bool preference = config.startup;
    if (!setStartup(value)) return false;
    scheduleSave();
    const bool saved = saveConfig();
    KillTimer(owner, 2);
    if (saved) return true;
    // A portable folder may be read-only: restore the exact pre-existing Run value.
    result = RegCreateKeyExW(HKEY_CURRENT_USER, path, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr);
    if (result == ERROR_SUCCESS) {
        result = existed ? RegSetValueExW(key, L"MouseOptions", 0, type, previous.data(), length)
                         : RegDeleteValueW(key, L"MouseOptions");
        if (!existed && result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
        RegCloseKey(key);
    }
    if (result == ERROR_SUCCESS) {
        config.startup = preference;
        status(L"Cannot save. Windows startup change was reverted. Use a writable folder.");
    } else status(L"Cannot save; Windows startup rollback also failed. Check Startup Apps.");
    return false;
}
}
