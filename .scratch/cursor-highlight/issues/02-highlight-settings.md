# 02 — Tùy chỉnh và lưu filled highlight / ring highlight

Status: ready-for-agent
Execution: complete
Implementation: complete
Verification: complete

Source: [PRD](../PRD.md)
Architecture: [ADR 0001](../../../docs/adr/0001-native-windows-app.md)
Glossary: [CONTEXT](../../../CONTEXT.md)

## What to build

Mở cửa sổ cài đặt native từ biểu tượng khay để chọn filled highlight hoặc ring highlight; chỉnh màu, độ trong suốt, highlight diameter và ring thickness. Thay đổi tác động ngay lên highlight và tự lưu cho lần chạy sau. Đóng cài đặt trả ứng dụng về chạy nền.

### Ràng buộc chung

- Dùng C++ với Win32 và giao diện Windows native theo ADR đã chấp nhận; phạm vi Windows 11.
- Giữ bản portable chạy được sau khi hoàn thành issue. Không yêu cầu trình cài đặt hoặc môi trường phát triển trên máy sử dụng.
- Ưu tiên giảm RAM; dùng cùng môi trường và bản Release để ghi working set, private bytes và CPU lúc đứng yên/di chuyển trong các trạng thái liên quan đến issue. Ghi rõ cấu hình màn hình, không đặt ngưỡng MB tùy ý hoặc trim working set để làm đẹp số đo.
- Phạm vi overlay là desktop và ứng dụng cửa sổ thông thường, chưa cam kết secure desktop hoặc game fullscreen độc quyền.
- Không thêm animation theo nhịp hoặc đổi màu liên tục. Dùng glossary chung cho các thuật ngữ.
- Có bằng chứng kiểm chứng phù hợp với acceptance criteria; phân biệt rõ phần đã chạy thực tế với phần chưa có môi trường để kiểm tra.

## Acceptance criteria

- [x] Bấm biểu tượng khay mở cài đặt; mở lại không tạo nhiều cửa sổ cài đặt trùng nhau. Đóng cài đặt vẫn giữ highlight và biểu tượng khay hoạt động.
- [x] Hai kiểu hiển thị đúng: filled highlight tô vùng tròn, ring highlight chỉ có viền và phần giữa rỗng.
- [x] Có điều khiển màu, độ trong suốt, highlight diameter và thanh ring thickness riêng; thay đổi hiển thị ngay mà không cần khởi động lại.
- [x] Highlight diameter không tính glow. Ring thickness có giới hạn hợp lệ tương ứng với đường kính; kiểm tra dữ liệu nhập không tạo kích thước vô lý.
- [x] Cài đặt tự lưu và khôi phục sau khi thoát/mở lại; cấu hình thiếu hoặc hỏng có giá trị mặc định hợp lệ để ứng dụng vẫn mở được.
- [x] Kiểm chứng đổi kiểu/chỉnh giá trị, lưu/đọc và trường hợp cấu hình hỏng. Đo tài nguyên khi mở/đóng cài đặt nhiều lần để phát hiện giữ tài nguyên không cần thiết hoặc tăng bộ nhớ liên tục.

## Blocked by

- [#01 — Cursor highlight cơ bản từ khay hệ thống](01-basic-cursor-highlight.md)

## Evidence

Đổi/lưu/khôi phục hai kiểu, màu, kích thước, viền, opacity; cấu hình hỏng/chỉ đọc; 30 chu kỳ mở/đóng cài đặt. Slider có tên accessibility đúng.

Chi tiết: [audit nghiệm thu](../VERIFICATION.md), [test và số đo](../../../docs/testing.md).

## Comments

- 2026-11-10: Người dùng duyệt cách chia 6 issue và quan hệ phụ thuộc. Xuất bản vào tracker local; chỉ bắt đầu thực hiện khi các blocker đã hoàn thành.
- 2026-11-10: Implementation hoàn tất. Acceptance criteria đã được kiểm chứng; Execution: complete.

