#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include "Define.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <memory>
#include <vector>

using namespace std;

class ServerSocket
{
private:
    string m_IP;
    int m_PORT;
    SOCKET m_socket;
    int m_BACK_LOG;
    int MAX_CONNECTION;
    SOCKET *ac_sockets;

public:
    ServerSocket(string IP, int port, int BACK_LOG, int MAX_CONNECTION)
        : m_BACK_LOG(BACK_LOG), MAX_CONNECTION(MAX_CONNECTION), m_IP(IP), m_PORT(port)
    {
        ac_sockets = new SOCKET[MAX_CONNECTION];
    }

    ~ServerSocket()
    {
        delete[] ac_sockets;
    }

public:
    bool prepareWinsock()
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

    SOCKET createSocket()
    {
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET)
        {
            cout << "socket fail - error: " << WSAGetLastError();
            WSACleanup();
            return 0;
        }
        else
            printf("socket success\n");

        return m_socket;
    }

    bool bindSocket()
    {
        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = inet_addr("127.0.0.1");
        service.sin_port = htons(5555); // [1024;49151]

        int bindErr = bind(m_socket, (SOCKADDR *)&service, sizeof(service));
        if (bindErr != 0)
        {
            cout << "bind fail - error:" << WSAGetLastError() << endl;
            closesocket(m_socket);
            WSACleanup();
            return 0;
        }
        else
            cout << "bind success\n";

        return 1;
    }

    bool listenSocket()
    {
        int listenErr = listen(m_socket, m_BACK_LOG);
        if (listenErr != 0)
        {
            printf("listen fail - error: %d", WSAGetLastError());
            closesocket(m_socket);
            WSACleanup();
            return 0;
        }
        else
            printf("listen success\n");

        return 1;
    }

    bool configSend(const SOCKET &ac_socket, const char *data)
    {
        int byteSend = send(ac_socket, data, strlen(data), 0);
        return !(byteSend == SOCKET_ERROR);
    }

    bool addToArray(const SOCKET &newSocket, SOCKET *ac_sockets)
    {
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            // If position is empty
            if (ac_sockets[i] == 0)
            {
                ac_sockets[i] = newSocket;
                configSend(newSocket, "You are connected by Server. Let's start!!!");
                cout << "Added to array of sockets as " << i + 1 << endl;
                return true;
            }
        }

        return false;
    };

    bool checkFree(SOCKET *ac_sockets)
    {
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            if (ac_sockets[i] == 0)
                return true;
        }
        return false;
    }

    void start()
    {
        if (!prepareWinsock())
            return;
        if (!createSocket())
            return;
        if (!bindSocket())
            return;
        if (!listenSocket())
            return;
        printf("Server waiting for Client...\n");

        int max_sd, sd;
        SOCKET newSocket;
        fd_set read_fds;

        bool continueServer = 1;

        while (continueServer)
        {
            // Clear the socket set
            FD_ZERO(&read_fds);

            // Add the m_socket to the set
            FD_SET(m_socket, &read_fds);
            max_sd = m_socket;
            // Update the set
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                sd = ac_sockets[i];
                if (sd > 0)
                {
                    FD_SET(sd, &read_fds);
                }
                if (sd > max_sd)
                    max_sd = sd;
            }

            // Wait for activity on any of the sockets
            int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
            if ((activity < 0) && (WSAGetLastError() != WSAEINTR))
            {
                cout << "select error" << endl;
                break;
            }

            // If there is incoming connection request
            if (FD_ISSET(m_socket, &read_fds) && checkFree(ac_sockets))
            {
                newSocket = accept(m_socket, nullptr, nullptr);
                cout << "new Socket: " << newSocket << endl;
                if (newSocket == INVALID_SOCKET)
                {
                    printf("conect fail - error: %d\n", WSAGetLastError());
                }
                else
                {
                    // Add new socket to array of client sockets
                    addToArray(newSocket, ac_sockets);
                }
            }

            // catch message
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                sd = ac_sockets[i];
                if (!FD_ISSET(sd, &read_fds))
                    continue;

                char Rmsg[MAX_LEN] = "", Smsg[MAX_LEN] = "";
                int recvBytes, sendBytes;
                recvBytes = recv(ac_sockets[i], Rmsg, MAX_LEN, 0);
                if (recvBytes == SOCKET_ERROR)
                    continue;

                if (recvBytes == 0)
                {
                    printf("Client Disconnected\n");
                    closesocket(ac_sockets[i]);
                    printf("Close ac_socket\n");

                    ac_sockets[i] = 0;
                    continue;
                }

                printf("Client: %s\n", Rmsg);
                if (strcmp(Rmsg, "x") == 0)
                    break;

                printf("Server: ");
                scanf("%[^\n]s", Smsg);
                fflush(stdin);

                sendBytes = send(ac_sockets[i], Smsg, strlen(Smsg), 0);

                if (strcmp(Smsg, "STOP SERVER") == 0)
                    continueServer = 0;
            }
        }

        closesocket(m_socket);
        printf("Close m_socket\n");
        WSACleanup();

        return;
    }
};