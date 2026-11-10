"""Reproducible Release resource measurements; no working-set trimming."""
import ctypes as C
from ctypes import wintypes as W
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time
from test_desktop import DesktopTests, EXE, ROOT, u, window, settings_window, button, click, hotkey, move_cursor, wait_for, child_texts

k = C.WinDLL('kernel32', use_last_error=True)
p = C.WinDLL('psapi', use_last_error=True)

class Memory(C.Structure):
    _fields_ = [('cb', W.DWORD), ('PageFaultCount', W.DWORD)] + [
        (name, C.c_size_t) for name in ('PeakWorkingSetSize', 'WorkingSetSize', 'QuotaPeakPagedPoolUsage',
        'QuotaPagedPoolUsage', 'QuotaPeakNonPagedPoolUsage', 'QuotaNonPagedPoolUsage',
        'PagefileUsage', 'PeakPagefileUsage', 'PrivateUsage')]

p.GetProcessMemoryInfo.argtypes = [W.HANDLE, C.c_void_p, W.DWORD]
k.GetProcessTimes.argtypes = [W.HANDLE] + [C.POINTER(W.FILETIME)] * 4
k.GetProcessHandleCount.argtypes = [W.HANDLE, C.POINTER(W.DWORD)]
u.GetGuiResources.argtypes = [W.HANDLE, W.DWORD]

def resources(process):
    handle = int(process._handle)
    memory = Memory()
    memory.cb = C.sizeof(memory)
    if not p.GetProcessMemoryInfo(handle, C.byref(memory), memory.cb):
        raise C.WinError(C.get_last_error())
    handles = W.DWORD()
    k.GetProcessHandleCount(handle, C.byref(handles))
    return dict(working_set_bytes=memory.WorkingSetSize, private_bytes=memory.PrivateUsage,
                gdi_objects=u.GetGuiResources(handle, 0), user_objects=u.GetGuiResources(handle, 1),
                handles=handles.value)

def cpu_seconds(process):
    values = [W.FILETIME() for _ in range(4)]
    if not k.GetProcessTimes(int(process._handle), *[C.byref(v) for v in values]):
        raise C.WinError(C.get_last_error())
    return sum((v.dwHighDateTime << 32) | v.dwLowDateTime for v in values[2:]) / 10_000_000

def sample(process, moving=False, seconds=2):
    time.sleep(.5)
    before, started = cpu_seconds(process), time.monotonic()
    index = 0
    while time.monotonic() - started < seconds:
        if moving:
            move_cursor(450 + (index * 7) % 500, 300 + (index * 3) % 200)
            index += 1
        time.sleep(.02)
    elapsed = time.monotonic() - started
    result = resources(process)
    result['cpu_percent_one_core'] = round((cpu_seconds(process) - before) / elapsed * 100, 3)
    result['sample_seconds'] = round(elapsed, 3)
    return result

def measure(output):
    case = DesktopTests('test_highlight_follows_cursor_without_taking_focus')
    case.setUp()
    try:
        overlay = case.launch()
        report = dict(platform=platform.platform(), architecture=platform.machine(), logical_cpus=os.cpu_count(),
                      physical_monitor_count=u.GetSystemMetrics(80), dpi=u.GetDpiForWindow(overlay),
                      executable_bytes=EXE.stat().st_size, executable_sha256=hashlib.sha256(EXE.read_bytes()).hexdigest(),
                      notes='Single physical monitor. No EmptyWorkingSet or SetProcessWorkingSetSize calls.', states={})
        states = report['states']
        states['fresh_background_idle'] = sample(case.process)
        states['background_moving'] = sample(case.process, moving=True)
        hotkey(); wait_for(lambda: not u.IsWindowVisible(overlay), 'Disable failed')
        states['highlight_off_idle'] = sample(case.process)
        hotkey(); wait_for(lambda: u.IsWindowVisible(overlay), 'Enable failed')
        subprocess.run([str(case.exe), '--settings'], check=True, timeout=4)
        settings = settings_window()
        states['settings_open'] = sample(case.process)
        click(button(settings, 'Glow'))
        states['glow_on_idle'] = sample(case.process)
        states['glow_on_moving'] = sample(case.process, moving=True)
        click(button(settings, 'Close'))
        wait_for(lambda: not window('Mouse Options Settings'), 'Close failed')
        states['settings_closed_glow_on'] = sample(case.process)
        before = resources(case.process)
        checkpoints = []
        for iteration in range(30):
            subprocess.run([str(case.exe), '--settings'], check=True, timeout=4)
            settings = settings_window()
            click(button(settings, 'Glow'))
            click(button(settings, 'Ring'))
            click(button(settings, 'Filled'))
            hotkey_field = u.FindWindowExW(settings, None, 'msctls_hotkey32', None)
            u.SendMessageW(hotkey_field, 0x0401, (0x78 + iteration % 2) | (7 << 8), 0)
            click(button(settings, 'Apply hotkey'))
            if not any('Hotkey registered.' in value for value in child_texts(settings)):
                raise AssertionError('Resource cycle hotkey registration failed; test shortcut may be occupied')
            click(button(settings, 'Close'))
            wait_for(lambda: not window('Mouse Options Settings'), 'Close failed')
            if (iteration + 1) % 10 == 0:
                time.sleep(.1)
                checkpoints.append(dict(cycles=iteration + 1, **resources(case.process)))
        after = sample(case.process)
        report['thirty_settings_glow_style_hotkey_cycles'] = dict(before=before, checkpoints=checkpoints, after=after)
        # Reach the maximum through the native slider's public keyboard behavior.
        subprocess.run([str(case.exe), '--settings'], check=True, timeout=4)
        settings = settings_window()
        slider = u.FindWindowExW(settings, None, 'msctls_trackbar32', None)
        u.SendMessageW(slider, 0x0100, 0x23, 0)  # VK_END: maximum diameter.
        click(button(settings, 'Close'))
        states['maximum_size_glow_on_moving'] = sample(case.process, moving=True)
        click_exit = subprocess.run([str(case.exe), '--settings'], check=True, timeout=4)
        settings = settings_window()
        click(button(settings, 'Exit'))
        case.process.wait(5)
        report['graceful_exit_code'] = case.process.returncode
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2), encoding='utf-8')
        print(json.dumps(report, indent=2))
    finally:
        case.doCleanups()

if __name__ == '__main__':
    measure(Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'docs/evidence/resources.json')
