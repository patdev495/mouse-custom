# 04 — Glow tùy chọn cho cả hai kiểu highlight

Status: needs-info
Execution: verification-pending
Implementation: complete
Verification: pending-physical-multi-monitor

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Thêm lựa chọn glow vào cài đặt, mặc định tắt. Bật tạo quầng sáng mềm theo màu highlight cho cả filled highlight và ring highlight, áp dụng ngay và tự lưu. Bản đầu dùng thông số glow cố định, không thêm thanh chỉnh hoặc animation mới.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [x] Mặc định glow tắt; bật/tắt có tác dụng ngay và được giữ sau khi mở lại ứng dụng.
- [x] Glow theo màu highlight, đổi đúng khi thay đổi màu, độ trong suốt, kiểu hoặc kích thước.
- [x] Glow không thay đổi ý nghĩa highlight diameter và không chặn thao tác chuột hay lấy focus.
- [ ] Glow không bị cắt sai ở các cạnh màn hình; hiển thị đúng khi chuyển qua màn hình có DPI khác nhau.
- [x] Tái sử dụng tài nguyên vẽ khi chỉ di chuyển chuột; cập nhật tài nguyên khi hình thức thay đổi. Chọn cách vẽ dựa trên số đo và hành vi quan sát được, không mặc định buộc dùng Direct2D.
- [x] So sánh số đo Release trong cùng môi trường giữa glow tắt/bật, đứng yên/di chuyển và lặp bật/tắt để kiểm tra tăng tài nguyên liên tục; ghi lại chi phí và quyết định tối ưu.

## Blocked by

- [#02 — Tùy chỉnh và lưu filled highlight / ring highlight](02-highlight-settings.md)
- [#03 — Cursor highlight trên nhiều màn hình và DPI](03-multi-monitor-dpi.md)

## Evidence

Glow mặc định tắt, tự lưu; pixel quầng mềm đúng màu, recolor trực tiếp, tâm ring rỗng; cache bitmap và số đo bật/tắt đã có. Qua monitor/DPI mới kiểm chứng bằng OS replay; chờ màn hình thật.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Nghiệm thu còn chờ môi trường thật; chưa hoàn thành issue.

