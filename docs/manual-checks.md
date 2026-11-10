# Kiểm chứng bản portable trên Windows 11

## Sao băng — bản 0.2.0

1. Bật **Meteor trail**, di chuyển chuột: sao phải xuất hiện trên đường vừa đi,
   rơi chéo và kéo đuôi sáng. Dừng chuột: sao cũ rơi tiếp, mờ hết trong khoảng một giây.
2. Ring/Filled và Glow vẫn dùng đồng thời. Tắt Meteor trail phải giữ highlight cũ.
   **Ctrl+Alt+H** và **Effects enabled** bật/tắt tổng cả hai hiệu ứng.
3. Mở **Options...** cạnh Meteor trail; thử màu, Density, Star size, Fall speed,
   Opacity. Màu sao độc lập với màu highlight; chỉnh có tác dụng ngay. Đóng/mở app
   để kiểm tra lưu cấu hình. Opacity 0 không hiển thị sao.
4. Click và kéo thả ứng dụng bên dưới khi sao đang rơi. Thử chạy chuột nhanh,
   rồi để yên; RAM không được tăng liên tục, sao không che thao tác hoặc lấy focus.

Người dùng nhận phần test thực tế bản 0.2.0. Ba test mới cho toggle/persistence,
options round-trip và rơi/fade đã đạt trong lúc phát triển; regression/performance
của gói cuối chưa chạy lại sau yêu cầu chuyển phần test cho người dùng.

## Bản nền 0.1.0

Đã có test tự động cho UI native, cấu hình/lưu/khôi phục, pixel màu/alpha/ring/glow,
click xuyên, phím tắt/trùng phím/giữ phím, singleton, đăng ký startup và đổi đường dẫn.
DPI 100%↔150%, tọa độ âm và thay đổi topology có fixture tại API Windows;
đó là mô phỏng phản hồi OS, không thay thế kiểm chứng hai màn hình thật.

## Hai màn hình thật — còn chờ

1. Dùng Windows 11 x64 với hai màn hình, một ở scale 100%, một ở 150%.
   Đặt một màn hình bên trái hoặc phía trên màn hình chính để có tọa độ âm.
2. Chạy ứng dụng, mở cài đặt từ biểu tượng khay. Thử Filled rồi Ring, Size 48
   và một kích thước lớn hơn; thử viền mảnh/dày. Ghi màn hình và mức scale đã dùng.
3. Di chuyển chuột qua đường nối và bốn góc mỗi màn hình. Highlight phải đúng tâm,
   không để lại ảnh cũ hoặc biến mất. Size 48 tương ứng vùng tròn chính 48 px ở 100%
   và 72 px ở 150%; glow không được tính vào kích thước này.
4. Bật Glow, đổi màu và opacity, lặp lại bước 3 với cả Filled và Ring. Quầng sáng
   phải chuyển màn hình đúng vị trí, không bị ứng dụng tự cắt ở ranh giới.
5. Đổi scale, đổi màn hình chính, ngắt/kết nối lại màn hình khi ứng dụng đang chạy.
   Highlight phải phục hồi đúng, ứng dụng không crash, cài đặt vẫn mở được.
6. Ghi kết quả, lỗi nếu có, cấu hình màn hình và ảnh/video minh họa. Đo tài nguyên
   trong cấu hình hai màn hình nếu có môi trường build/test.

## Đăng nhập Windows thực tế — còn chờ

Chỉ người dùng chủ động thực hiện khi đã lưu công việc; agent không tự đăng xuất
phiên làm việc đang dùng.

1. Đặt portable trong thư mục cố định, có thể có khoảng trắng. Chỉnh màu/kiểu,
   bật **Start when I sign in**, đóng cài đặt. Nếu Windows Startup Apps đã vô hiệu
   hóa MouseOptions trước đó, kiểm tra trạng thái tại đó.
2. Đăng xuất rồi đăng nhập lại. Xác nhận đúng một tiến trình/biểu tượng khay,
   highlight dùng cấu hình đã lưu, không tự mở cửa sổ cài đặt.
3. Mở ứng dụng lần nữa: phải mở cài đặt của tiến trình hiện tại, không nhân đôi highlight.
4. Tắt **Start when I sign in**, đăng xuất/đăng nhập lại. Ứng dụng không được tự chạy.
5. Ghi kết quả hai lần đăng nhập và cấu hình Windows. Việc bật/tắt Run và chạy lệnh
   startup trực tiếp đã có test; bước này kiểm chứng Windows thực sự thực thi ở logon.
