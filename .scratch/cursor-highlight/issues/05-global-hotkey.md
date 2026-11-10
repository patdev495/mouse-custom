# 05 — Bật/tắt cursor highlight bằng phím tắt toàn hệ thống

Status: ready-for-agent
Execution: complete
Implementation: complete
Verification: complete

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Người dùng đổi và lưu phím tắt trong cài đặt, rồi dùng tổ hợp đó để bật/tắt cursor highlight khi đang làm việc trong ứng dụng khác. Trạng thái hiển thị nhất quán với menu khay hệ thống.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [x] Có điều khiển xem/đổi phím tắt, lưu lựa chọn cho lần chạy sau và đăng ký phím tắt trong phiên hiện tại.
- [x] Phím tắt bật/tắt highlight khi ứng dụng khác có focus, không mở cài đặt hoặc lấy focus ngoài ý muốn.
- [x] Bật/tắt từ phím tắt và từ menu khay cùng cập nhật một trạng thái; tắt highlight dừng các cập nhật hiển thị không cần thiết.
- [x] Phím tắt không hợp lệ hoặc đã bị chiếm có thông báo rõ; không âm thầm lưu như đã đăng ký thành công. Nếu đổi thất bại, giữ phím tắt cũ nếu còn dùng được và luôn giữ cách bật/tắt từ khay.
- [x] Đổi phím tắt hoặc thoát ứng dụng giải phóng đăng ký cũ; giữ phím không khiến trạng thái đảo liên tục ngoài ý muốn.
- [x] Kiểm chứng khi ứng dụng khác có focus, khi trùng phím và sau khi mở lại; kiểm tra việc đăng ký/đổi phím nhiều lần không làm tăng tài nguyên liên tục.

## Blocked by

- [#02 — Tùy chỉnh và lưu filled highlight / ring highlight](02-highlight-settings.md)

## Evidence

Input Windows: toggle giữ focus, trùng phím giữ tổ hợp cũ, đổi/lưu/mở lại, MOD_NOREPEAT. Resource cycles gồm 30 thay đổi hotkey.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Acceptance criteria đã được kiểm chứng; Execution: complete.

