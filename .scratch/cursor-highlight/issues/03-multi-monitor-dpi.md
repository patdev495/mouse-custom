# 03 — Cursor highlight trên nhiều màn hình và DPI

Status: needs-info
Execution: verification-pending
Implementation: complete
Verification: pending-physical-multi-monitor

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Filled highlight và ring highlight đi theo con trỏ xuyên các màn hình Windows với các mức scale khác nhau. Cài đặt kích thước có cùng ý nghĩa theo đơn vị hiển thị logic trên mỗi màn hình; không bị lệch vị trí, cắt hình ở rìa màn hình hoặc mất highlight khi đổi màn hình.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [ ] Con trỏ và highlight khớp vị trí qua nhiều màn hình, gồm màn hình đặt bên trái hoặc phía trên màn hình chính với tọa độ âm.
- [ ] Highlight diameter và ring thickness được scale theo DPI màn hình hiện tại; kiểm chứng ít nhất tổ hợp 100% và 150%, không yêu cầu kích thước vật lý theo centimet bằng nhau.
- [ ] Đi qua ranh giới màn hình không làm mất highlight, để lại ảnh cũ hoặc khiến các phần của hình bị cắt sai.
- [ ] Thay đổi scale, thay đổi màn hình chính hoặc kết nối/ngắt màn hình trong lúc chạy không làm ứng dụng crash; cài đặt vẫn truy cập được.
- [ ] Kiểm chứng cả hai kiểu với một số kích thước khác nhau; ghi rõ ma trận màn hình đã thực sự kiểm tra và số đo tài nguyên trong cấu hình nhiều màn hình.

## Blocked by

- [#02 — Tùy chỉnh và lưu filled highlight / ring highlight](02-highlight-settings.md)

## Evidence

Test API Windows: 96/144 DPI, tọa độ âm, ranh giới, đổi monitor và topology. Máy thật chỉ có một màn hình 150%; chưa có ma trận hai màn hình và số đo tương ứng.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Nghiệm thu còn chờ môi trường thật; chưa hoàn thành issue.

