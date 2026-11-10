"""Black-box Windows integration tests. Run on an interactive desktop.

Only public windows, UI controls, documented portable config and Windows state.
No app internals or private test messages. Each test gets an isolated portable copy.
"""
import ctypes as C
from ctypes import wintypes as W
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
import unittest
import winreg
import os
from windows_fixture import replay_dpi, replay_cursor, replace_windows_import
from canvas import Canvas, pump
from windows_fixture import k as kernel

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MOUSE_OPTIONS_EXE', ROOT / 'build/Release/MouseOptions.exe'))
u = C.WinDLL('user32', use_last_error=True)
u.FindWindowW.argtypes = [W.LPCWSTR, W.LPCWSTR]
u.FindWindowW.restype = W.HWND
u.GetForegroundWindow.restype = W.HWND
u.GetWindowRect.argtypes = [W.HWND, C.POINTER(W.RECT)]
u.IsWindowVisible.argtypes = [W.HWND]
u.PostMessageW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.GetCursorPos.argtypes = [C.POINTER(W.POINT)]
u.GetDpiForWindow.argtypes = [W.HWND]
u.FindWindowExW.argtypes = [W.HWND, W.HWND, W.LPCWSTR, W.LPCWSTR]
u.FindWindowExW.restype = W.HWND
u.SendMessageW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.SendMessageW.restype = C.c_ssize_t
u.GetWindowTextW.argtypes = [W.HWND, W.LPWSTR, C.c_int]
u.RegisterHotKey.argtypes = [W.HWND, C.c_int, W.UINT, W.UINT]
u.UnregisterHotKey.argtypes = [W.HWND, C.c_int]
u.WaitForInputIdle.argtypes = [W.HANDLE, W.DWORD]
u.GetGuiResources.argtypes = [W.HANDLE, W.DWORD]
u.SetProcessDpiAwarenessContext.argtypes = [C.c_void_p]
u.SetProcessDpiAwarenessContext(C.c_void_p(-4))

def move_cursor(x, y):
    # Real mouse-input path: SetCursorPos alone does not emit WH_MOUSE_LL input.
    width, height = u.GetSystemMetrics(0), u.GetSystemMetrics(1)
    u.mouse_event(0x8001, (x * 65536 + 32768) // width,
                  (y * 65536 + 32768) // height, 0, 0)

def wait_for(predicate, message, seconds=4):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        pump()
        value = predicate()
        if value:
            return value
        time.sleep(.025)
    raise AssertionError(message)

def window(title):
    return u.FindWindowW(None, title)

def bounds(hwnd):
    rect = W.RECT()
    if not u.GetWindowRect(hwnd, C.byref(rect)):
        raise AssertionError('Window no longer exists')
    return rect.left, rect.top, rect.right, rect.bottom

def button(parent, caption):
    return wait_for(lambda: u.FindWindowExW(parent, None, 'Button', caption),
                    f'Button not found: {caption}')

def settings_window():
    return wait_for(lambda: (h if (h := window('Mouse Options Settings')) and
                    u.FindWindowExW(h, None, 'Button', 'Close') else None), 'Settings did not finish opening')

def click(hwnd):
    u.SendMessageW(hwnd, 0x00F5, 0, 0)  # BM_CLICK: public native button action.

def text(hwnd):
    value = C.create_unicode_buffer(1024)
    u.SendMessageW(hwnd, 0x000D, len(value), C.cast(value, C.c_void_p).value)
    return value.value

def child_texts(parent):
    result, child = [], None
    while (child := u.FindWindowExW(parent, child, None, None)):
        result.append(text(child))
    return result

def hotkey(key=ord('H')):
    try:
        for vk in (0x11, 0x12, key):
            u.keybd_event(vk, 0, 0, 0)
    finally:
        for vk in (key, 0x12, 0x11):
            u.keybd_event(vk, 0, 2, 0)

def preserve_startup_value(case):
    path = r'Software\Microsoft\Windows\CurrentVersion\Run'
    def read():
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, path) as key:
                return winreg.QueryValueEx(key, 'MouseOptions')
        except FileNotFoundError:
            return None
    original = read()
    def restore():
        with winreg.CreateKey(winreg.HKEY_CURRENT_USER, path) as key:
            if original:
                winreg.SetValueEx(key, 'MouseOptions', 0, original[1], original[0])
            else:
                try:
                    winreg.DeleteValue(key, 'MouseOptions')
                except FileNotFoundError:
                    pass
    case.addCleanup(restore)
    return read

class DesktopTests(unittest.TestCase):
    def setUp(self):
        self.assertTrue(EXE.is_file(), 'Build Release portable app first')
        self.tmp = tempfile.TemporaryDirectory(prefix='Mouse Options test ')
        self.addCleanup(self.tmp.cleanup)
        self.folder = Path(self.tmp.name)
        self.exe = self.folder / EXE.name
        shutil.copy2(EXE, self.exe)
        self.process = None
        self.cursor = W.POINT()
        u.GetCursorPos(C.byref(self.cursor))
        self.addCleanup(lambda: u.SetCursorPos(self.cursor.x, self.cursor.y))
        self.addCleanup(self.stop)

    def launch(self, *args):
        self.process = subprocess.Popen([str(self.exe), *args])
        self.assertEqual(u.WaitForInputIdle(int(self.process._handle), 4000), 0, 'App did not become idle')
        return wait_for(lambda: (h if (h := window('Mouse Options Highlight')) and
                                  u.IsWindowVisible(h) else None), 'Highlight did not start')

    def stop(self):
        if self.process and self.process.poll() is None:
            self.process.terminate()
            self.process.wait(5)
        time.sleep(.1)

    def test_highlight_follows_cursor_without_taking_focus(self):
        foreground = u.GetForegroundWindow()
        overlay = self.launch()
        self.assertEqual(u.GetForegroundWindow(), foreground)
        move_cursor(320, 240)
        wait_for(lambda: ((bounds(overlay)[0] + bounds(overlay)[2]) // 2,
                          (bounds(overlay)[1] + bounds(overlay)[3]) // 2) == (320, 240),
                 'Highlight did not follow pointer')
        left, top, right, bottom = bounds(overlay)
        self.assertEqual((top + bottom) // 2, 240)
        self.assertTrue(u.IsWindowVisible(overlay))
        self.assertEqual(u.GetForegroundWindow(), foreground)

    def test_portable_config_controls_highlight_diameter(self):
        (self.folder / 'mouse-options.ini').write_text(
            '[highlight]\ndiameter=80\ncolor=FF0000\nopacity=35\n', encoding='utf-16')
        overlay = self.launch()
        left, top, right, bottom = bounds(overlay)
        logical_pixels = round(80 * u.GetDpiForWindow(overlay) / 96)
        self.assertGreaterEqual(right - left, logical_pixels)
        self.assertLessEqual(right - left, logical_pixels + 4)
        self.assertEqual(right - left, bottom - top)

    def test_ring_choice_survives_closing_settings_and_restarting(self):
        overlay = self.launch('--settings')
        settings = settings_window()
        ring = button(settings, 'Ring')
        click(ring)
        self.assertEqual(u.SendMessageW(ring, 0x00F0, 0, 0), 1)
        u.PostMessageW(settings, 0x0010, 0, 0)
        wait_for(lambda: not window('Mouse Options Settings'), 'Settings did not close')
        self.assertTrue(u.IsWindowVisible(overlay))
        self.stop()
        self.launch('--settings')
        settings = settings_window()
        self.assertEqual(u.SendMessageW(button(settings, 'Ring'), 0x00F0, 0, 0), 1)

    def test_highlight_reacts_to_windows_dpi_changes(self):
        overlay = self.launch()
        replay_dpi(self.process, self.exe, 96)
        move_cursor(400, 320)
        wait_for(lambda: bounds(overlay)[2] - bounds(overlay)[0] == 50,
                 '48 logical pixels must render as 48 pixels at 100%, plus AA border')
        replay_dpi(self.process, self.exe, 144)
        move_cursor(500, 320)
        wait_for(lambda: bounds(overlay)[2] - bounds(overlay)[0] == 74,
                 '48 logical pixels must render as 72 pixels at 150%, plus AA border')
        replay_dpi(self.process, self.exe, 96)
        move_cursor(600, 320)
        wait_for(lambda: bounds(overlay)[2] - bounds(overlay)[0] == 50,
                 'Returning to 100% must restore the original size')

    def test_glow_expands_halo_and_survives_restart(self):
        overlay = self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        before = bounds(overlay)[2] - bounds(overlay)[0]
        glow = button(settings, 'Glow')
        self.assertEqual(u.SendMessageW(glow, 0x00F0, 0, 0), 0)
        click(glow)
        wait_for(lambda: bounds(overlay)[2] - bounds(overlay)[0] > before + 4,
                 'Glow halo did not appear')
        click(button(settings, 'Close'))
        self.stop()
        self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not reopen')
        self.assertEqual(u.SendMessageW(button(settings, 'Glow'), 0x00F0, 0, 0), 1)

    def test_global_hotkey_toggles_without_opening_settings(self):
        overlay = self.launch()
        foreground = u.GetForegroundWindow()
        hotkey()
        wait_for(lambda: not u.IsWindowVisible(overlay), 'Global hotkey did not disable highlight')
        self.assertFalse(window('Mouse Options Settings'))
        self.assertEqual(u.GetForegroundWindow(), foreground)
        hotkey()
        wait_for(lambda: u.IsWindowVisible(overlay), 'Global hotkey did not enable highlight')

    def test_startup_toggle_registers_quoted_portable_path(self):
        path = r'Software\Microsoft\Windows\CurrentVersion\Run'
        def read():
            try:
                with winreg.OpenKey(winreg.HKEY_CURRENT_USER, path) as key:
                    return winreg.QueryValueEx(key, 'MouseOptions')
            except FileNotFoundError:
                return None
        original = read()
        def restore():
            with winreg.CreateKey(winreg.HKEY_CURRENT_USER, path) as key:
                if original:
                    winreg.SetValueEx(key, 'MouseOptions', 0, original[1], original[0])
                else:
                    try:
                        winreg.DeleteValue(key, 'MouseOptions')
                    except FileNotFoundError:
                        pass
        self.addCleanup(restore)
        self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        startup = button(settings, 'Start when I sign in')
        self.assertEqual(u.SendMessageW(startup, 0x00F0, 0, 0), 0)
        click(startup)
        wait_for(lambda: read() and read()[0] == f'"{self.exe}" --background', 'Startup command not registered')
        click(button(settings, 'Close'))
        self.stop()
        self.launch('--background')
        self.assertFalse(window('Mouse Options Settings'))
        second = subprocess.Popen([str(self.exe), '--settings'])
        second.wait(4)
        self.assertEqual(second.returncode, 0)
        settings = settings_window()
        self.assertIsNone(self.process.poll())
        self.assertEqual(u.SendMessageW(button(settings, 'Start when I sign in'), 0x00F0, 0, 0), 1)
        click(button(settings, 'Start when I sign in'))
        wait_for(lambda: read() is None, 'Startup command not removed')

    def test_filled_pixels_blend_color_and_allow_click_through(self):
        canvas = Canvas()
        self.addCleanup(canvas.close)
        move_cursor(1100, 800)
        wait_for(lambda: canvas.pixel(490, 400) == (255, 255, 255), 'White test canvas is not visible')
        (self.folder / 'mouse-options.ini').write_text(
            '[highlight]\ndiameter=80\ncolor=FF0000\nopacity=50\n', encoding='utf-16')
        overlay = self.launch()
        # Isolate rendered pixels from the computer-use helper's blue pointer feedback.
        replay_cursor(self.process, self.exe, 500, 400)
        wait_for(lambda: (bounds(overlay)[0] + bounds(overlay)[2]) // 2 == 500, 'Pointer did not move')
        time.sleep(.15)
        red, green, blue = canvas.pixel(490, 400)  # Left of the software cursor glyph/shadow.
        self.assertGreaterEqual(red, 250)
        self.assertTrue(120 <= green <= 135, (red, green, blue))
        self.assertTrue(120 <= blue <= 135, (red, green, blue))
        move_cursor(500, 400)
        u.mouse_event(0x0002, 0, 0, 0, 0)
        u.mouse_event(0x0004, 0, 0, 0, 0)
        wait_for(lambda: canvas.clicks == 1, 'Highlight intercepted the click')
        try:
            u.mouse_event(0x0002, 0, 0, 0, 0)
            move_cursor(550, 430)
            wait_for(lambda: canvas.received_drag, 'Highlight intercepted drag input')
        finally:
            u.mouse_event(0x0004, 0, 0, 0, 0)

    def test_ring_center_is_empty_and_outline_is_colored(self):
        canvas = Canvas()
        self.addCleanup(canvas.close)
        move_cursor(1100, 800)
        wait_for(lambda: canvas.pixel(490, 400) == (255, 255, 255), 'White test canvas is not visible')
        (self.folder / 'mouse-options.ini').write_text(
            '[highlight]\nstyle=ring\ndiameter=80\nthickness=8\ncolor=FF0000\nopacity=100\n', encoding='utf-16')
        overlay = self.launch()
        replay_cursor(self.process, self.exe, 500, 400)
        wait_for(lambda: (bounds(overlay)[0] + bounds(overlay)[2]) // 2 == 500, 'Pointer did not move')
        time.sleep(.15)
        self.assertEqual(canvas.pixel(490, 400), (255, 255, 255))
        radius = round(40 * u.GetDpiForWindow(overlay) / 96)
        red, green, blue = canvas.pixel(500 + radius - 4, 400)
        self.assertGreater(red, 245)
        self.assertLess(green, 10)
        self.assertLess(blue, 10)

    def test_negative_windows_coordinates_are_not_clamped(self):
        overlay = self.launch()
        replay_dpi(self.process, self.exe, 96)
        replay_cursor(self.process, self.exe, -700, -240)
        wait_for(lambda: bounds(overlay) == (-725, -265, -675, -215),
                 'Negative virtual desktop coordinates were not preserved')
        replay_cursor(self.process, self.exe, 450, 300)
        wait_for(lambda: bounds(overlay) == (425, 275, 475, 325),
                 'Returning to the primary display did not update position')

    def test_hotkey_conflict_preserves_old_then_new_choice_survives_restart(self):
        overlay = self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        field = u.FindWindowExW(settings, None, 'msctls_hotkey32', None)
        self.assertTrue(field)
        self.assertTrue(u.RegisterHotKey(None, 991, 3, ord('J')), 'Test hotkey already occupied')
        self.addCleanup(lambda: u.UnregisterHotKey(None, 991))
        u.SendMessageW(field, 0x0401, ord('J') | (6 << 8), 0)  # HKM_SETHOTKEY
        click(button(settings, 'Apply hotkey'))
        self.assertTrue(any('unavailable' in value for value in child_texts(settings)))
        hotkey()
        wait_for(lambda: not u.IsWindowVisible(overlay), 'Old hotkey was lost after conflict')
        hotkey()
        wait_for(lambda: u.IsWindowVisible(overlay), 'Old hotkey failed to restore highlight')
        u.UnregisterHotKey(None, 991)
        u.SendMessageW(field, 0x0401, ord('J') | (6 << 8), 0)
        click(button(settings, 'Apply hotkey'))
        click(button(settings, 'Close'))
        self.stop()
        overlay = self.launch()
        hotkey(ord('J'))
        wait_for(lambda: not u.IsWindowVisible(overlay), 'Changed hotkey was not restored')

    def test_corrupt_config_falls_back_to_usable_settings(self):
        (self.folder / 'mouse-options.ini').write_text(
            '[highlight]\nstyle=unknown\ndiameter=oops\nthickness=99999\nopacity=-50\ncolor=ZZZZZZ\n', encoding='utf-16')
        overlay = self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        self.assertEqual(u.SendMessageW(button(settings, 'Filled'), 0x00F0, 0, 0), 1)
        self.assertGreater(bounds(overlay)[2] - bounds(overlay)[0], 40)
        edit = u.FindWindowExW(settings, None, 'Edit', None)
        self.assertEqual(text(edit), 'FFD200')
        click(button(settings, 'Exit'))
        self.process.wait(4)
        self.assertEqual(self.process.returncode, 0)
        self.assertFalse(window('Mouse Options Highlight'))

    def test_startup_failure_does_not_claim_success(self):
        self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        # Windows boundary returns ERROR_ACCESS_DENIED; no real Run value is changed.
        replace_windows_import(self.process, self.exe, 'RegCreateKeyExW', b'\xb8\x05\0\0\0\xc3')
        startup = button(settings, 'Start when I sign in')
        click(startup)
        self.assertEqual(u.SendMessageW(startup, 0x00F0, 0, 0), 0)
        self.assertTrue(any('Cannot change Windows startup' in value for value in child_texts(settings)))

    def test_color_editor_rejects_non_hex_characters(self):
        self.launch('--settings')
        settings = wait_for(lambda: window('Mouse Options Settings'), 'Settings did not open')
        edit = u.FindWindowExW(settings, None, 'Edit', None)
        candidate = C.create_unicode_buffer('+FF000')
        u.SendMessageW(edit, 0x000C, 0, C.cast(candidate, C.c_void_p).value)
        self.assertTrue(any('six-digit hex color' in value for value in child_texts(settings)))

    def test_disabling_highlight_releases_render_resources(self):
        overlay = self.launch()
        before = u.GetGuiResources(int(self.process._handle), 0)
        hotkey()
        wait_for(lambda: not u.IsWindowVisible(overlay), 'Disable failed')
        self.assertLess(u.GetGuiResources(int(self.process._handle), 0), before)
        hotkey()
        wait_for(lambda: u.IsWindowVisible(overlay), 'Highlight did not recover after releasing resources')

    def test_appearance_controls_are_named_and_round_trip(self):
        overlay = self.launch('--settings')
        settings = settings_window()
        click(button(settings, 'Ring'))
        sliders = [u.FindWindowExW(settings, None, 'msctls_trackbar32', name)
                   for name in ('Size', 'Ring thickness', 'Opacity (%)')]
        self.assertTrue(all(sliders), 'Sliders need semantic names for accessibility')
        u.SendMessageW(sliders[0], 0x0100, 0x23, 0)  # End: size 256.
        size = bounds(overlay)[2] - bounds(overlay)[0]
        u.SendMessageW(sliders[1], 0x0100, 0x23, 0)  # End: thickness 127.
        self.assertEqual(bounds(overlay)[2] - bounds(overlay)[0], size)
        u.SendMessageW(sliders[2], 0x0100, 0x23, 0)  # End: opacity 100.
        edit = u.FindWindowExW(settings, None, 'Edit', None)
        color = C.create_unicode_buffer('12AB34')
        u.SendMessageW(edit, 0x000C, 0, C.cast(color, C.c_void_p).value)
        click(button(settings, 'Close'))
        self.stop()
        self.launch('--settings')
        settings = settings_window()
        restored = tuple(u.SendMessageW(u.FindWindowExW(settings, None, 'msctls_trackbar32', name), 0x0400, 0, 0)
                         for name in ('Size', 'Ring thickness', 'Opacity (%)'))
        self.assertEqual(restored, (256, 127, 100))
        self.assertEqual(text(u.FindWindowExW(settings, None, 'Edit', None)), '12AB34')

    def test_moved_portable_repairs_startup_registration(self):
        read = preserve_startup_value(self)
        self.launch('--settings')
        settings = settings_window()
        click(button(settings, 'Start when I sign in'))
        click(button(settings, 'Close'))
        self.stop()
        moved = self.folder / 'Moved portable folder'
        moved.mkdir()
        shutil.move(self.exe, moved / self.exe.name)
        shutil.move(self.folder / 'mouse-options.ini', moved / 'mouse-options.ini')
        self.exe = moved / self.exe.name
        self.launch('--background')
        self.assertEqual(read()[0], f'"{self.exe}" --background')
        self.assertFalse(window('Mouse Options Settings'))

    def test_read_only_configuration_reports_save_failure(self):
        path = self.folder / 'mouse-options.ini'
        original = '[highlight]\nstyle=filled\n'
        path.write_text(original, encoding='utf-16')
        kernel.SetFileAttributesW.argtypes = [W.LPCWSTR, W.DWORD]
        self.assertTrue(kernel.SetFileAttributesW(str(path), 1))
        self.addCleanup(lambda: kernel.SetFileAttributesW(str(path), 0x80))
        self.launch('--settings')
        settings = settings_window()
        click(button(settings, 'Ring'))
        wait_for(lambda: any('Cannot save.' in value for value in child_texts(settings)),
                 'Save failure was not reported')
        self.assertEqual(path.read_text(encoding='utf-16'), original)

    def test_holding_global_hotkey_does_not_toggle_repeatedly(self):
        overlay = self.launch()
        try:
            for key in (0x11, 0x12, ord('H')):
                u.keybd_event(key, 0, 0, 0)
            wait_for(lambda: not u.IsWindowVisible(overlay), 'Initial press did not toggle')
            for _ in range(8):
                u.keybd_event(ord('H'), 0, 0, 0)
                time.sleep(.04)
            self.assertFalse(u.IsWindowVisible(overlay))
        finally:
            for key in (ord('H'), 0x12, 0x11):
                u.keybd_event(key, 0, 2, 0)

    def test_glow_renders_soft_colored_pixels_outside_core(self):
        canvas = Canvas()
        self.addCleanup(canvas.close)
        move_cursor(1100, 800)
        wait_for(lambda: canvas.pixel(490, 400) == (255, 255, 255), 'White test canvas is not visible')
        (self.folder / 'mouse-options.ini').write_text(
            '[highlight]\nstyle=ring\nglow=1\ndiameter=80\nthickness=8\ncolor=FF0000\nopacity=100\n', encoding='utf-16')
        overlay = self.launch()
        replay_cursor(self.process, self.exe, 500, 400)
        wait_for(lambda: (bounds(overlay)[0] + bounds(overlay)[2]) // 2 == 500, 'Overlay did not move')
        time.sleep(.15)
        radius = round(40 * u.GetDpiForWindow(overlay) / 96)
        near = canvas.pixel(500 - radius - 6, 400)
        far = canvas.pixel(500 - radius - 12, 400)
        self.assertGreaterEqual(near[0], 250)
        self.assertTrue(190 <= near[1] <= 220, near)
        self.assertAlmostEqual(near[1], near[2], delta=2)
        self.assertGreater(far[1], near[1])
        self.assertEqual(canvas.pixel(490, 400), (255, 255, 255))
        subprocess.run([str(self.exe), '--settings'], check=True, timeout=4)
        settings = settings_window()
        edit = u.FindWindowExW(settings, None, 'Edit', None)
        green = C.create_unicode_buffer('00FF00')
        u.SendMessageW(edit, 0x000C, 0, C.cast(green, C.c_void_p).value)
        click(button(settings, 'Close'))
        wait_for(lambda: not window('Mouse Options Settings'), 'Settings did not close')
        time.sleep(.15)
        changed = canvas.pixel(500 - radius - 6, 400)
        self.assertGreaterEqual(changed[1], 250)
        self.assertTrue(190 <= changed[0] <= 220, changed)
        self.assertAlmostEqual(changed[0], changed[2], delta=2)

    def test_glow_survives_monitor_and_topology_response_changes(self):
        import struct
        (self.folder / 'mouse-options.ini').write_text('[highlight]\nglow=1\n', encoding='utf-16')
        overlay = self.launch()
        for monitor, dpi, x, y, expected in (
            (0x10001, 96, 600, 400, (563, 363, 637, 437)),
            (0x10002, 144, -700, 300, (-755, 245, -645, 355)),
            (0x10002, 144, -1, 300, (-56, 245, 54, 355)),
            (0x10001, 96, 1, 300, (-36, 263, 38, 337)),
        ):
            replace_windows_import(self.process, self.exe, 'MonitorFromPoint', b'\x48\xb8' + struct.pack('<Q', monitor) + b'\xc3')
            replay_dpi(self.process, self.exe, dpi)
            replay_cursor(self.process, self.exe, x, y)
            wait_for(lambda: bounds(overlay) == expected, f'Wrong glow bounds at monitor={monitor}, DPI={dpi}')
        # Replay the public display-change notification to this process's windows only.
        callback_type = C.WINFUNCTYPE(W.BOOL, W.HWND, W.LPARAM)
        u.GetWindowThreadProcessId.argtypes = [W.HWND, C.POINTER(W.DWORD)]
        def notify(hwnd, _):
            pid = W.DWORD()
            u.GetWindowThreadProcessId(hwnd, C.byref(pid))
            if pid.value == self.process.pid:
                u.PostMessageW(hwnd, 0x007E, 32, (1080 << 16) | 1920)
            return True
        callback = callback_type(notify)
        u.EnumWindows.argtypes = [callback_type, W.LPARAM]
        u.EnumWindows(callback, 0)
        time.sleep(.1)
        self.assertIsNone(self.process.poll())
        self.assertEqual(bounds(overlay), (-36, 263, 38, 337))

    def test_startup_rolls_back_when_config_cannot_be_saved(self):
        read = preserve_startup_value(self)
        previous = read()
        # This isolated case starts with no startup command; restore the user value later.
        if previous:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r'Software\Microsoft\Windows\CurrentVersion\Run', 0, winreg.KEY_SET_VALUE) as key:
                winreg.DeleteValue(key, 'MouseOptions')
        path = self.folder / 'mouse-options.ini'
        path.write_text('[highlight]\nstartup=0\n', encoding='utf-16')
        kernel.SetFileAttributesW.argtypes = [W.LPCWSTR, W.DWORD]
        self.assertTrue(kernel.SetFileAttributesW(str(path), 1))
        self.addCleanup(lambda: kernel.SetFileAttributesW(str(path), 0x80))
        self.launch('--settings')
        settings = settings_window()
        startup = button(settings, 'Start when I sign in')
        click(startup)
        wait_for(lambda: any('Cannot save.' in value for value in child_texts(settings)), 'Save failure missing')
        self.assertEqual(u.SendMessageW(startup, 0x00F0, 0, 0), 0)
        self.assertIsNone(read())

if __name__ == '__main__':
    unittest.main(verbosity=2)
