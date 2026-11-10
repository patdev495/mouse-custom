---
status: accepted
---

# Ứng dụng Windows native với C++ và Win32

Ứng dụng chỉ nhắm Windows 11 và ưu tiên mức sử dụng RAM thấp nhất có thể. Chọn C++ với Win32 và các thành phần cài đặt native để kiểm soát tài nguyên trực tiếp; đánh đổi là phải tự xử lý nhiều chi tiết giao diện, vòng đời cửa sổ và DPI.

Rust với Win32 cũng là lựa chọn native hợp lệ; không có kết luận rằng C++ luôn dùng ít RAM hơn Rust. Các framework giao diện cấp cao giúp phát triển giao diện thuận tiện hơn nhưng thêm chi phí nền cần đo. Chọn renderer sau khi đo bản thử thay vì mặc định dùng GPU. Người dùng chấp nhận thiết kế tổng hợp ngày 2026-11-10.
