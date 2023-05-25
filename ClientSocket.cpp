#include "Define.h"
#include "ClientSocket.h"
#include "VectorSerialize.h"
#include "Process.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;

char recv_msg[MAX_LEN];
int recvBytes;

bool ClientSocket::prepareWinsock()
{
    WORD verRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    int wsaErr = WSAStartup(verRequested, &wsaData);
    if (wsaErr != 0)
    {
        cout << "WSAStartup fail - error: %d" << wsaErr;
        return 0;
    }
    else
        cout << "WSAStartup success - status: "
             << wsaData.szSystemStatus << "\n";

    return 1;
}

ClientSocket::ClientSocket(string IP, int port) : m_IP(IP), m_port(port)
{
    m_socket = 0;
    is_continue = true;
}

bool ClientSocket::createSocket()
{
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        printf("socket fail - error: ", WSAGetLastError());
        WSACleanup();
        return 0;
    }
    else
        printf("socket success\n");
    return 1;
}

bool ClientSocket::connectSocket()
{
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(m_IP.c_str());
    service.sin_port = htons(m_port); // [1024;49151]
    cout << "Connect to " << m_IP << ":" << m_port << endl;

    int connectErr = connect(m_socket, (SOCKADDR *)&service, sizeof(service));
    if (connectErr != 0)
    {
        printf("connect fail - error: ", WSAGetLastError());
        cin.get();
        WSACleanup();
        return 0;
    }

    // receive msg
    cout << "Waiting for Server Accepted!" << endl;
    do
    {
        recv_msg[0] = '\0'; // clear recv_msg[
        recvBytes = recv(m_socket, recv_msg, MAX_LEN, 0);
    } while (recvBytes == SOCKET_ERROR);

    if (recvBytes == 0)
    {
        printf("Connection closed\n");

        closesocket(m_socket);
        printf("Close m_socket\n");
        WSACleanup();

        return 0;
    }
    else
    {
        string recv(recv_msg, recvBytes);
        vector<string> message;
        message = configRecv(recv, message);
        for (auto &msg : message)
            cout << msg << endl;
    }

    return 1;
}

void ClientSocket::start()
{
    if (!ClientSocket::prepareWinsock())
        return;
    if (!createSocket())
        return;
    if (!connectSocket())
        return;

    while (is_continue)
    {
        // printf("Enter your message: ");
        printf("Client: ");
        getline(wcin, send_msg);
        string data = configSend(send_msg);

        // cout << "sendBytes: " << sendBytes << endl;
        // printf("msg = .%s.\n", Smsg);
        // printf("Send %d/%d bytes\n", sendBytes, strlen(Smsg));

        if (send_msg == L"STOP_CLIENT")
        {
            is_continue = false;
            break;
        }
        sendBytes = send(m_socket, data.c_str(), data.length(), 0);
        cout << "sendBytes: " << sendBytes << endl;
        // std::this_thread::sleep_for(std::chrono::seconds(1));

        do
        {
            recvBytes = recv(m_socket, recv_msg, MAX_LEN, 0);
            recv_msg[recvBytes] = '\0';
        } while (recvBytes == SOCKET_ERROR);

        if (recvBytes == 0)
        {
            printf("Connection closed\n");
            break;
        }
        else
        {
            cout << "recvBytes: " << recvBytes << endl;

            string msg(recv_msg, recvBytes);
            // Xác định kiểu nhận về (String) và tạo 1 vector ans
            vector<string> datas;
            datas = configRecv(msg, datas);

            string status;
            status = configRecv(datas[0], status);
            cout << "status: " << status << endl;
            if (status == "0")
            {
                string error;
                error = configRecv(datas[1], error);
                cout << "Server: "
                     << "Error (" << error << ")" << endl;
            }
            else if (status == "1") // process
            {
                vector<Process> processes;
                processes = configRecv(datas[1], processes);
                cout << "Status: Success\n";
                cout << "List of processes: \n"
                     << endl;
                for (auto &process : processes)
                    wcout << process << endl;
            }
            else if (status == "2") // screenshot
            {
                // string data;
                // data = configRecv(datas[1], data);
                // cout << "Status: Success - " << data << "\n";

                SaveScreenShotToFile(m_socket, "screenshot_client.jpg");
            }
            else if (status == "3") // file
            {
                vector<string> abc;
                abc = configRecv(datas[1], abc);
                cout << "Status: Success"
                     << "\n";

                for (auto &file : abc)
                    cout << file << endl;
            }
        }
    }

    closesocket(m_socket);
    printf("Close m_socket\n");
    WSACleanup();
}

void ClientSocket::SaveScreenShotToFile(SOCKET m_socket, char *filename)
{
    // Nhan kich thuoc file
    streampos file_size;
    int bytes_received;
    do
    {
        bytes_received = recv(m_socket, reinterpret_cast<char *>(&file_size), sizeof(streampos), 0);
    } while (bytes_received == -1);

    cout << "file_size: " << file_size << endl;
    // Tao file anh
    ofstream out(filename, ios::binary);
    char *buffer = new char[BUFFER_SIZE];
    int bytes_read = 0;

    // Nhan file anh
    while (bytes_read < file_size)
    {
        do
        {
            bytes_received = recv(m_socket, buffer, BUFFER_SIZE, 0);
        } while (bytes_received == -1);

        cout << "bytes_received: " << bytes_received << endl;
        out.write(buffer, bytes_received);
        bytes_read += bytes_received;
        if (bytes_read == file_size)
        {
            break;
        }
    }

    out.close();

    cout << "write file success\n";
    delete[] buffer;
}
