# Meteor trail

Status: ready-for-agent
Execution: verification-pending
Implementation: complete
Verification: user-testing-pending

## Yêu cầu đã chốt

- Chuột di chuyển sinh sao nhỏ trên đường vừa đi; sao rơi chéo xuống, có đuôi sáng, mờ dần.
- Bật/tắt riêng trong cài đặt, dùng cùng cursor highlight; mặc định tắt.
- Giữ cấu hình và hành vi highlight hiện có, phím tắt tổng bật/tắt cả hai hiệu ứng.
- Điều chỉnh riêng màu, mật độ, kích thước, tốc độ rơi và opacity của sao.
- Tự lưu/khôi phục cấu hình; cấu hình cũ chưa có meteor vẫn dùng bình thường.
- Không chặn click/kéo thả hoặc lấy focus; không sinh thêm sao khi con trỏ đứng yên.
- Tài nguyên hạt và vùng vẽ có giới hạn; giải phóng khi tắt hoặc khi mọi sao đã mờ hết.

## Kiểm chứng

Giữ ba seam người dùng đã duyệt: UI native, cấu hình công khai/restart, Windows integration/overlay. TDD từng hành vi, chạy lại regression suite. Đo RAM/CPU khi bật/tắt và di chuyển; không hứa số MB trước khi đo.

## Comments

- 2026-11-10: Người dùng chọn phương án 1, sao băng bật/tắt riêng và dùng cùng highlight. Kiểm chứng hai màn hình/logon của feature cũ tiếp tục để chờ theo yêu cầu trước đó.
- 2026-11-10: TDD đã chạy đỏ → xanh cho toggle/lưu, sao sinh-rơi-fade và options round-trip. Ba test mới đạt trước các thay đổi cuối về phiên bản/nhãn UI.
- 2026-11-10: Người dùng yêu cầu “code nốt đi, tao sẽ tự test”. Hoàn tất code/build/gói portable; phần kiểm chứng thực tế và đo tài nguyên bản mới để người dùng thực hiện.

## Bản đã triển khai

- Checkbox Meteor trail độc lập với Glow; Options mở cửa sổ native riêng, không kéo dài cửa sổ cài đặt chính.
- Màu sao riêng, density 5–100, size 2–8, speed 40–240, opacity 0–100; cấu hình cũ vẫn đọc được.
- Pool cố định 64 sao, bitmap tối đa 512×384×4 byte; loại sao cũ khi vượt vùng vẽ.
- Tái sử dụng bitmap giữa các frame; hủy window/DC/bitmap sau khi sao cuối hết tuổi hoặc khi tắt.
- Các setting highlight trước cập nhật được giữ nguyên; không phân phối INI cá nhân trong ZIP.
