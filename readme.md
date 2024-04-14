## Các thư viện sử dụng trong code:

### Winsock2:

- Command Line: `-lws2_32`
- VS: `ws2_32.lib`

### -lpsapi:

- Comand Line: `-lpsapi`

## Lệnh chạy
- Câu lệnh chạy file server: make
- Câu lệnh chạy file client: make c

## Các API sử dụng:
> Format Request: Xâu gồm những cụm cách nhau bởi khoảng trắng.
### Format Response: 2 giá trị:
- Giá trị đầu là `Status Code`: 0 là thất bại. (> 0) là thành công.
- Nếu là thất bại thì giá trị thứ 2 là thông báo.
- Nếu thành công thì có thể là string thông báo hoặc là Data.

| Nhóm lệnh | API                                | Thực hiện                                                               |
| --------- | ---------------------------------- | ----------------------------------------------------------------------- |
| Process   | process list                       | Liệt kê Process                                                         |
|           | process open path                  | Mở một process                                                          |
|           | process close id                   | Tắt process với id                                                      |
| Screen    | screen                             | Chụp màn hình và lưu vào file screenshot_client.jpg                     |
| file      | file list path id                  | Duyệt thư mục path với điều kiện (id = 1, 2) [1 là con, 2 là cháu chắt] |
|           | file create dir path               | Tạo 1 folder với path là ...                                            |
|           | file create file path              | Tạo 1 file với path là ...                                              |
|           | file delete dir path               | Xóa 1 folder                                                            |
|           | file delete file path              | Xóa 1 file                                                              |
|           | file copy dir pathSrc pathDes      | Copy thư mục                                                            |
|           | file rename file oldPath newPath   | Đổi tên 1 file                                                          |
|           | file rename folder oldPath newPath | Đổi tên folder                                                          |
|           |                                    |                                                                         |
|           |                                    |                                                                         |
