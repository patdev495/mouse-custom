# 06 — Tùy chọn tự chạy khi đăng nhập Windows

Status: needs-info
Execution: verification-pending
Implementation: complete
Verification: pending-real-logon

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Thêm lựa chọn tự chạy khi đăng nhập Windows cho tài khoản hiện tại, mặc định tắt. Bật/tắt trong cài đặt tác động lên đăng ký tự chạy thực tế. Khi tự chạy, ứng dụng portable dùng cấu hình đã lưu và nằm ở khay hệ thống mà không mở cài đặt.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [x] Cài đặt có tùy chọn tự chạy khi đăng nhập, mặc định tắt trên lần dùng đầu tiên; không tự bật lại sau khi người dùng tắt.
- [x] Bật tạo đăng ký cho tài khoản hiện tại với đúng đường dẫn executable portable, xử lý được đường dẫn có khoảng trắng; không cần quyền quản trị.
- [x] Tắt gỡ đăng ký của ứng dụng. Thay đổi thất bại hiển thị lỗi và không báo trạng thái thành công giả.
- [x] Đăng nhập lại khởi chạy ứng dụng với hình thức đã lưu và chỉ hiện biểu tượng khay; mở thủ công khi đã chạy không tạo nhiều overlay hoặc biểu tượng trùng.
- [x] Khi người dùng di chuyển thư mục portable, mở ứng dụng ở vị trí mới phát hiện và cập nhật đăng ký cũ nếu tùy chọn tự chạy vẫn bật; mô tả việc cần mở lại sau khi di chuyển trong hướng dẫn portable.
- [ ] Kiểm chứng một chu kỳ bật → đăng nhập lại → tắt → đăng nhập lại, đường dẫn có khoảng trắng và trường hợp di chuyển thư mục. Kiểm tra khởi động nhiều lần không sinh nhiều tiến trình nền.

## Blocked by

- [#02 — Tùy chỉnh và lưu filled highlight / ring highlight](02-highlight-settings.md)

## Evidence

HKCU Run, quoting khoảng trắng, bật/tắt, lỗi quyền, rollback khi INI chỉ đọc, singleton, sửa đường dẫn portable đã kiểm chứng. Chưa đăng xuất/đăng nhập thực tế.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Nghiệm thu còn chờ môi trường thật; chưa hoàn thành issue.

