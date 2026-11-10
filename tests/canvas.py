"""A white external window: verifies actual pixels and click-through at the UI seam."""
import ctypes as C
from ctypes import wintypes as W
import os

u = C.WinDLL('user32', use_last_error=True)
g = C.WinDLL('gdi32', use_last_error=True)
k = C.WinDLL('kernel32', use_last_error=True)
PROC = C.WINFUNCTYPE(C.c_ssize_t, W.HWND, W.UINT, W.WPARAM, W.LPARAM)

class WNDCLASS(C.Structure):
    _fields_ = [('style', W.UINT), ('proc', PROC), ('clsExtra', C.c_int), ('wndExtra', C.c_int),
                ('instance', W.HINSTANCE), ('icon', W.HICON), ('cursor', W.HANDLE),
                ('background', W.HBRUSH), ('menu', W.LPCWSTR), ('name', W.LPCWSTR)]

u.DefWindowProcW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.DefWindowProcW.restype = C.c_ssize_t
u.RegisterClassW.argtypes = [C.POINTER(WNDCLASS)]
u.CreateWindowExW.argtypes = [W.DWORD, W.LPCWSTR, W.LPCWSTR, W.DWORD, C.c_int, C.c_int,
                             C.c_int, C.c_int, W.HWND, W.HMENU, W.HINSTANCE, C.c_void_p]
u.CreateWindowExW.restype = W.HWND
u.DestroyWindow.argtypes = [W.HWND]
u.UpdateWindow.argtypes = [W.HWND]
u.SetForegroundWindow.argtypes = [W.HWND]
u.GetDC.argtypes = [W.HWND]
u.GetDC.restype = W.HDC
u.ReleaseDC.argtypes = [W.HWND, W.HDC]
u.UnregisterClassW.argtypes = [W.LPCWSTR, W.HINSTANCE]
u.LoadCursorW.argtypes = [W.HINSTANCE, C.c_void_p]
u.LoadCursorW.restype = W.HANDLE
u.PeekMessageW.argtypes = [C.POINTER(W.MSG), W.HWND, W.UINT, W.UINT, W.UINT]
u.TranslateMessage.argtypes = [C.POINTER(W.MSG)]
u.DispatchMessageW.argtypes = [C.POINTER(W.MSG)]
u.DispatchMessageW.restype = C.c_ssize_t
g.GetStockObject.argtypes = [C.c_int]
g.GetStockObject.restype = W.HGDIOBJ
g.GetPixel.argtypes = [W.HDC, C.c_int, C.c_int]
g.GetPixel.restype = W.DWORD
k.GetModuleHandleW.argtypes = [W.LPCWSTR]
k.GetModuleHandleW.restype = W.HMODULE

def pump():
    message = W.MSG()
    while u.PeekMessageW(C.byref(message), None, 0, 0, 1):
        u.TranslateMessage(C.byref(message))
        u.DispatchMessageW(C.byref(message))

class Canvas:
    def __init__(self):
        self.clicks = 0
        self.received_drag = False
        def handler(hwnd, msg, wp, lp):
            if msg == 0x0201:
                self.clicks += 1
            if msg == 0x0200 and wp & 1:
                self.received_drag = True
            return u.DefWindowProcW(hwnd, msg, wp, lp)
        self.proc = PROC(handler)
        self.name = 'MouseOptionsTestCanvas' + str(os.getpid())
        self.instance = k.GetModuleHandleW(None)
        cls = WNDCLASS(0, self.proc, 0, 0, self.instance, None, u.LoadCursorW(None, C.c_void_p(32512)), g.GetStockObject(0), None, self.name)
        if not u.RegisterClassW(C.byref(cls)):
            raise C.WinError(C.get_last_error())
        self.hwnd = u.CreateWindowExW(0x00000008, self.name, 'External test canvas', 0x90000000,
                                      100, 100, 1000, 700, None, None, self.instance, None)
        if not self.hwnd:
            raise C.WinError(C.get_last_error())
        u.UpdateWindow(self.hwnd)
        u.SetForegroundWindow(self.hwnd)
        pump()

    def close(self):
        u.DestroyWindow(self.hwnd)
        u.UnregisterClassW(self.name, self.instance)

    def pixel(self, x, y):
        pump()
        dc = u.GetDC(None)
        try:
            color = g.GetPixel(dc, x, y)
            if color == 0xFFFFFFFF:
                raise AssertionError('Cannot capture desktop pixel')
            return color & 255, (color >> 8) & 255, (color >> 16) & 255
        finally:
            u.ReleaseDC(None, dc)
