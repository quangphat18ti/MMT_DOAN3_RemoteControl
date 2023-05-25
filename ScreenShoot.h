#pragma once
#include "VectorSerialize.h"
#include <cstdio>
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void SaveScreenShotToFile(char *filename)
{
    // Lấy handle của màn hình chính
    HDC hScreen = GetDC(NULL);

    // Lấy chiều rộng và chiều cao của màn hình
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // Tạo một bitmap với kích thước bằng với màn hình
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);

    // Tạo một DC (device context) mới để vẽ lên bitmap
    HDC hDC = CreateCompatibleDC(hScreen);
    SelectObject(hDC, hBitmap);

    // Sử dụng hàm BitBlt để chụp màn hình vào bitmap
    BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

    // Lưu bitmap vào file
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    BITMAPFILEHEADER bmfHeader = {0};
    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp.bmWidthBytes * bmp.bmHeight;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;
    char *pBuffer = new char[bmp.bmWidthBytes * bmp.bmHeight];
    GetDIBits(hDC, hBitmap, 0, bmp.bmHeight, pBuffer, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    // FILE *pFile = fopen(filename, "wb");
    // fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
    // fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, pFile);
    // fwrite(pBuffer, bmp.bmWidthBytes * bmp.bmHeight, 1, pFile);
    // fclose(pFile);

    std::ofstream outputFile(filename, std::ios::binary);
    if (outputFile)
    {
        // Write bmfHeader to the file
        outputFile.write(reinterpret_cast<char *>(&bmfHeader), sizeof(BITMAPFILEHEADER));
        // Write bi to the file
        outputFile.write(reinterpret_cast<char *>(&bi), sizeof(BITMAPINFOHEADER));

        // Write pBuffer to the file
        // Assuming pBuffer is a pointer to the buffer
        // bmp.bmWidthBytes * bmp.bmHeight represents the size of the buffer
        outputFile.write(reinterpret_cast<char *>(pBuffer), bmp.bmWidthBytes * bmp.bmHeight);

        outputFile.close();
        std::wcout << L"Data written to the file successfully." << std::endl;
    }
    else
    {
        std::wcerr << L"Failed to open the file." << std::endl;
    }

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        std::streampos fileSize = file.tellg();
        file.close();

        std::cout << "File size: " << fileSize << " bytes" << std::endl;
    }
    else
    {
        std::cerr << "Failed to open the file." << std::endl;
    }

    delete[] pBuffer;

    // Giải phóng bộ nhớ
    SelectObject(hDC, hOldBitmap);
    DeleteDC(hDC);
    DeleteObject(hBitmap);
    ReleaseDC(NULL, hScreen);
}

void ScreenShotAndSendToClient(SOCKET &ac_socket, char *filename)
{
    // std::ifstream file(filename, std::ios::binary);
    // if (file.is_open())
    // {
    //     // Read the file data into the existing buffer
    //     file.read(Rmsg, MAX_LEN - 1); // Read up to bufferSize - 1 characters

    //     // Null-terminate the buffer
    //     buffer[file.gcount()] = '\0';

    //     file.close();

    //     std::cout << "Data read from the file: " << buffer << std::endl;
    // }
    // else
    // {
    //     std::cerr << "Failed to open the file." << std::endl;
    // }

    ifstream in(filename, ios::binary | ios::ate);
    streampos file_size = in.tellg();
    in.seekg(0, ios::beg);

    char *buffer = new char[BUFFER_SIZE];
    int bytes_sent = 0;

    string status = "2";
    status = configSend(status);
    string Data = "";
    Data = configSend(Data);
    vector<string> response;
    response.push_back(status);
    response.push_back(Data);
    string data = configSend(response);
    send(ac_socket, data.c_str(), data.size(), 0);
    // Gui kich thuoc file den server
    send(ac_socket, reinterpret_cast<char *>(&file_size), sizeof(streampos), 0);

    // Gui file anh den server
    while (!in.eof())
    {
        in.read(buffer, BUFFER_SIZE);
        bytes_sent = send(ac_socket, buffer, BUFFER_SIZE, 0);
    }

    in.close();

    cout << "File sent successfully." << endl;
    delete[] buffer;
}
