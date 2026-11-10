#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cwchar>
#include "app.h"

namespace mo {
constexpr wchar_t OverlayName[] = L"Mouse Options Highlight";
constexpr UINT TrayEvent = WM_APP + 1;
HINSTANCE instance;
HWND owner{}, overlay{};
HDC bitmapDC{};
HBITMAP bitmap{};
HGDIOBJ oldBitmap{};
bool enabled = true;
POINT lastCursor{LONG_MIN, LONG_MIN};
int imageSize = 50;
UINT renderDpi = 96;
HMONITOR activeMonitor{};
HWND lastForeground{};
UINT taskbarCreated{};
Settings config;
std::wstring configPath;

bool parseColor(const wchar_t* text, unsigned& value) {
    if (wcslen(text) != 6) return false;
    for (int i = 0; i < 6; ++i) if (!wcschr(L"0123456789abcdefABCDEF", text[i])) return false;
    value = wcstoul(text, nullptr, 16);
    return true;
}

std::wstring readSetting(const wchar_t* key, const wchar_t* fallback) {
    wchar_t buffer[128]{};
    GetPrivateProfileStringW(L"highlight", key, fallback, buffer, 128, configPath.c_str());
    return buffer;
}

int readNumber(const wchar_t* key, int fallback, int minimum, int maximum) {
    const auto value = readSetting(key, L"");
    wchar_t* end{};
    const long number = wcstol(value.c_str(), &end, 10);
    if (value.empty() || *end) return fallback;
    return static_cast<int>(std::clamp(number, static_cast<long>(minimum), static_cast<long>(maximum)));
}

void loadConfig() {
    wchar_t exePath[32768]{};
    GetModuleFileNameW(nullptr, exePath, 32768);
    configPath = exePath;
    configPath.resize(configPath.find_last_of(L"\\/") + 1);
    configPath += L"mouse-options.ini";
    config.ring = readSetting(L"style", L"filled") == L"ring";
    config.glow = readNumber(L"glow", 0, 0, 1) != 0;
    config.meteor = readNumber(L"meteor", 0, 0, 1) != 0;
    config.meteorDensity = readNumber(L"meteor_density", 55, 5, 100);
    config.meteorSize = readNumber(L"meteor_size", 4, 2, 8);
    config.meteorSpeed = readNumber(L"meteor_speed", 110, 40, 240);
    config.meteorOpacity = readNumber(L"meteor_opacity", 85, 0, 100);
    parseColor(readSetting(L"meteor_color", L"72CFFF").c_str(), config.meteorColor);
    config.startup = readNumber(L"startup", 0, 0, 1) != 0;
    config.diameter = readNumber(L"diameter", 48, 8, 256);
    config.thickness = readNumber(L"thickness", 4, 1, config.diameter / 2 - 1);
    config.opacity = readNumber(L"opacity", 25, 0, 100);
    config.hotkeyModifiers = static_cast<UINT>(readNumber(L"hotkey_modifiers", 3, 1, 7));
    config.hotkeyKey = static_cast<UINT>(readNumber(L"hotkey_key", 'H', 1, 255));
    const auto color = readSetting(L"color", L"FFD200");
    parseColor(color.c_str(), config.color);
    enabled = readNumber(L"enabled", 1, 0, 1) != 0;
}

void freeBitmap() {
    if (bitmapDC) {
        SelectObject(bitmapDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(bitmapDC);
        bitmapDC = nullptr;
        bitmap = nullptr;
    }
}

void createBitmap() {
    freeBitmap();
    const int diameterPixels = MulDiv(config.diameter, static_cast<int>(renderDpi), 96);
    const int spread = config.glow ? MulDiv(12, static_cast<int>(renderDpi), 96) : 0;
    imageSize = diameterPixels + 2 + 2 * spread;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = imageSize;
    info.bmiHeader.biHeight = -imageSize;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits{};
    bitmapDC = CreateCompatibleDC(nullptr);
    bitmap = CreateDIBSection(bitmapDC, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmapDC || !bitmap || !bits) {
        MessageBoxW(nullptr, L"Cannot allocate highlight bitmap.", AppName, MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }
    oldBitmap = SelectObject(bitmapDC, bitmap);
    auto pixels = static_cast<std::uint32_t*>(bits);
    const float center = imageSize / 2.0f;
    for (int y = 0; y < imageSize; ++y) for (int x = 0; x < imageSize; ++x) {
        const float dx = x + .5f - center, dy = y + .5f - center;
        const float distance = std::sqrt(dx * dx + dy * dy);
        const float radius = diameterPixels / 2.f;
        float edge = std::clamp(radius - distance + .5f, 0.f, 1.f);
        const float thickness = config.thickness * renderDpi / 96.f;
        if (config.ring) edge *= std::clamp(distance - (radius - thickness) + .5f, 0.f, 1.f);
        if (config.glow && distance >= radius) {
            const float normalized = (distance - radius) / (4.f * renderDpi / 96.f);
            edge = std::max(edge, .32f * std::exp(-.5f * normalized * normalized));
        }
        const auto alpha = static_cast<unsigned>(edge * config.opacity * 2.55f);
        const unsigned red = ((config.color >> 16) & 255) * alpha / 255;
        const unsigned green = ((config.color >> 8) & 255) * alpha / 255;
        const unsigned blue = (config.color & 255) * alpha / 255;
        pixels[y * imageSize + x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
    }
}

void moveOverlay(bool upload) {
    if (!enabled || !bitmapDC) return;
    POINT cursor{};
    GetCursorPos(&cursor);
    const HMONITOR monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
    if (monitor != activeMonitor) {
        activeMonitor = monitor;
        // A hidden one-pixel anchor obtains the DPI at the pointer, even near edges.
        SetWindowPos(owner, nullptr, cursor.x, cursor.y, 1, 1, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    const UINT dpi = GetDpiForWindow(owner);
    if (dpi && dpi != renderDpi) { renderDpi = dpi; createBitmap(); upload = true; }
    const HWND foreground = GetForegroundWindow();
    const bool foregroundChanged = foreground != lastForeground;
    lastForeground = foreground;
    if (!upload && !foregroundChanged && cursor.x == lastCursor.x && cursor.y == lastCursor.y) return;
    lastCursor = cursor;
    POINT location{cursor.x - imageSize / 2, cursor.y - imageSize / 2};
    if (upload || !IsWindowVisible(overlay)) {
        SIZE size{imageSize, imageSize};
        POINT source{};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(overlay, nullptr, &location, &size, bitmapDC, &source, 0, &blend, ULW_ALPHA);
        ShowWindow(overlay, SW_SHOWNOACTIVATE);
    }
    SetWindowPos(overlay, HWND_TOPMOST, location.x, location.y, 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void setEnabled(bool value) {
    enabled = value;
    KillTimer(owner, 1);
    if (enabled) {
        if (!bitmapDC) createBitmap();
        SetTimer(owner, 1, 16, nullptr);
        moveOverlay(true);
    } else { ShowWindow(overlay, SW_HIDE); freeBitmap(); resetMeteors(); }
    refreshSettings();
}

void tray(bool add) {
    NOTIFYICONDATAW data{sizeof(data)};
    data.hWnd = owner;
    data.uID = 1;
    data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data.uCallbackMessage = TrayEvent;
    data.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy_s(data.szTip, L"Mouse Options - cursor effects");
    Shell_NotifyIconW(add ? NIM_ADD : NIM_DELETE, &data);
    if (add) { data.uVersion = NOTIFYICON_VERSION_4; Shell_NotifyIconW(NIM_SETVERSION, &data); }
}

void trayMenu() {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, OpenSettings, L"Settings...");
    AppendMenuW(menu, MF_STRING | (enabled ? MF_CHECKED : 0), Toggle, L"Effects enabled");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, Quit, L"Exit");
    POINT point{}; GetCursorPos(&point);
    SetForegroundWindow(owner);
    const UINT command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                        point.x, point.y, 0, owner, nullptr);
    DestroyMenu(menu);
    if (command) PostMessageW(owner, WM_COMMAND, command, 0);
    PostMessageW(owner, WM_NULL, 0, 0);
}

LRESULT CALLBACK ownerProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == taskbarCreated) { tray(true); return 0; }
    switch (msg) {
    case WM_TIMER:
        if (wp == 1) { moveOverlay(); tickMeteors(); }
        if (wp == 2) { KillTimer(owner, 2); saveConfig(); }
        return 0;
    case TrayEvent:
        if (LOWORD(lp) == WM_CONTEXTMENU) trayMenu();
        if (LOWORD(lp) == NIN_SELECT || LOWORD(lp) == NIN_KEYSELECT) showSettings();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == OpenSettings) showSettings();
        if (LOWORD(wp) == Toggle) { setEnabled(!enabled); scheduleSave(); }
        if (LOWORD(wp) == Quit) DestroyWindow(hwnd);
        return 0;
    case WM_HOTKEY: setEnabled(!enabled); scheduleSave(); return 0;
    case WM_DISPLAYCHANGE: case WM_SETTINGCHANGE:
        activeMonitor = nullptr; moveOverlay(true); return 0;
    case WM_DESTROY:
        resetMeteors();
        shutdownHotkey();
        saveConfig();
        if (settingsWindow) DestroyWindow(settingsWindow);
        tray(false);
        KillTimer(owner, 1);
        DestroyWindow(overlay);
        freeBitmap();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
}

int WINAPI wWinMain(HINSTANCE current, HINSTANCE, PWSTR commandLine, int) {
    using namespace mo;
    int argumentCount{};
    LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    bool openSettings = false, background = false;
    for (int i = 1; arguments && i < argumentCount; ++i) {
        if (!wcscmp(arguments[i], L"--settings")) openSettings = true;
        if (!wcscmp(arguments[i], L"--background")) background = true;
    }
    if (arguments) LocalFree(arguments);
    (void)commandLine;
    HANDLE singleton = CreateMutexW(nullptr, FALSE, L"Local\\MouseOptions.CursorHighlight");
    if (!singleton) return 1;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        for (int attempt = 0; attempt < 80; ++attempt) {
            if (HWND existing = FindWindowW(L"MouseOptionsOwner", AppName)) {
                if (openSettings || !background) {
                    DWORD pid{}; GetWindowThreadProcessId(existing, &pid);
                    AllowSetForegroundWindow(pid);
                    PostMessageW(existing, WM_COMMAND, OpenSettings, 0);
                }
                CloseHandle(singleton); return 0;
            }
            Sleep(25);
        }
        CloseHandle(singleton); return 1;
    }
    instance = current;
    loadConfig();
    registerSettingsClass();
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    WNDCLASSW cls{};
    cls.hInstance = instance;
    cls.lpszClassName = L"MouseOptionsOwner";
    cls.lpfnWndProc = ownerProc;
    RegisterClassW(&cls);
    owner = CreateWindowExW(WS_EX_TOOLWINDOW, cls.lpszClassName, AppName,
                            WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    cls.lpszClassName = L"MouseOptionsOverlay";
    cls.lpfnWndProc = DefWindowProcW;
    RegisterClassW(&cls);
    overlay = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                              cls.lpszClassName, OverlayName, WS_POPUP, 0, 0, imageSize, imageSize,
                              nullptr, nullptr, instance, nullptr);
    if (!owner || !overlay) return 1;
    createBitmap();
    tray(true);
    applyHotkey(config.hotkeyModifiers, config.hotkeyKey);
    if (config.startup) setStartup(true);
    setEnabled(enabled);
    if (openSettings) showSettings();
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (meteorOptionsWindow && IsDialogMessageW(meteorOptionsWindow, &message)) continue;
        if (settingsWindow && IsDialogMessageW(settingsWindow, &message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    CloseHandle(singleton);
    return static_cast<int>(message.wParam);
}
