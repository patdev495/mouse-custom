# Cursor highlight cho Windows desktop

Status: needs-info

## Yêu cầu đã biết

- Ứng dụng desktop Windows để tùy chỉnh hành vi con trỏ chuột.
- Bản đầu chỉ hỗ trợ Windows 11.
- Highlight đi theo con trỏ qua mọi màn hình, hỗ trợ các màn hình có mức scale khác nhau và giữ kích thước hiển thị nhất quán theo DPI.
- Ưu tiên sử dụng ít RAM nhất có thể.
- Không đặt ngưỡng RAM cố định theo MB; tối ưu mức sử dụng RAM và đo thực tế trên bản thử.
- Chức năng đầu tiên: cursor highlight, tùy chỉnh màu sắc, độ rộng vùng hiển thị và độ trong suốt.
- Khi bật, highlight hiển thị liên tục và đi theo con trỏ chuột, không cần nhấp chuột để xuất hiện.
- Hỗ trợ hai kiểu hình thức, chọn trong cài đặt: hình tròn tô màu và vòng tròn chỉ có viền với phần giữa rỗng.
- Có hai thanh chỉnh riêng: đường kính toàn vùng highlight và độ dày viền khi dùng ring highlight.
- Có hiệu ứng glow (quầng sáng mềm quanh highlight), bật/tắt được và mặc định tắt.
- Bản đầu không có hiệu ứng chuyển động theo nhịp hoặc đổi màu liên tục.
- Ứng dụng chạy nền với biểu tượng ở khay hệ thống cạnh đồng hồ; bấm biểu tượng để mở cài đặt.
- Đóng cửa sổ cài đặt không thoát ứng dụng và highlight vẫn chạy; chọn “Thoát” để tắt ứng dụng.
- Có phím tắt toàn hệ thống để bật/tắt highlight, cho phép đổi phím tắt trong cài đặt; menu khay hệ thống cũng có nút bật/tắt.
- Có tùy chọn tự chạy khi đăng nhập Windows, mặc định tắt.
- Bản đầu dạng portable: giải nén rồi chạy `.exe`, không cần trình cài đặt.

## Đề xuất đang thảo luận

- C++ với Win32; giao diện cài đặt dùng thành phần Windows native, ưu tiên ít RAM. Xem ADR đề xuất `docs/adr/0001-native-windows-app.md`.
- Chỉnh cài đặt có tác dụng ngay và tự lưu cho lần mở sau.
- Màu và độ trong suốt áp dụng cho cả hai kiểu highlight; glow theo màu highlight. Bản đầu dùng thông số glow cố định, chỉ có bật/tắt.
- Overlay không chặn thao tác chuột và không lấy focus của ứng dụng đang dùng.
- Tối ưu vùng vẽ và tái sử dụng tài nguyên; chọn cách vẽ dựa trên kết quả đo bản thử, chưa bắt buộc Direct2D.
- Đo working set và private bytes của bản Release ở cùng cấu hình màn hình, sau khi khởi động ổn định: highlight tắt, highlight bật với glow tắt, glow bật, và cửa sổ cài đặt mở. Ghi thêm CPU khi đứng yên và khi di chuyển chuột để đánh giá chi phí vận hành.
- Phạm vi bản đầu là desktop và ứng dụng cửa sổ thông thường; chưa cam kết overlay trên secure desktop hoặc game fullscreen độc quyền.

## Kiểm chứng dự kiến

- Highlight theo vị trí con trỏ, đổi màn hình đúng tọa độ và kích thước theo DPI, không ngăn click hoặc kéo thả.
- Hai kiểu highlight, màu, độ trong suốt, đường kính, độ dày viền và bật/tắt glow hoạt động đúng.
- Đóng cài đặt vẫn chạy nền; bật/tắt từ khay và phím tắt hoạt động; thoát dọn tài nguyên.
- Cài đặt được khôi phục sau khi thoát và mở lại. Nếu phím tắt bị trùng, thông báo để người dùng đổi.
- Tùy chọn tự chạy khi đăng nhập mặc định tắt; bật/tắt tác động đúng cho tài khoản hiện tại.
- Bản portable chạy được trên Windows 11 trong phạm vi kiến trúc CPU được đóng gói.
- Ghi nhận số đo tài nguyên, không dùng ngưỡng MB tùy ý làm tiêu chí đạt.

## Cần xác nhận

- Người dùng duyệt bản thiết kế tổng hợp và các đề xuất bổ sung trước khi triển khai theo skill grilling.
- Chưa chốt kiến trúc CPU của gói phát hành; xác định từ máy đích trước khi build.

## Comments

- 2026-11-10: Bắt đầu grill-with-docs. Chỉ làm rõ yêu cầu và ghi tài liệu; chờ đạt thống nhất trước khi triển khai ứng dụng.
- 2026-11-10: Người dùng chọn highlight luôn đi theo con trỏ (lựa chọn 1), thay vì chỉ xuất hiện khi nhấp chuột.
- 2026-11-10: Người dùng chọn hỗ trợ cả hình tròn tô màu và vòng tròn chỉ có viền, đổi kiểu trong cài đặt (lựa chọn 3).
- 2026-11-10: Người dùng chọn thêm glow bật/tắt được, mặc định tắt (lựa chọn 2); chưa đưa hiệu ứng chuyển động liên tục vào bản đầu.
- 2026-11-10: Người dùng chọn hai thanh riêng cho đường kính highlight và độ dày viền (lựa chọn 1).
- 2026-11-10: Người dùng chọn chạy nền ở khay hệ thống; đóng cài đặt vẫn chạy, chọn “Thoát” mới tắt ứng dụng (lựa chọn 1).
- 2026-11-10: Người dùng chọn bản đầu chỉ hỗ trợ Windows 11 (lựa chọn 1).
- 2026-11-10: Người dùng chọn hỗ trợ mọi màn hình, kể cả khi mức scale khác nhau như 100% và 150% (lựa chọn 1).
- 2026-11-10: Người dùng chọn cả phím tắt toàn hệ thống có thể tùy chỉnh và nút bật/tắt trong menu khay hệ thống (lựa chọn 1).
- 2026-11-10: Người dùng không chọn ngưỡng 20 MB hoặc 50 MB; yêu cầu RAM nhỏ nhất có thể. Sẽ đo bản thử ở trạng thái chạy nền và khi bật glow.
- 2026-11-10: Người dùng chọn có tùy chọn tự chạy khi đăng nhập Windows, mặc định tắt (lựa chọn 1).
- 2026-11-10: Người dùng chọn bản đầu portable, giải nén rồi chạy `.exe` (lựa chọn 1).
- 2026-11-10: Tổng hợp thiết kế để duyệt; các mục trong “Đề xuất đang thảo luận” chưa được coi là quyết định đã chấp nhận.
