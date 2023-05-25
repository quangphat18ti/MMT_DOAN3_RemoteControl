#include "Define.h"
#include "ScreenShoot.h"
#include "Process.h"
#include "VectorSerialize.h"
#include "FolderTask.h"
#include <iostream>
#include <string>
#include <thread>
#include <WinSock2.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

#define MAX_CONNECTION 2 // 2 client
#define BACK_LOG 1

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

const char *IP = "127.0.0.1";
const int port = 5555;

using namespace std;

int Cnt[MAX_CONNECTION] = {0};
bool isRunningThread = false;

char Rmsg[MAX_LEN] = "", Smsg[MAX_LEN] = "";
int recvBytes, sendBytes;

namespace startupWinsock
{
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
		SOCKET m_socket;
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

	bool bindSocket(SOCKET &m_socket)
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr(IP);
		service.sin_port = htons(port); // [1024;49151]

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

	bool listenSocket(SOCKET &m_socket)
	{
		int listenErr = listen(m_socket, BACK_LOG);
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

	bool connectSocket(SOCKET &m_socket)
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr("192.168.0.44");
		service.sin_port = htons(12345); // [1024;49151]

		int connectErr = connect(m_socket, (SOCKADDR *)&service, sizeof(service));
		if (connectErr != 0)
		{
			printf("connect fail - error: ", WSAGetLastError());
			WSACleanup();
			return 0;
		}
		else
			printf("connect success\n");
		return 1;
	}
}

bool addToArray(const SOCKET &newSocket, SOCKET *ac_sockets)
{
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		// If position is empty
		if (ac_sockets[i] == 0)
		{
			string msg = "You are connected by Server. Let's start!!!";
			// cout << "msg: " << msg.size() << endl;

			string Data = configSend(msg);
			// cout << "DataSend: " << Data.length() << endl;
			int byteSend = send(newSocket, Data.c_str(), Data.length(), 0);
			// cout << "byteSend: " << byteSend << endl;

			ac_sockets[i] = newSocket;
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

int main()
{
	if (!startupWinsock::prepareWinsock())
		return 0;

	auto m_socket = startupWinsock::createSocket();
	if (!m_socket)
		return 0;

	if (!startupWinsock::bindSocket(m_socket))
		return 0;
	if (!startupWinsock::listenSocket(m_socket))
		return 0;

	printf("Server waiting for Client...\n");
	SOCKET ac_sockets[MAX_CONNECTION] = {0};

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
		int activity = select(0, &read_fds, NULL, NULL, NULL);
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
				sd = newSocket;
				FD_SET(sd, &read_fds);
			}
		}

		// catch message
		for (int i = 0; i < MAX_CONNECTION; i++)
		{
			sd = ac_sockets[i];
			if (!FD_ISSET(sd, &read_fds))
				continue;

			recvBytes = recv(ac_sockets[i], Rmsg, MAX_LEN, 0);
			cout << "recvBytes: " << recvBytes << endl;
			if (recvBytes == -1)
			{
				Cnt[i]++;
				if (Cnt[i] > 1000)
				{
					printf("Client Disconnected\n");
					closesocket(ac_sockets[i]);
					printf("Close ac_socket\n");

					ac_sockets[i] = 0;
					Cnt[i] = 0;
					continue;
				}
			}
			if (recvBytes == SOCKET_ERROR)
				continue;

			if (recvBytes == 0)
			{
				printf("Client Disconnected\n");
				closesocket(ac_sockets[i]);
				printf("Close ac_socket\n");

				ac_sockets[i] = 0;
				Cnt[i] = 0;
				continue;
			}

			Cnt[i] = 0;
			// printf("Client: %s\n", Rmsg);

			string req(Rmsg, recvBytes);
			wstring S;
			S = configRecv(req, S);
			wcout << "Client: " << S << endl;

			wstringstream ss(S);
			wstring token;
			ss >> token;

			string Status = "", Data = "";
			if (token == L"process")
			{
				wstring task;
				ss >> task;

				if (task == L"list")
				{
					// process list
					cout << "here!\n";
					vector<Process> processes = enumerateAllProcess();
					for (auto &processe : processes)
					{
						wcout << processe << endl;
					}
					Status = configSend(string("1"));
					Data = configSend(processes);
				}
				else if (task == L"open")
				{
					// process open path
					wstring path;
					ss >> path;
					bool isOk = createProcessByPath(path);
					if (isOk)
					{
						vector<Process> processes = enumerateAllProcess();
						for (auto &processe : processes)
						{
							wcout << processe << endl;
						}
						Status = configSend(string("1"));
						Data = configSend(processes);
					}
					else
					{
						vector<Process> processes = enumerateAllProcess();
						for (auto &processe : processes)
						{
							wcout << processe << endl;
						}
						Status = configSend(string("1"));
						Data = configSend(processes);
					}
				}
				else if (task == L"close")
				{
					// process close id
					wstring id;
					ss >> id;
					DWORD processID = stoi(id);
					bool isOk = closeProcess(processID);
					if (isOk)
					{
						Status = configSend(string("1"));
						Data = configSend(enumerateAllProcess());
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("close process fail"));
					}
				}
				else if (task == L"terminate")
				{
					// process terminate name.exe
					wstring name;
					ss >> name;
					cout << "here!\n";
					bool isOk = closeProcess(name);
					if (isOk)
					{
						Status = configSend(string("1"));
						vector<Process> processes = enumerateAllProcess();
						Data = configSend(processes);
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("close process by name fail"));
					}
				}
				else
				{
					Status = configSend(string("0"));
					Data = configSend(string("task not found"));
				}
			}
			else if (token == L"screen")
			{
				// screen
				SaveScreenShotToFile("screenshot_server.jpg");
				ScreenShotAndSendToClient(ac_sockets[i], "screenshot_server.jpg");
				// Status = configSend(string("2"));
				// Data = configSend(string("Send and Save as screenshot.jpg"));
				continue;
			}
			else if (token == L"file")
			{
				// file
				FolderTask f;
				wstring task;
				ss >> task;
				if (task == L"list")
				{
					// file list path id
					// id = 1 la con truc tiep
					// id = 2 la liet ke tat ca
					wstring Path, id;
					ss >> Path >> id;
					string path = f.convertWstringToConstChar(Path);
					cout << "Path: " << path << endl;
					wcout << "id: " << id << endl;
					if (id == L"1")
					{
						cout << "Here!\n";
						vector<string> ans;
						bool isOk = f.list_files(path.c_str(), ans);
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(ans);
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Wrong Path"));
						}
					}
					else if (id == L"2")
					{
						vector<string> ans;
						bool isOk = f.recursive_list_files(path.c_str(), ans);
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(ans);
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Wrong Path"));
						}
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("task not found"));
					}
				}
				else if (task == L"create")
				{
					// file create type path
					// dir
					// file
					wstring type, Path;
					ss >> type >> Path;
					string path = f.convertWstringToConstChar(Path);

					if (type == L"dir")
					{
						bool isOk = f.createFolder(path.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("create folder SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("create folder fail"));
						}
					}
					else if (type == L"file")
					{
						bool isOk = f.createFile(path.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("create file SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("create file fail"));
						}
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("task not found"));
					}
				}
				else if (task == L"delete")
				{
					// file delete type path
					wstring type, Path;
					ss >> type >> Path;
					string path = f.convertWstringToConstChar(Path);
					if (type == L"dir")
					{
						bool isOk = f.deleteFolder(path.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("Delete Folder SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Delete Folder fail"));
						}
					}
					else if (type == L"file")
					{
						bool isOk = f.deleteFile(path.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("Delete File SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Delete File fail"));
						}
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("task not found"));
					}
				}
				else if (task == L"copy")
				{
					// file copy type pathSrc pathDes
					wstring type, PathSrc, PathDes;
					ss >> type >> PathSrc >> PathDes;
					string pathSrc = f.convertWstringToConstChar(PathSrc);
					string pathDes = f.convertWstringToConstChar(PathDes);

					if (type == L"dir")
					{
						bool isOk = f.copyFolder(pathSrc.c_str(), pathDes.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("Copy Folder SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Copy Folder fail"));
						}
					}
					else if (type == L"file")
					{
						bool isOk = f.copyFile(pathSrc.c_str(), pathDes.c_str());
						if (isOk)
						{
							Status = configSend(string("3"));
							Data = configSend(string("Copy File SUCCESS!!!"));
						}
						else
						{
							Status = configSend(string("0"));
							Data = configSend(string("Copy File fail"));
						}
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("task not found"));
					}
				}
				else if (task == L"rename")
				{
					// file rename type pathSrc pathDes
					wstring type, OldName, NewName;
					ss >> type >> OldName >> NewName;
					string oldName = f.convertWstringToConstChar(OldName);
					string newName = f.convertWstringToConstChar(NewName);
					bool isOk = f.renameFileFolder(oldName.c_str(), newName.c_str());
					if (isOk)
					{
						Status = configSend(string("3"));
						Data = configSend(string("Rename SUCCESS!!!"));
					}
					else
					{
						Status = configSend(string("0"));
						Data = configSend(string("Rename fail"));
					}
				}
				else
				{
					Status = configSend(string("0"));
					Data = configSend(string("task not found"));
				}
			}
			// else if (token == L"app")
			// {
			// 	// app
			// 	wstring task;
			// 	ss >> task;
			// 	if (task == L"list")
			// 	{
			// 		// app list
			// 		Data = listApp();
			// 	}
			// 	else if (task == L"open")
			// 	{
			// 		// app open id
			// 		wstring id;
			// 		ss >> id;
			// 		Data = openApp();
			// 	}
			// 	else if (task == L"close")
			// 	{
			// 		// app close id
			// 		string id;
			// 		ss >> id;
			// 		Data = closeApp();
			// 	}
			// 	else
			// 	{
			// 		Status = configSend(string("0"));
			// 		Data = configSend(string("task not found"));
			// 	}
			// }
			// else
			// {
			// 	Status = configSend(string("error"));
			// 	Data = configSend(string("task not found"));
			// }
			// else if (token == L"key")
			// {
			// 	// key
			// 	string task;
			// 	ss >> task;
			// 	if (task == L"start")
			// 	{
			// 		// key start
			// 		if (isRunningThread == false)
			// 		{
			// 			isRunningThread = true;
			// 			Status = configSend(string("success"));
			// 			thread t(trackKeyboard);
			// 			Data = configSend(string("Start KeyBoard Tracking!"));
			// 		}
			// 		else
			// 		{
			// 			Status = configSend(string("error"));
			// 			Data = configSend(string("KeyBoard Tracking is running!"));
			// 		}
			// 	}
			// 	else
			// 	{
			// 		Status = configSend(string("error"));
			// 		Data = configSend(string("task not found"));
			// 	}
			// }
			// else if (token == L"file")
			// {
			// 	string task;
			// 	ss >> task;
			// 	if (task == L"list")
			// 	{
			// 		// file list
			// 		Data = listFile();
			// 	}
			// 	else if (task == L"open")
			// 	{
			// 		// file open path
			// 		string path;
			// 		ss >> path;
			// 		Data = openFile();
			// 	}
			// 	else if (task == L"delete")
			// 	{
			// 		// file close path
			// 		string path;
			// 		ss >> path;
			// 		Data = deleteFile();
			// 	}
			// 	else
			// 	{
			// 		Status = configSend(string("error"));
			// 		Data = configSend(string("task not found"));
			// 	}
			// }
			// else
			// {
			// 	Status = configSend(string("error"));
			// 	Data = configSend(string("task not found"));
			// }
			vector<string> response;
			response.push_back(Status);
			response.push_back(Data);

			string data = configSend(response);
			sendBytes = send(ac_sockets[i], data.c_str(), data.length(), 0);
			cout << "sendBytes: " << sendBytes << endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	closesocket(m_socket);
	printf("Close m_socket\n");
	WSACleanup();
	return 0;
}