#pragma once
#include <windows.h>
#include <string>

namespace mo {
constexpr wchar_t AppName[] = L"Mouse Options";
constexpr UINT OpenSettings = 100, Toggle = 101, Quit = 102;
enum Control : int { Enabled = 200, Filled, Ring, ColorEdit, ColorPick,
    Diameter, Thickness, Opacity, DiameterValue, ThicknessValue, OpacityValue, Status, Close, Glow, Hotkey, ApplyHotkey, Startup, MeteorToggle, MeteorOptions };
struct Settings {
    bool ring = false;
    bool glow = false;
    bool startup = false;
    bool meteor = false;
    int diameter = 48, thickness = 4, opacity = 25;
    unsigned color = 0xFFD200;
    unsigned meteorColor = 0x72CFFF;
    int meteorDensity = 55, meteorSize = 4, meteorSpeed = 110, meteorOpacity = 85;
    UINT hotkeyModifiers = MOD_CONTROL | MOD_ALT, hotkeyKey = 'H';
};
extern Settings config;
extern HINSTANCE instance;
extern HWND owner, overlay, settingsWindow;
extern HWND meteorOptionsWindow;
extern bool enabled;
extern std::wstring configPath;
void createBitmap();
void freeBitmap();
void moveOverlay(bool upload = false);
void setEnabled(bool value);
void showSettings();
void refreshSettings();
void registerSettingsClass();
void status(const wchar_t* message);
void scheduleSave();
bool saveConfig();
bool applyHotkey(UINT modifiers, UINT key);
void shutdownHotkey();
bool setStartup(bool value);
bool changeStartupPreference(bool value);
bool parseColor(const wchar_t* text, unsigned& value);
void tickMeteors();
void resetMeteors();
void registerMeteorOptionsClass();
void showMeteorOptions();
void meteorStatus(const wchar_t* message);
}
