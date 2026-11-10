# Cursor highlight — Issues

Thiết kế và breakdown đã duyệt ngày 2026-11-10. **Implementation đủ 6 issue; nghiệm thu chưa hoàn tất.**

| Issue | Phần việc | Blocked by | Status (triage) | Execution |
| --- | --- | --- | --- | --- |
| [#01](issues/01-basic-cursor-highlight.md) | Cursor highlight từ khay | Không | ready-for-agent | complete |
| [#02](issues/02-highlight-settings.md) | Cài đặt hình thức và tự lưu | 01 | ready-for-agent | complete |
| [#03](issues/03-multi-monitor-dpi.md) | Nhiều màn hình / DPI | 02 | needs-info | verification-pending |
| [#04](issues/04-optional-glow.md) | Glow | 02, 03 | needs-info | verification-pending |
| [#05](issues/05-global-hotkey.md) | Phím tắt toàn hệ thống | 02 | ready-for-agent | complete |
| [#06](issues/06-start-at-sign-in.md) | Tự chạy khi đăng nhập | 02 | needs-info | verification-pending |

Status ghi triage; Execution ghi tiến độ thực hiện. Các issue complete không còn việc implementation.

**27/34 acceptance criteria đã có bằng chứng.** Bảy mục còn chờ hai màn hình vật lý 100%/150%, thay topology và đăng nhập lại Windows thật. OS API replay không thay thế các bước đó.

Người dùng xác nhận chỉ có một màn hình và yêu cầu để các kiểm chứng này chờ.

- [Audit từng tiêu chí](VERIFICATION.md).
- [22 test đạt, không skip](../../docs/evidence/integration-summary.json).
- [Số đo tài nguyên](../../docs/evidence/resources.json), [manifest ZIP](../../docs/evidence/package.json).
- [Checklist còn chờ](../../docs/manual-checks.md).
- [PRD nguồn](PRD.md), [breakdown đã duyệt](issue-breakdown-draft.md), [ADR](../../docs/adr/0001-native-windows-app.md).

Nguồn PRD/breakdown có nội dung chờ duyệt mang tính lịch sử trước lần chốt thiết kế. Trạng thái hiện tại nằm trong các issue và audit, không suy ra từ các câu chờ xác nhận cũ.

