# Testing agreement and evidence

## Version scope

The historical 22-test integration report, resource samples and six-issue acceptance
audit below describe the 0.1.0 baseline binary identified by their SHA-256 hashes.
Version 0.2.0 adds meteor trails. The user requested completing the code and will test
the final release themselves; do not treat baseline reports as proof for the new binary.
See `.scratch/meteor-trail/PRD.md` and its release record for the current feature.

## Agreed seams

2026-11-10: User approved all three seams before the first test:

1. Native UI and user interactions.
2. Public portable configuration and restarting the app.
3. Windows integration: global hotkey, startup, DPI and overlay.

Tests observe those boundaries, not private functions or app-only test messages.
Use one red → green behavior at a time; keep build and UI evidence here.

## TDD log

Each cycle first produced the listed failing observation, then passed after implementation.

| Cycle | Public behavior | Red observation |
| --- | --- | --- |
| 01 | Launch, pointer-following, preserve foreground | Release executable did not exist |
| 02 | Portable diameter setting | Requested 80; observed fixed-size 50-pixel window |
| 03 | Ring setting survives close/restart | Settings window did not exist |
| 04 | React to Windows DPI response | Fixed-size highlight did not grow at 150% |
| 05 | Optional persistent glow | Glow button did not exist |
| 06 | Global hotkey | Ctrl+Alt+H did not disable the highlight |
| 07 | Startup / singleton path | Startup checkbox did not exist |
| 08 | Reject malformed hex input | +FF000 was accepted |
| 09 | Release disabled rendering resources | GDI count stayed at 2 after disabling |
| 10 | Accessible appearance controls / persistence | Sliders lacked semantic names |
| 11 | Startup + read-only portable settings | Startup remained enabled after config-save failure |
| 12 | Glow updates after returning to an external topmost window | External window covered the cached highlight |

The initial mouse-hook implementation did not deliver pointer updates on the test
desktop. Replaced it with a Windows message timer, verified real input tracking and
measured CPU. The timer skips unchanged position/foreground state and is removed when off.

## Reproduction

```powershell
./scripts/build.ps1 -Package
python tests/run_tests.py --exe dist/MouseOptions-x64/MouseOptions.exe
python tests/measure_resources.py docs/evidence/resources.json
```

Exit other Mouse Options instances and use an unlocked interactive Windows desktop.
Run UI tests serially. They move the pointer, use temporary topmost white canvases,
press/release test shortcuts and preserve the app's current-user Run value around
startup scenarios. Tests use isolated portable copies in temporary folders with spaces.

## Evidence

- [Full integration output](evidence/integration-tests.txt) and [summary with binary hash](evidence/integration-summary.json).
- [Resource measurements](evidence/resources.json), with working set, private bytes,
  GDI/USER objects, process handles and CPU sample duration. CPU is percent of one
  logical core, not Task Manager's whole-machine percentage; short samples are quantized.
- [Package manifest](evidence/package.json): x64, ZIP contents, binary and archive hashes.
- Resource cycles repeatedly open/close settings, switch styles, toggle glow and replace
  hotkeys; final output records actual before/after numbers rather than claiming a universal bound.
- Static runtime confirmed by PE import inspection: USER32, GDI32, SHELL32, COMCTL32,
  COMDLG32, ADVAPI32, KERNEL32 only. No Visual C++ redistributable, .NET or browser runtime import.

## Actual versus simulated coverage

Actual machine: Windows 11 build 26200, x64, one physical monitor at 144 DPI (150%).

Actual Windows execution covers pointer following, focus, pixel color/alpha, hollow
ring center, soft colored glow and live recoloring, click/drag through the overlay,
settings persistence/corruption/read-only errors, global shortcuts/conflicts/no-repeat,
startup Run registration/removal/failure rollback, moved portable paths and singleton
activation. The packaged executable is used by the final integration suite.

`windows_fixture.py` replaces selected imported Windows API responses **only inside
the test's own child process**. It never patches application functions and is not
shipped. GetDpiForWindow responses at 96/144 DPI, MonitorFromPoint identity changes,
negative/boundary GetCursorPos coordinates, WM_DISPLAYCHANGE and registry access-denied
responses exercise the OS integration seam. These are simulated monitor/topology/error
responses, not a claim that a second physical monitor was attached.

Pixel tests replay the pointer location while keeping the actual pointer away from
sampled pixels, isolating rendering from the computer-use helper's blue pointer overlay.
The white external canvas baseline is checked before sampling. Real mouse input is used
separately to prove pointer following, clicks and dragging.

Native UI was also inspected with the computer-use skill: ring selection enabled its
thickness control, the Windows color picker opened/cancelled, and Exit removed the window.
An accessibility action for the modal Cancel button was unavailable in the helper cache;
Escape dismissed the picker successfully. No app failure was inferred from that helper error.

## Remaining acceptance gates

Real two-monitor 100%/150% transitions, primary-display changes and physical disconnect /
reconnect have not been performed on this one-monitor machine. Real sign-out/sign-in
startup execution has not been performed; registry state and direct startup invocation
have been tested. No automatic logoff, monitor-driver install or system display change
was used to manufacture those results.

Follow [manual checks](manual-checks.md) and update the local issue evidence with actual
results. Until then, the six features are implemented but the full six-issue goal is
not claimed complete. See the [acceptance audit](../.scratch/cursor-highlight/VERIFICATION.md).

The user confirmed only one monitor is available and explicitly deferred these real
monitor/logon checks. They remain pending, not waived or passed.

## Windows API references

- [Layered windows and mouse event pass-through](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features).
- [UpdateLayeredWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatelayeredwindow).
- [RegisterHotKey and MOD_NOREPEAT](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerhotkey).
- [Current-user Run registration](https://learn.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys).
