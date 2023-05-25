#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <WinSock2.h>
#include <Windows.h>
#include <memory>
#include <string>
#include <wchar.h>
#include <vector>
#include <iostream>
#include "Define.h"

// #pragma comment(lib, "ws2_32.lib") // link library

using namespace std;

class ClientSocket
{
public:
    static bool prepareWinsock();

private:
    string m_IP;
    int m_port;
    SOCKET m_socket;
    bool is_continue;

    wstring send_msg;
    int sendBytes;

public:
    ClientSocket(string IP, int port);
    bool createSocket();
    bool connectSocket();
    void start();
    void SaveScreenShotToFile(SOCKET m_socket, char *filename);
};