#include "app.h"
#include <commctrl.h>
#include <commdlg.h>
#include <algorithm>
#include <cwchar>

namespace mo {
HWND meteorOptionsWindow{};
namespace {
enum : int { Color = 300, Pick, Density, Size, Speed, Alpha, DensityValue, SizeValue, SpeedValue, AlphaValue, Info, Done };
HFONT font{};
UINT dpi = 96;
bool loading = false;
constexpr int SliderIds[] = {Density, Size, Speed, Alpha};
constexpr int LabelIds[] = {DensityValue, SizeValue, SpeedValue, AlphaValue};
constexpr const wchar_t* Names[] = {L"Density", L"Star size", L"Fall speed", L"Opacity (%)"};
int dip(int value) { return MulDiv(value, static_cast<int>(dpi), 96); }

HWND make(const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int width, int height, int id = 0) {
    HWND child = CreateWindowExW(wcscmp(cls, L"Edit") == 0 ? WS_EX_CLIENTEDGE : 0, cls, text,
        WS_CHILD | WS_VISIBLE | style, dip(x), dip(y), dip(width), dip(height), meteorOptionsWindow,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance, nullptr);
    SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return child;
}

void refresh() {
    loading = true;
    const int values[] = {config.meteorDensity, config.meteorSize, config.meteorSpeed, config.meteorOpacity};
    for (int i = 0; i < 4; ++i) {
        SendDlgItemMessageW(meteorOptionsWindow, SliderIds[i], TBM_SETPOS, TRUE, values[i]);
        SetDlgItemInt(meteorOptionsWindow, LabelIds[i], static_cast<UINT>(values[i]), FALSE);
    }
    wchar_t color[8]{}, current[16]{};
    swprintf_s(color, L"%06X", config.meteorColor);
    GetDlgItemTextW(meteorOptionsWindow, Color, current, 16);
    if (wcscmp(color, current)) SetDlgItemTextW(meteorOptionsWindow, Color, color);
    loading = false;
}

void changed() { refresh(); scheduleSave(); }

void layout() {
    loading = true;
    dpi = GetDpiForWindow(meteorOptionsWindow);
    font = CreateFontW(-dip(15), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    make(L"Static", L"Meteor trail", 0, 22, 16, 372, 24);
    make(L"Static", L"Falling stars follow the path, then fade away.", 0, 22, 45, 372, 24);
    make(L"Static", L"Color (hex)", 0, 22, 86, 130, 24);
    make(L"Edit", L"", ES_UPPERCASE | ES_AUTOHSCROLL | WS_TABSTOP, 160, 82, 110, 28, Color);
    SendDlgItemMessageW(meteorOptionsWindow, Color, EM_SETLIMITTEXT, 6, 0);
    make(L"Button", L"Choose...", WS_TABSTOP, 284, 82, 110, 28, Pick);
    constexpr int minimum[] = {5, 2, 40, 0}, maximum[] = {100, 8, 240, 100};
    for (int i = 0; i < 4; ++i) {
        const int y = 132 + i * 58;
        make(L"Static", Names[i], 0, 22, y, 260, 22);
        HWND slider = make(TRACKBAR_CLASSW, Names[i], TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
            16, y + 24, 384, 26, SliderIds[i]);
        make(L"Static", L"", SS_RIGHT, 332, y, 60, 22, LabelIds[i]);
        SendMessageW(slider, TBM_SETRANGE, TRUE, MAKELPARAM(minimum[i], maximum[i]));
    }
    make(L"Static", L"Changes apply immediately.", 0, 22, 370, 372, 36, Info);
    make(L"Button", L"Close", WS_TABSTOP | BS_DEFPUSHBUTTON, 292, 416, 100, 30, Done);
    refresh(); loading = false;
}

LRESULT CALLBACK procedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
    case WM_CREATE: meteorOptionsWindow = hwnd; layout(); return 0;
    case WM_COMMAND: {
        if (loading) return 0;
        const int id = LOWORD(wp), event = HIWORD(wp);
        if (id == Done || id == IDCANCEL) { SendMessageW(hwnd, WM_CLOSE, 0, 0); return 0; }
        if (id == Color && event == EN_CHANGE) {
            wchar_t text[16]{}; unsigned color{};
            GetDlgItemTextW(hwnd, Color, text, 16);
            if (parseColor(text, color)) { config.meteorColor = color; changed(); }
            else status(L"Enter a six-digit hex color, for example 72CFFF.");
        }
        if (id == Pick && event == BN_CLICKED) {
            static COLORREF custom[16]{};
            CHOOSECOLORW chooser{sizeof(chooser)};
            chooser.hwndOwner = hwnd; chooser.lpCustColors = custom;
            chooser.rgbResult = RGB((config.meteorColor >> 16) & 255, (config.meteorColor >> 8) & 255, config.meteorColor & 255);
            chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColorW(&chooser)) {
                config.meteorColor = (GetRValue(chooser.rgbResult) << 16) | (GetGValue(chooser.rgbResult) << 8) | GetBValue(chooser.rgbResult);
                changed();
            }
        }
        return 0;
    }
    case WM_HSCROLL: {
        const int id = GetDlgCtrlID(reinterpret_cast<HWND>(lp));
        const int value = static_cast<int>(SendMessageW(reinterpret_cast<HWND>(lp), TBM_GETPOS, 0, 0));
        if (id == Density) config.meteorDensity = value;
        if (id == Size) config.meteorSize = value;
        if (id == Speed) config.meteorSpeed = value;
        if (id == Alpha) config.meteorOpacity = value;
        changed(); return 0;
    }
    case WM_DPICHANGED: {
        const auto rect = *reinterpret_cast<RECT*>(lp);
        while (HWND child = GetWindow(hwnd, GW_CHILD)) DestroyWindow(child);
        DeleteObject(font); font = nullptr; layout();
        SetWindowPos(hwnd, nullptr, rect.left, rect.top, rect.right - rect.left,
            rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_CLOSE: saveConfig(); DestroyWindow(hwnd); return 0;
    case WM_DESTROY: meteorOptionsWindow = nullptr; DeleteObject(font); font = nullptr; return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
}

void meteorStatus(const wchar_t* message) {
    if (meteorOptionsWindow) SetDlgItemTextW(meteorOptionsWindow, Info, message);
}

void registerMeteorOptionsClass() {
    WNDCLASSW cls{};
    cls.hInstance = instance; cls.lpszClassName = L"MouseOptionsMeteorSettings";
    cls.lpfnWndProc = procedure; cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    RegisterClassW(&cls);
}

void showMeteorOptions() {
    if (!meteorOptionsWindow) {
        RECT main{}; GetWindowRect(settingsWindow, &main);
        MONITORINFO monitor{sizeof(monitor)};
        GetMonitorInfoW(MonitorFromWindow(settingsWindow, MONITOR_DEFAULTTONEAREST), &monitor);
        meteorOptionsWindow = CreateWindowExW(WS_EX_TOOLWINDOW, L"MouseOptionsMeteorSettings", L"Meteor Options",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, main.left, main.top, 440, 500,
            settingsWindow, nullptr, instance, nullptr);
        RECT rect{0, 0, dip(420), dip(466)};
        AdjustWindowRectExForDpi(&rect, static_cast<DWORD>(GetWindowLongPtrW(meteorOptionsWindow, GWL_STYLE)), FALSE, WS_EX_TOOLWINDOW, dpi);
        const int width = rect.right - rect.left, height = rect.bottom - rect.top;
        const int x = std::max(monitor.rcWork.left, std::min(main.right + dip(12), monitor.rcWork.right - width));
        const int y = std::max(monitor.rcWork.top, std::min(main.top, monitor.rcWork.bottom - height));
        SetWindowPos(meteorOptionsWindow, nullptr, x, y, width, height, SWP_NOZORDER);
    }
    ShowWindow(meteorOptionsWindow, SW_SHOWNORMAL); SetForegroundWindow(meteorOptionsWindow);
}
}
