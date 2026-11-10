# Bản chia việc đề xuất: Cursor highlight

Đây là bản nháp để duyệt cách chia việc, chưa phải các issue đã xuất bản.

Thiết kế tổng hợp đã được người dùng chốt ngày 2026-11-10. Nguồn: [PRD](PRD.md), [glossary](../../CONTEXT.md), [ADR đã chấp nhận](../../docs/adr/0001-native-windows-app.md). Phần trạng thái chờ xác nhận trong PRD là nội dung lịch sử trước lần chốt này; không sửa tài liệu nguồn trong bước to-issues.

## Nguyên tắc chung

- Repo hiện chỉ có tài liệu; không có mã nguồn cần prefactor.
- Mỗi phần phải tạo ra hành vi người dùng có thể chạy và kiểm chứng, gồm giao diện/thao tác, trạng thái, hành vi thực tế và bằng chứng kiểm chứng phù hợp.
- C++ và Win32, giao diện cài đặt native, Windows 11. Xác định kiến trúc CPU máy đích trước khi build; không mặc định cam kết hỗ trợ mọi kiến trúc.
- Bản portable chạy được từ phần 01 và được cập nhật ở mọi phần; không tách build hoặc kiểm thử thành issue ngang riêng.
- Giữ mục tiêu giảm RAM xuyên suốt. Dùng cùng môi trường và bản Release để ghi working set, private bytes, CPU lúc đứng yên và lúc di chuyển; mô tả cấu hình màn hình và điều kiện đo. Không đặt ngưỡng MB hoặc ép working set xuống bằng thao tác trim để làm đẹp số đo.
- Không cam kết overlay trên secure desktop hoặc game fullscreen độc quyền.
- Không thêm hiệu ứng chuyển động theo nhịp hoặc đổi màu liên tục.
- Khi được duyệt, xuất bản theo thứ tự dưới đây vào tracker local với `Status: ready-for-agent`. Các số 01–06 dưới đây chỉ là mã đề xuất cho đến lúc xuất bản.

## Tóm tắt phụ thuộc

| Mã đề xuất | Phần việc | Blocked by | Hành vi người dùng nhận được |
| --- | --- | --- | --- |
| 01 | Cursor highlight cơ bản từ khay hệ thống | Không | Chạy portable, thấy filled highlight theo chuột, bật/tắt và thoát từ khay |
| 02 | Tùy chỉnh và lưu filled highlight / ring highlight | 01 | Chọn kiểu, chỉnh hình thức tức thời và giữ cài đặt sau khi mở lại |
| 03 | Cursor highlight trên nhiều màn hình và DPI | 02 | Di chuyển qua mọi màn hình, giữ đúng vị trí và kích thước theo scale |
| 04 | Glow tùy chọn cho cả hai kiểu highlight | 02, 03 | Bật/tắt glow, lưu lựa chọn, dùng được xuyên các màn hình |
| 05 | Bật/tắt cursor highlight bằng phím tắt toàn hệ thống | 02 | Đổi phím tắt, dùng khi ứng dụng khác có focus, xử lý trùng phím |
| 06 | Tùy chọn tự chạy khi đăng nhập Windows | 02 | Bật/tắt tự chạy cho tài khoản hiện tại, mặc định tắt |

## 01 — Cursor highlight cơ bản từ khay hệ thống

### What to build

Một ứng dụng portable native chạy được trên Windows 11: khi mở, filled highlight mặc định liên tục đi theo con trỏ trên màn hình chính. Người dùng có thể bật/tắt highlight và thoát từ menu khay hệ thống. Đây là đường chạy đầu tiên hoàn chỉnh, gồm cách build và gói chạy thử; không phải issue chỉ dựng skeleton.

### Acceptance criteria

- [ ] Có hướng dẫn build Release, xác định kiến trúc CPU đích và gói portable giải nén rồi chạy được mà không cần trình cài đặt hoặc môi trường phát triển.
- [ ] Filled highlight bán trong suốt theo đúng điểm con trỏ trên màn hình chính, không chỉ xuất hiện khi click.
- [ ] Highlight không nhận focus, không che mất hình con trỏ và không cản click hoặc kéo thả ứng dụng bên dưới.
- [ ] Biểu tượng khay cung cấp bật/tắt highlight và “Thoát”; thoát gỡ overlay và biểu tượng, dọn tài nguyên.
- [ ] Ghi bằng chứng chạy thử tương tác và số đo tài nguyên nền ban đầu khi highlight bật/tắt; không dùng vòng lặp bận để theo dõi chuột.

### Blocked by

None - can start immediately.

## 02 — Tùy chỉnh và lưu filled highlight / ring highlight

### What to build

Mở cửa sổ cài đặt native từ biểu tượng khay để chọn filled highlight hoặc ring highlight; chỉnh màu, độ trong suốt, highlight diameter và ring thickness. Thay đổi tác động ngay lên highlight và tự lưu cho lần chạy sau. Đóng cài đặt trả ứng dụng về chạy nền.

### Acceptance criteria

- [ ] Bấm biểu tượng khay mở cài đặt; mở lại không tạo nhiều cửa sổ cài đặt trùng nhau. Đóng cài đặt vẫn giữ highlight và biểu tượng khay hoạt động.
- [ ] Hai kiểu hiển thị đúng: filled highlight tô vùng tròn, ring highlight chỉ có viền và phần giữa rỗng.
- [ ] Có điều khiển màu, độ trong suốt, highlight diameter và thanh ring thickness riêng; thay đổi hiển thị ngay mà không cần khởi động lại.
- [ ] Highlight diameter không tính glow. Ring thickness có giới hạn hợp lệ tương ứng với đường kính; kiểm tra dữ liệu nhập không tạo kích thước vô lý.
- [ ] Cài đặt tự lưu và khôi phục sau khi thoát/mở lại; cấu hình thiếu hoặc hỏng có giá trị mặc định hợp lệ để ứng dụng vẫn mở được.
- [ ] Kiểm chứng đổi kiểu/chỉnh giá trị, lưu/đọc và trường hợp cấu hình hỏng. Đo tài nguyên khi mở/đóng cài đặt nhiều lần để phát hiện giữ tài nguyên không cần thiết hoặc tăng bộ nhớ liên tục.

### Blocked by

- 01.

## 03 — Cursor highlight trên nhiều màn hình và DPI

### What to build

Filled highlight và ring highlight đi theo con trỏ xuyên các màn hình Windows với các mức scale khác nhau. Cài đặt kích thước có cùng ý nghĩa theo đơn vị hiển thị logic trên mỗi màn hình; không bị lệch vị trí, cắt hình ở rìa màn hình hoặc mất highlight khi đổi màn hình.

### Acceptance criteria

- [ ] Con trỏ và highlight khớp vị trí qua nhiều màn hình, gồm màn hình đặt bên trái hoặc phía trên màn hình chính với tọa độ âm.
- [ ] Highlight diameter và ring thickness được scale theo DPI màn hình hiện tại; kiểm chứng ít nhất tổ hợp 100% và 150%, không yêu cầu kích thước vật lý theo centimet bằng nhau.
- [ ] Đi qua ranh giới màn hình không làm mất highlight, để lại ảnh cũ hoặc khiến các phần của hình bị cắt sai.
- [ ] Thay đổi scale, thay đổi màn hình chính hoặc kết nối/ngắt màn hình trong lúc chạy không làm ứng dụng crash; cài đặt vẫn truy cập được.
- [ ] Kiểm chứng cả hai kiểu với một số kích thước khác nhau; ghi rõ ma trận màn hình đã thực sự kiểm tra và số đo tài nguyên trong cấu hình nhiều màn hình.

### Blocked by

- 02.

## 04 — Glow tùy chọn cho cả hai kiểu highlight

### What to build

Thêm lựa chọn glow vào cài đặt, mặc định tắt. Bật tạo quầng sáng mềm theo màu highlight cho cả filled highlight và ring highlight, áp dụng ngay và tự lưu. Bản đầu dùng thông số glow cố định, không thêm thanh chỉnh hoặc animation mới.

### Acceptance criteria

- [ ] Mặc định glow tắt; bật/tắt có tác dụng ngay và được giữ sau khi mở lại ứng dụng.
- [ ] Glow theo màu highlight, đổi đúng khi thay đổi màu, độ trong suốt, kiểu hoặc kích thước.
- [ ] Glow không thay đổi ý nghĩa highlight diameter và không chặn thao tác chuột hay lấy focus.
- [ ] Glow không bị cắt sai ở các cạnh màn hình; hiển thị đúng khi chuyển qua màn hình có DPI khác nhau.
- [ ] Tái sử dụng tài nguyên vẽ khi chỉ di chuyển chuột; cập nhật tài nguyên khi hình thức thay đổi. Chọn cách vẽ dựa trên số đo và hành vi quan sát được, không mặc định buộc dùng Direct2D.
- [ ] So sánh số đo Release trong cùng môi trường giữa glow tắt/bật, đứng yên/di chuyển và lặp bật/tắt để kiểm tra tăng tài nguyên liên tục; ghi lại chi phí và quyết định tối ưu.

### Blocked by

- 02.
- 03.

## 05 — Bật/tắt cursor highlight bằng phím tắt toàn hệ thống

### What to build

Người dùng đổi và lưu phím tắt trong cài đặt, rồi dùng tổ hợp đó để bật/tắt cursor highlight khi đang làm việc trong ứng dụng khác. Trạng thái hiển thị nhất quán với menu khay hệ thống.

### Acceptance criteria

- [ ] Có điều khiển xem/đổi phím tắt, lưu lựa chọn cho lần chạy sau và đăng ký phím tắt trong phiên hiện tại.
- [ ] Phím tắt bật/tắt highlight khi ứng dụng khác có focus, không mở cài đặt hoặc lấy focus ngoài ý muốn.
- [ ] Bật/tắt từ phím tắt và từ menu khay cùng cập nhật một trạng thái; tắt highlight dừng các cập nhật hiển thị không cần thiết.
- [ ] Phím tắt không hợp lệ hoặc đã bị chiếm có thông báo rõ; không âm thầm lưu như đã đăng ký thành công. Nếu đổi thất bại, giữ phím tắt cũ nếu còn dùng được và luôn giữ cách bật/tắt từ khay.
- [ ] Đổi phím tắt hoặc thoát ứng dụng giải phóng đăng ký cũ; giữ phím không khiến trạng thái đảo liên tục ngoài ý muốn.
- [ ] Kiểm chứng khi ứng dụng khác có focus, khi trùng phím và sau khi mở lại; kiểm tra việc đăng ký/đổi phím nhiều lần không làm tăng tài nguyên liên tục.

### Blocked by

- 02.

## 06 — Tùy chọn tự chạy khi đăng nhập Windows

### What to build

Thêm lựa chọn tự chạy khi đăng nhập Windows cho tài khoản hiện tại, mặc định tắt. Bật/tắt trong cài đặt tác động lên đăng ký tự chạy thực tế. Khi tự chạy, ứng dụng portable dùng cấu hình đã lưu và nằm ở khay hệ thống mà không mở cài đặt.

### Acceptance criteria

- [ ] Cài đặt có tùy chọn tự chạy khi đăng nhập, mặc định tắt trên lần dùng đầu tiên; không tự bật lại sau khi người dùng tắt.
- [ ] Bật tạo đăng ký cho tài khoản hiện tại với đúng đường dẫn executable portable, xử lý được đường dẫn có khoảng trắng; không cần quyền quản trị.
- [ ] Tắt gỡ đăng ký của ứng dụng. Thay đổi thất bại hiển thị lỗi và không báo trạng thái thành công giả.
- [ ] Đăng nhập lại khởi chạy ứng dụng với hình thức đã lưu và chỉ hiện biểu tượng khay; mở thủ công khi đã chạy không tạo nhiều overlay hoặc biểu tượng trùng.
- [ ] Khi người dùng di chuyển thư mục portable, mở ứng dụng ở vị trí mới phát hiện và cập nhật đăng ký cũ nếu tùy chọn tự chạy vẫn bật; mô tả việc cần mở lại sau khi di chuyển trong hướng dẫn portable.
- [ ] Kiểm chứng một chu kỳ bật → đăng nhập lại → tắt → đăng nhập lại, đường dẫn có khoảng trắng và trường hợp di chuyển thư mục. Kiểm tra khởi động nhiều lần không sinh nhiều tiến trình nền.

### Blocked by

- 02.

## Cần duyệt

- Sáu phần có vừa mức hay cần gộp/tách?
- Quan hệ phụ thuộc có đúng không? Sau 02 có thể làm độc lập 03, 05 và 06; 04 cần 03 để kiểm chứng glow xuyên màn hình/DPI.
- Sau khi người dùng duyệt, xuất bản sáu issue với đầy đủ nội dung và liên kết blocker thực tế, không sửa hoặc đóng tài liệu nguồn.
