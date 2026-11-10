# Mouse Options

Version 0.2.0 — meteor trail.

Lightweight native Windows 11 cursor highlight. C++ / Win32, no browser runtime.

## Run

Extract the x64 portable ZIP into a writable folder and run `MouseOptions.exe`.
Click its system-tray icon (possibly inside the hidden-icons arrow) to open settings.
Right-click for Settings, Highlight enabled, or Exit. Closing settings keeps the app
running. Launching the executable again opens the existing settings window.

- Filled circle or hollow ring; live color picker / six-digit hex color.
- Independent size, ring thickness, opacity, optional soft glow (off by default).
- **Meteor trail** is a separate opt-in effect alongside the circle/ring. Moving the
  pointer emits small stars that fall diagonally with luminous tails and fade away.
  Tick **Meteor trail**, then use **Options...** for its own color, density, star size,
  fall speed and opacity. Settings save automatically. Existing highlight settings stay separate.
- Default global toggle: **Ctrl+Alt+H**. Choose a replacement and click **Apply hotkey**.
  It toggles all cursor effects. Conflicts show an error and preserve the previous working combination.
- **Start when I sign in** is off by default; opt in from settings.
- Multiple-monitor positioning and per-monitor DPI scaling. Real two-monitor and
  sign-in acceptance checks are listed in `MANUAL-CHECKS.md` and still await hardware validation.

Normal desktop/windowed applications are supported; secure desktop and exclusive
fullscreen games are outside the initial scope. Windows 11 x64 is the packaged target.

Command line: `MouseOptions.exe --settings` opens settings; `--background` starts in
the tray. One instance per Windows session, including after relocating the portable app.

## Portable settings

Settings apply immediately and are saved next to the executable in `mouse-options.ini`.
Use a writable folder. Move the executable and its INI together; launch once from the
new location to repair an enabled startup registration. Turn startup off before removing
the portable app. Windows Startup Apps can independently disable login execution.

The public INI format uses ASCII or UTF-16, section `[highlight]`. Edit while the app is
stopped; missing/corrupt values recover to defaults or valid bounded values.

| Key | Default | Meaning |
| --- | --- | --- |
| style | filled | filled or ring |
| color | FFD200 | Six hex digits, no # prefix |
| diameter | 48 | 8–256 logical pixels, excluding glow |
| thickness | 4 | Logical pixels, 1 to half the diameter minus 1 |
| opacity | 25 | 0–100 percent |
| glow | 0 | 0/1, soft outer halo matching the highlight color |
| enabled | 1 | 0/1, highlight visibility |
| hotkey_modifiers | 3 | Windows modifier bits: Alt=1, Ctrl=2, Shift=4 |
| hotkey_key | 72 | Windows virtual-key code; 72 means H |
| startup | 0 | 0/1, desired automatic login startup |
| meteor | 0 | 0/1, optional meteor trail alongside the highlight |
| meteor_color | 72CFFF | Six hex digits for stars, independent of highlight color |
| meteor_density | 55 | 5–100; higher values emit stars closer together |
| meteor_size | 4 | 2–8 logical pixels for the star head |
| meteor_speed | 110 | 40–240, falling speed in logical pixels per second before gravity |
| meteor_opacity | 85 | 0–100 percent; zero releases trail rendering resources |

## Build and test

Requires Visual Studio 2022 or 2026 with Desktop development with C++, Windows SDK and CMake.

```powershell
./scripts/build.ps1 -Package
python tests/run_tests.py
```

Output: `dist/MouseOptions-x64.zip`. Static C++ runtime; target machines need no SDK.
Tests require an unlocked interactive Windows desktop and move the mouse temporarily.
Exit other running Mouse Options instances before testing. Integration tests snapshot
and restore the app's current-user Run value in startup scenarios.

```powershell
python tests/measure_resources.py docs/evidence/resources.json
```

Resource measurements use Release without trimming working sets. A cached, premultiplied
bitmap is regenerated only when appearance/DPI changes. A 16 ms Windows message timer
tracks position, skips unchanged positions and is stopped when highlight is disabled;
disabling also releases render resources. Settings controls initialize lazily, and the
settings window is destroyed on close. No browser, CLR, graphics-device allocation or
network service is required.

See `docs/testing.md` for agreed test seams, TDD cycles, actual evidence and remaining checks.

The meteor renderer has a fixed 64-star pool and a maximum 512×384 pixel CPU canvas.
Older particles are retired if the trail would exceed that canvas; no desktop-sized buffer
is allocated. Rendering resources are released when the final star expires or the effect
is disabled. Version 0.2.0 final interactive/performance testing is delegated to the user.
