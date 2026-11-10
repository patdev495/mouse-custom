"""System-boundary fixture for the test's own x64 child process only.

Replays Windows DPI responses on single-monitor CI without changing desktop settings.
Patches imported Windows functions, never application functions or private messages.
Hardware tests must still be reported separately from these simulated responses.
"""
import ctypes as C
from ctypes import wintypes as W
import struct

k = C.WinDLL('kernel32', use_last_error=True)
p = C.WinDLL('psapi', use_last_error=True)
k.OpenProcess.argtypes = [W.DWORD, W.BOOL, W.DWORD]
k.OpenProcess.restype = W.HANDLE
k.VirtualAllocEx.argtypes = [W.HANDLE, C.c_void_p, C.c_size_t, W.DWORD, W.DWORD]
k.VirtualAllocEx.restype = C.c_void_p
k.VirtualProtectEx.argtypes = [W.HANDLE, C.c_void_p, C.c_size_t, W.DWORD, C.POINTER(W.DWORD)]
k.WriteProcessMemory.argtypes = [W.HANDLE, C.c_void_p, C.c_void_p, C.c_size_t, C.POINTER(C.c_size_t)]
k.FlushInstructionCache.argtypes = [W.HANDLE, C.c_void_p, C.c_size_t]
k.CloseHandle.argtypes = [W.HANDLE]
p.EnumProcessModulesEx.argtypes = [W.HANDLE, C.c_void_p, W.DWORD, C.POINTER(W.DWORD), W.DWORD]

def replace_windows_import(process, exe, name, machine_code):
    data = exe.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3C)[0]
    sections, optional_size = struct.unpack_from('<H', data, pe + 6)[0], struct.unpack_from('<H', data, pe + 20)[0]
    optional = pe + 24
    if struct.unpack_from('<H', data, optional)[0] != 0x20B:
        raise AssertionError('DPI fixture supports x64 builds only')
    section_table = optional + optional_size

    def offset(rva):
        for index in range(sections):
            virtual_size, address, size, raw = struct.unpack_from('<IIII', data, section_table + 40 * index + 8)
            if address <= rva < address + max(size, virtual_size):
                return raw + rva - address
        raise AssertionError(f'RVA not mapped: {rva:x}')

    descriptor = offset(struct.unpack_from('<I', data, optional + 120)[0])
    imported_slot = None
    while any(data[descriptor:descriptor + 20]):
        lookup, _, _, _, thunk = struct.unpack_from('<IIIII', data, descriptor)
        if lookup:
            entry = offset(lookup)
            index = 0
            while (value := struct.unpack_from('<Q', data, entry + 8 * index)[0]):
                if not value >> 63:
                    start = offset(value) + 2
                    imported_name = data[start:data.index(b'\0', start)].decode('ascii')
                    if imported_name == name:
                        imported_slot = thunk + index * 8
                        break
                index += 1
        if imported_slot is not None:
            break
        descriptor += 20
    if imported_slot is None:
        raise AssertionError(f'Windows import not found: {name}')
    handle = k.OpenProcess(0x043A, False, process.pid)
    if not handle:
        raise C.WinError(C.get_last_error())
    try:
        modules = (C.c_void_p * 1024)()
        needed = W.DWORD()
        if not p.EnumProcessModulesEx(handle, modules, C.sizeof(modules), C.byref(needed), 3):
            raise C.WinError(C.get_last_error())
        target = k.VirtualAllocEx(handle, None, len(machine_code), 0x3000, 0x40)
        if not target:
            raise C.WinError(C.get_last_error())

        def write(address, payload):
            buffer = C.create_string_buffer(payload)
            count = C.c_size_t()
            if not k.WriteProcessMemory(handle, address, buffer, len(payload), C.byref(count)) or count.value != len(payload):
                raise C.WinError(C.get_last_error())

        write(target, machine_code)
        k.FlushInstructionCache(handle, target, len(machine_code))
        slot = modules[0] + imported_slot
        old = W.DWORD()
        if not k.VirtualProtectEx(handle, slot, 8, 0x04, C.byref(old)):
            raise C.WinError(C.get_last_error())
        try:
            write(slot, struct.pack('<Q', target))
        finally:
            discarded = W.DWORD()
            k.VirtualProtectEx(handle, slot, 8, old, C.byref(discarded))
    finally:
        k.CloseHandle(handle)

def replay_dpi(process, exe, dpi):
    # UINT GetDpiForWindow(HWND): return a fixed recorded Windows response.
    replace_windows_import(process, exe, 'GetDpiForWindow', b'\xb8' + struct.pack('<I', dpi) + b'\xc3')

def replay_cursor(process, exe, x, y):
    code = (b'\xc7\x01' + struct.pack('<i', x) + b'\xc7\x41\x04' + struct.pack('<i', y)
            + b'\xb8\x01\x00\x00\x00\xc3')
    replace_windows_import(process, exe, 'GetCursorPos', code)
