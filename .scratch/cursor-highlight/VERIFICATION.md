# Cursor highlight — Audit nghiệm thu

2026-11-10. Implementation đủ 6 issue. **27/34 tiêu chí đã có bằng chứng;
7 tiêu chí còn chờ phần cứng/logon thật. Goal chưa hoàn thành.**

Người dùng xác nhận hiện chỉ có một màn hình và yêu cầu để các kiểm chứng thực tế
này chờ. Giữ nguyên 7 tiêu chí chưa tick; không coi trì hoãn là đã đạt hoặc bỏ yêu cầu.

## Bằng chứng

- [22 test đạt, không skip](../../docs/evidence/integration-summary.json), [log](../../docs/evidence/integration-tests.txt).
- [Manifest ZIP, hash, dependency](../../docs/evidence/package.json): Release x64 /MT,
  executable 190,464 byte; ZIP khớp binary test, không chứa cấu hình cá nhân.
- [RAM/CPU/GDI/USER/handles và checkpoint chu kỳ](../../docs/evidence/resources.json).
- [TDD, seam đã duyệt, UI inspection và giới hạn mô phỏng](../../docs/testing.md).
- Máy thật: Windows 11 build 26200, x64, **một màn hình, 144 DPI / 150%**.

Mã tiêu chí dưới đây theo thứ tự checkbox trong issue. Bằng chứng ghi rõ test runtime,
kiểm tra mã hoặc mô phỏng OS; không dùng mô phỏng để tuyên bố đã gắn hai màn hình
hoặc đã đăng nhập lại Windows.

| Tiêu chí | Kết quả | Bằng chứng / phần còn thiếu |
| --- | --- | --- |
| 01.1 Build và portable không cần SDK | Đạt | CMake /MT; x64 PE; chỉ import 7 DLL hệ thống; ZIP khớp executable đã test |
| 01.2 Highlight liên tục theo chuột | Đạt | test_highlight_follows_cursor_without_taking_focus; không cần click |
| 01.3 Giữ focus, không che cursor/chặn input | Đạt | Test focus, pixel và canvas nhận click/drag; app không thay/hide cursor; UI inspection |
| 01.4 Khay, toggle, Exit, dọn tài nguyên | Đạt | Kiểm tra chuỗi NotifyIcon v4 → menu → command trong mã; UI Exit thật; toggle/GDI release test; process exit 0 |
| 01.5 Tương tác và baseline tài nguyên | Đạt | Integration suite và resource states fresh/off/moving; Windows timer, không busy loop |
| 02.1 Mở/đóng cài đặt không nhân đôi hay thoát nền | Đạt | Test close/restart, singleton activation, 30 vòng mở/đóng |
| 02.2 Filled / ring, tâm ring rỗng | Đạt | Test pixel trên canvas trắng: tâm rỗng, outline đúng màu |
| 02.3 Điều khiển hình thức tác động ngay | Đạt | Appearance round-trip, live recolor, size/thickness độc lập |
| 02.4 Diameter loại glow, viền hợp lệ | Đạt | Bounds core/halo; UI thickness độc lập; max round-trip 256/127 và clamp trong mã |
| 02.5 Tự lưu, khôi phục, cấu hình hỏng | Đạt | Ring/glow/appearance round-trip và corrupt config; save atomic |
| 02.6 Kiểm chứng cấu hình/tài nguyên lặp | Đạt | Config hỏng/chỉ đọc; resource checkpoints; native controls/font được destroy khi đóng |
| 03.1 Mọi màn hình, tọa độ âm | Chờ | GetCursorPos/MonitorFromPoint replay đạt; thiếu hai màn hình thật |
| 03.2 Scale 100%/150% cho đường kính/viền | Chờ | Replay 96/144 DPI đạt; máy thật mới kiểm chứng 144 DPI |
| 03.3 Ranh giới không mất/cắt/để ảnh cũ | Chờ | Bounds replay đạt; thiếu quan sát render tại đường nối thật |
| 03.4 Đổi scale/primary/ngắt-kết nối | Chờ | Thông báo topology OS replay không crash; chưa thay topology phần cứng |
| 03.5 Ma trận và số đo nhiều màn hình | Chờ | Hiện chỉ có ma trận/số đo một màn hình |
| 04.1 Glow mặc định tắt và tự lưu | Đạt | test_glow_expands_halo_and_survives_restart |
| 04.2 Glow đúng màu/kiểu/kích thước/opacity | Đạt | Pixel halo mềm, live red → green, appearance tests; renderer dùng config hiện tại |
| 04.3 Không đổi diameter, chặn chuột/lấy focus | Đạt | Bounds core/halo, external input/focus; cùng overlay no-activate/click-through |
| 04.4 Glow qua cạnh màn hình/DPI thật | Chờ | Replay 100%/150%, cạnh x=-1/1 đạt; thiếu hai màn hình vật lý |
| 04.5 Tái sử dụng bitmap, chọn renderer theo đo | Đạt | Cached premultiplied CPU bitmap; chỉ tái tạo khi appearance/DPI đổi; resource evidence |
| 04.6 Đo glow bật/tắt, idle/moving/lặp | Đạt | Resource JSON, 30 vòng UI/style/glow/hotkey, không trim working set |
| 05.1 Xem/đổi/lưu/đăng ký shortcut | Đạt | Hotkey control + Apply; test đổi và mở lại |
| 05.2 Toggle từ app khác, giữ focus | Đạt | test_global_hotkey_toggles_without_opening_settings |
| 05.3 Chung trạng thái, dừng cập nhật khi tắt | Đạt | Shared setEnabled; gỡ timer, giải phóng bitmap; test GDI giảm |
| 05.4 Trùng/invalid phím, giữ tổ hợp cũ | Đạt | Chiếm Ctrl+Alt+J, lỗi unavailable, Ctrl+Alt+H vẫn hoạt động; validation trong applyHotkey |
| 05.5 Gỡ đăng ký, thoát, không repeat | Đạt | Register mới trước rồi unregister cũ; held-key test; graceful exit |
| 05.6 Thử shortcut, đổi nhiều lần/tài nguyên | Đạt | Conflict/restart tests; 30 thay đổi shortcut trong resource cycles |
| 06.1 Startup mặc định tắt, không tự bật | Đạt | Default checkbox test; tắt/gỡ Run; launch mặc định không bật mới |
| 06.2 HKCU và đường dẫn có khoảng trắng | Đạt | Actual Run query; quoted executable + --background; asInvoker manifest |
| 06.3 Gỡ startup và báo lỗi/rollback thật | Đạt | Actual remove; OS denied fixture; read-only config rollback test |
| 06.4 Nền/cấu hình/singleton | Đạt | --background thật; cấu hình lưu; secondary --settings exits 0 và mở UI hiện tại |
| 06.5 Di chuyển portable sửa đăng ký | Đạt | test_moved_portable_repairs_startup_registration; hướng dẫn README |
| 06.6 Bật/login/tắt/login thật | Chờ | Run/direct invocation/singleton đã test; chưa đăng xuất phiên người dùng |

## Đóng các tiêu chí còn chờ

Thực hiện [checklist màn hình/logon](../../docs/manual-checks.md), ghi cấu hình,
kết quả từng bước, ảnh/video và số đo nhiều màn hình. Khi có bằng chứng thật,
cập nhật 03.1–03.5, 04.4, 06.6 và trạng thái nghiệm thu tương ứng.

Giữ nguyên mục tiêu đủ 6 issue. Không tự đổi thành chỉ cần test mô phỏng.
Gói hiện tại chỉ nhắm Windows 11 x64, desktop và ứng dụng cửa sổ thông thường.
