# 01 — Cursor highlight cơ bản từ khay hệ thống

Status: ready-for-agent
Execution: complete
Implementation: complete
Verification: complete

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Một ứng dụng portable native chạy được trên Windows 11: khi mở, filled highlight mặc định liên tục đi theo con trỏ trên màn hình chính. Người dùng có thể bật/tắt highlight và thoát từ menu khay hệ thống. Đây là đường chạy đầu tiên hoàn chỉnh, gồm cách build và gói chạy thử; không phải issue chỉ dựng skeleton.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [x] Có hướng dẫn build Release, xác định kiến trúc CPU đích và gói portable giải nén rồi chạy được mà không cần trình cài đặt hoặc môi trường phát triển.
- [x] Filled highlight bán trong suốt theo đúng điểm con trỏ trên màn hình chính, không chỉ xuất hiện khi click.
- [x] Highlight không nhận focus, không che mất hình con trỏ và không cản click hoặc kéo thả ứng dụng bên dưới.
- [x] Biểu tượng khay cung cấp bật/tắt highlight và “Thoát”; thoát gỡ overlay và biểu tượng, dọn tài nguyên.
- [x] Ghi bằng chứng chạy thử tương tác và số đo tài nguyên nền ban đầu khi highlight bật/tắt; không dùng vòng lặp bận để theo dõi chuột.

## Blocked by

None - can start immediately.

## Evidence

Release x64 /MT; ZIP khớp binary đã chạy 22 test. Pointer/focus, pixel alpha, click/kéo thả, giải phóng tài nguyên đã kiểm chứng. Menu khay được kiểm tra trong mã Win32; Exit đã chạy qua UI thật.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Acceptance criteria đã được kiểm chứng; Execution: complete.

