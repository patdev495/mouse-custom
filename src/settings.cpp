#include "app.h"
#include <commctrl.h>
#include <commdlg.h>
#include <algorithm>
#include <cwchar>

namespace mo {
HWND settingsWindow{};
HFONT settingsFont{};
UINT settingsDpi = 96;
bool uiLoading = false, dirty = false;
std::wstring statusMessage = L"Changes apply immediately.";

void status(const wchar_t* message) {
    statusMessage = message;
    if (settingsWindow) SetWindowTextW(GetDlgItem(settingsWindow, Status), message);
    meteorStatus(message);
}

bool saveConfig() {
    if (!dirty) return true;
    std::wstring text = L"\xFEFF[highlight]\r\n";
    auto add = [&](const wchar_t* key, const std::wstring& value) { text += key; text += L"=" + value + L"\r\n"; };
    add(L"style", config.ring ? L"ring" : L"filled");
    add(L"glow", config.glow ? L"1" : L"0");
    add(L"meteor", config.meteor ? L"1" : L"0");
    add(L"meteor_density", std::to_wstring(config.meteorDensity));
    add(L"meteor_size", std::to_wstring(config.meteorSize));
    add(L"meteor_speed", std::to_wstring(config.meteorSpeed));
    add(L"meteor_opacity", std::to_wstring(config.meteorOpacity));
    add(L"startup", config.startup ? L"1" : L"0");
    add(L"diameter", std::to_wstring(config.diameter));
    add(L"thickness", std::to_wstring(config.thickness));
    add(L"opacity", std::to_wstring(config.opacity));
    add(L"enabled", enabled ? L"1" : L"0");
    add(L"hotkey_modifiers", std::to_wstring(config.hotkeyModifiers));
    add(L"hotkey_key", std::to_wstring(config.hotkeyKey));
    wchar_t color[8]{}; swprintf_s(color, L"%06X", config.color); add(L"color", color);
    swprintf_s(color, L"%06X", config.meteorColor); add(L"meteor_color", color);
    const auto temporary = configPath + L".tmp";
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    bool success = false;
    if (file != INVALID_HANDLE_VALUE) {
        DWORD written{}, length = static_cast<DWORD>(text.size() * sizeof(wchar_t));
        success = WriteFile(file, text.data(), length, &written, nullptr) && written == length;
        if (success) success = FlushFileBuffers(file) != FALSE;
        CloseHandle(file);
        if (success) success = MoveFileExW(temporary.c_str(), configPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
    }
    if (success) { dirty = false; status(L"Saved. Changes apply immediately."); }
    else status(L"Cannot save. Move the portable folder to a writable location.");
    return success;
}

void scheduleSave() { dirty = true; SetTimer(owner, 2, 250, nullptr); }

void changed() {
    config.thickness = std::clamp(config.thickness, 1, config.diameter / 2 - 1);
    if (enabled) { createBitmap(); moveOverlay(true); }
    refreshSettings(); scheduleSave();
}

int px(int value) { return MulDiv(value, static_cast<int>(settingsDpi), 96); }

HWND control(const wchar_t* cls, const wchar_t* text, DWORD style,
             int x, int y, int width, int height, int id = 0) {
    HWND child = CreateWindowExW(wcscmp(cls, L"Edit") == 0 ? WS_EX_CLIENTEDGE : 0,
        cls, text, WS_CHILD | WS_VISIBLE | style, px(x), px(y), px(width), px(height),
        settingsWindow, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance, nullptr);
    SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(settingsFont), TRUE);
    return child;
}

void refreshSettings() {
    if (!settingsWindow) return;
    uiLoading = true;
    CheckDlgButton(settingsWindow, Enabled, enabled ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settingsWindow, Glow, config.glow ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settingsWindow, MeteorToggle, config.meteor ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settingsWindow, Startup, config.startup ? BST_CHECKED : BST_UNCHECKED);
    CheckRadioButton(settingsWindow, Filled, Ring, config.ring ? Ring : Filled);
    const int ids[] = {Diameter, Thickness, Opacity};
    const int values[] = {config.diameter, config.thickness, config.opacity};
    const int labels[] = {DiameterValue, ThicknessValue, OpacityValue};
    SendDlgItemMessageW(settingsWindow, Thickness, TBM_SETRANGEMAX, TRUE, config.diameter / 2 - 1);
    for (int i = 0; i < 3; ++i) {
        SendDlgItemMessageW(settingsWindow, ids[i], TBM_SETPOS, TRUE, values[i]);
        SetDlgItemInt(settingsWindow, labels[i], static_cast<UINT>(values[i]), FALSE);
    }
    EnableWindow(GetDlgItem(settingsWindow, Thickness), config.ring);
    wchar_t color[8]{}; swprintf_s(color, L"%06X", config.color);
    wchar_t current[16]{}; GetDlgItemTextW(settingsWindow, ColorEdit, current, 16);
    if (wcscmp(current, color)) SetDlgItemTextW(settingsWindow, ColorEdit, color);
    uiLoading = false;
}

void layoutSettings() {
    uiLoading = true;
    settingsDpi = GetDpiForWindow(settingsWindow);
    settingsFont = CreateFontW(-px(15), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    control(L"Static", L"Cursor effects", 0, 22, 16, 380, 26);
    control(L"Static", L"Changes apply instantly and are saved.", 0, 22, 46, 380, 22);
    control(L"Button", L"Effects enabled", BS_AUTOCHECKBOX | WS_TABSTOP, 22, 80, 360, 24, Enabled);
    control(L"Button", L"Filled", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 22, 118, 130, 24, Filled);
    control(L"Button", L"Ring", BS_AUTORADIOBUTTON | WS_TABSTOP, 170, 118, 130, 24, Ring);
    control(L"Static", L"Color (hex)", 0, 22, 162, 120, 24);
    control(L"Edit", L"", ES_UPPERCASE | ES_AUTOHSCROLL | WS_TABSTOP, 160, 158, 110, 28, ColorEdit);
    SendDlgItemMessageW(settingsWindow, ColorEdit, EM_SETLIMITTEXT, 6, 0);
    control(L"Button", L"Choose...", WS_TABSTOP, 284, 158, 110, 28, ColorPick);
    const wchar_t* names[] = {L"Size", L"Ring thickness", L"Opacity (%)"};
    const int ids[] = {Diameter, Thickness, Opacity};
    const int labels[] = {DiameterValue, ThicknessValue, OpacityValue};
    for (int i = 0; i < 3; ++i) {
        const int y = 208 + 62 * i;
        control(L"Static", names[i], 0, 22, y, 290, 22);
        HWND slider = control(TRACKBAR_CLASSW, names[i], TBS_HORZ | TBS_NOTICKS | WS_TABSTOP, 16, y + 24, 384, 28, ids[i]);
        control(L"Static", L"", SS_RIGHT, 332, y, 60, 22, labels[i]);
        SendMessageW(slider, TBM_SETRANGE, TRUE, MAKELPARAM(i == 0 ? 8 : i == 1 ? 1 : 0, i == 0 ? 256 : i == 1 ? 64 : 100));
    }
    control(L"Button", L"Glow", BS_AUTOCHECKBOX | WS_TABSTOP, 22, 402, 100, 24, Glow);
    control(L"Button", L"Meteor trail", BS_AUTOCHECKBOX | WS_TABSTOP, 116, 402, 152, 24, MeteorToggle);
    control(L"Button", L"Options...", WS_TABSTOP, 284, 398, 110, 30, MeteorOptions);
    control(L"Static", L"Global hotkey", 0, 22, 442, 126, 24);
    HWND hotkey = control(HOTKEY_CLASSW, L"", WS_TABSTOP, 160, 438, 144, 28, Hotkey);
    const BYTE modifiers = static_cast<BYTE>(((config.hotkeyModifiers & MOD_SHIFT) ? HOTKEYF_SHIFT : 0) |
        ((config.hotkeyModifiers & MOD_CONTROL) ? HOTKEYF_CONTROL : 0) | ((config.hotkeyModifiers & MOD_ALT) ? HOTKEYF_ALT : 0));
    SendMessageW(hotkey, HKM_SETHOTKEY, MAKEWORD(config.hotkeyKey, modifiers), 0);
    control(L"Button", L"Apply hotkey", WS_TABSTOP, 312, 438, 82, 28, ApplyHotkey);
    control(L"Button", L"Start when I sign in", BS_AUTOCHECKBOX | WS_TABSTOP, 22, 484, 372, 26, Startup);
    control(L"Static", L"", 0, 22, 524, 380, 42, Status);
    control(L"Button", L"Exit", WS_TABSTOP, 22, 578, 100, 30, Quit);
    control(L"Button", L"Close", WS_TABSTOP | BS_DEFPUSHBUTTON, 292, 578, 100, 30, Close);
    refreshSettings(); SetDlgItemTextW(settingsWindow, Status, statusMessage.c_str()); uiLoading = false;
}

LRESULT CALLBACK settingsProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: settingsWindow = hwnd; layoutSettings(); return 0;
    case WM_COMMAND: {
        const int id = LOWORD(wp), event = HIWORD(wp);
        if (uiLoading) return 0;
        if (id == Close || id == IDCANCEL) { SendMessageW(hwnd, WM_CLOSE, 0, 0); return 0; }
        if (id == Quit) { DestroyWindow(owner); return 0; }
        if (id == Enabled && event == BN_CLICKED) { setEnabled(!enabled); scheduleSave(); return 0; }
        if (id == Glow && event == BN_CLICKED) { config.glow = !config.glow; changed(); return 0; }
        if (id == MeteorToggle && event == BN_CLICKED) {
            config.meteor = !config.meteor; resetMeteors(); refreshSettings(); scheduleSave(); return 0;
        }
        if (id == MeteorOptions && event == BN_CLICKED) { showMeteorOptions(); return 0; }
        if (id == Startup && event == BN_CLICKED) {
            changeStartupPreference(!config.startup);
            refreshSettings(); return 0;
        }
        if (id == ApplyHotkey && event == BN_CLICKED) {
            const auto value = static_cast<WORD>(SendDlgItemMessageW(hwnd, Hotkey, HKM_GETHOTKEY, 0, 0));
            const BYTE flags = HIBYTE(value);
            const UINT modifiers = ((flags & HOTKEYF_SHIFT) ? MOD_SHIFT : 0) |
                ((flags & HOTKEYF_CONTROL) ? MOD_CONTROL : 0) | ((flags & HOTKEYF_ALT) ? MOD_ALT : 0);
            if (applyHotkey(modifiers, LOBYTE(value))) scheduleSave();
            return 0;
        }
        if ((id == Filled || id == Ring) && event == BN_CLICKED) { config.ring = id == Ring; changed(); return 0; }
        if (id == ColorEdit && event == EN_CHANGE) {
            wchar_t text[16]{}; GetDlgItemTextW(hwnd, ColorEdit, text, 16);
            unsigned color{};
            if (parseColor(text, color)) { config.color = color; changed(); }
            else status(L"Enter a six-digit hex color, for example FFD200.");
        }
        if (id == ColorPick && event == BN_CLICKED) {
            static COLORREF custom[16]{};
            CHOOSECOLORW chooser{sizeof(chooser)};
            chooser.hwndOwner = hwnd; chooser.lpCustColors = custom;
            chooser.rgbResult = RGB((config.color >> 16) & 255, (config.color >> 8) & 255, config.color & 255);
            chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColorW(&chooser)) {
                config.color = (GetRValue(chooser.rgbResult) << 16) | (GetGValue(chooser.rgbResult) << 8) | GetBValue(chooser.rgbResult);
                changed();
            }
        }
        return 0;
    }
    case WM_HSCROLL: {
        const int id = GetDlgCtrlID(reinterpret_cast<HWND>(lp));
        const auto value = static_cast<int>(SendMessageW(reinterpret_cast<HWND>(lp), TBM_GETPOS, 0, 0));
        if (id == Diameter) config.diameter = value;
        if (id == Thickness) config.thickness = value;
        if (id == Opacity) config.opacity = value;
        changed(); return 0;
    }
    case WM_CLOSE:
        saveConfig();
        if (meteorOptionsWindow) DestroyWindow(meteorOptionsWindow);
        DestroyWindow(hwnd); return 0;
    case WM_DPICHANGED: {
        const RECT suggested = *reinterpret_cast<RECT*>(lp);
        while (HWND child = GetWindow(hwnd, GW_CHILD)) DestroyWindow(child);
        DeleteObject(settingsFont); settingsFont = nullptr;
        layoutSettings();
        SetWindowPos(hwnd, nullptr, suggested.left, suggested.top,
            suggested.right - suggested.left, suggested.bottom - suggested.top, SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_DESTROY: settingsWindow = nullptr; DeleteObject(settingsFont); settingsFont = nullptr; return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void registerSettingsClass() {
    WNDCLASSW cls{};
    cls.hInstance = instance; cls.lpszClassName = L"MouseOptionsSettings";
    cls.lpfnWndProc = settingsProc; cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    RegisterClassW(&cls);
    registerMeteorOptionsClass();
}

void showSettings() {
    if (!settingsWindow) {
        INITCOMMONCONTROLSEX common{sizeof(common), ICC_BAR_CLASSES | ICC_HOTKEY_CLASS};
        InitCommonControlsEx(&common);
        POINT point{}; GetCursorPos(&point);
        MONITORINFO monitor{sizeof(monitor)};
        GetMonitorInfoW(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &monitor);
        const int x = monitor.rcWork.left + 40, y = monitor.rcWork.top + 24;
        settingsWindow = CreateWindowExW(0, L"MouseOptionsSettings", L"Mouse Options Settings",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            x, y, 440, 550, nullptr, nullptr, instance, nullptr);
        RECT rect{0, 0, px(420), px(628)};
        AdjustWindowRectExForDpi(&rect, static_cast<DWORD>(GetWindowLongPtrW(settingsWindow, GWL_STYLE)), FALSE, 0, settingsDpi);
        SetWindowPos(settingsWindow, nullptr, x, y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
    }
    ShowWindow(settingsWindow, SW_SHOWNORMAL); SetForegroundWindow(settingsWindow);
}
}
