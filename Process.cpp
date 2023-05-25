#include "Process.h"

#define MAX_PROCESS 2000
#include <windows.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <psapi.h>

using namespace std;

std::wstring ConvertTCHARToWideString(const TCHAR *tcharString)
{
    int bufferSize = lstrlen(tcharString) + 1;
    int wideBufferSize = MultiByteToWideChar(CP_ACP, 0, tcharString, bufferSize, NULL, 0);

    std::wstring wideString(wideBufferSize, L'\0');
    MultiByteToWideChar(CP_ACP, 0, tcharString, bufferSize, &wideString[0], wideBufferSize);

    return wideString;
}

wstring getProcessNameByID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (processHandle)
    {
        HMODULE hModule;
        DWORD bytesNeeded;
        if (EnumProcessModules(processHandle, &hModule, sizeof(hModule), &bytesNeeded))
        {
            GetModuleBaseName(processHandle, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    CloseHandle(processHandle);

    // Convert TCHAR* to wstring
    wstring ws = ConvertTCHARToWideString(szProcessName);
    return ws;
}

// wstring getProcessPathByID(DWORD processID)
// {

//     // TCHAR is same as WCHAR (wchar_t)
//     TCHAR procName[MAX_PATH] = _T("<Unknown>");

//     // opening the process
//     // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
//     HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

//     // checking if process is opened
//     // the processes running in same privilege will only be opened
//     if (process != NULL)
//     {
//         HMODULE hMod;
//         DWORD cbNeeded;

//         GetProcessImageFileNameW(process, &procName[0], MAX_PATH);
//     }

//     // closing process handle
//     // https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
//     CloseHandle(process);
//     //_tprintf(_T("[%d] %s\n"), pId, procName);
//     wstring path(procName);
//     return path;
// }

Process getProcessByID(DWORD pId)
{
    // TCHAR is same as WCHAR (wchar_t)
    TCHAR procName[MAX_PATH] = _T("<Unknown>");

    // opening the process
    // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pId);

    // checking if process is opened
    // the processes running in same privilege will only be opened
    if (process != NULL)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        // enumerating the process modules
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
        if (EnumProcessModules(process, &hMod, sizeof(hMod),
                               &cbNeeded))
        {
            // getting module base name, which is the name of the process
            // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmappedfilenamew
            GetModuleBaseName(process, hMod, procName,
                              sizeof(procName) / sizeof(TCHAR));
        }
    }

    // closing process handle
    // https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
    CloseHandle(process);
    //_tprintf(_T("[%d] %s\n"), pId, procName);
    wstring name = ConvertTCHARToWideString(procName);
    return Process(pId, name);
}

vector<Process> enumerateAllProcess()
{
    vector<Process> processObjects;

    DWORD processes[MAX_PROCESS], bytesNeeded;
    if (!EnumProcesses(processes, sizeof(processes), &bytesNeeded))
    {
        cerr << "Failed to enumerate processes" << std::endl;
        exit(-1);
    }

    auto numProcess = bytesNeeded / sizeof(DWORD);
    for (int i = 0; i < numProcess; i++)
    {
        if (processes[i] == 0)
            continue;
        processObjects.push_back(getProcessByID(processes[i]));
    }

    return processObjects;
}

wstring Process::print()
{
    wstringstream wss;
    wss << (*this);
    return wss.str();
}

int createProcessByPath(wstring path)
{
    STARTUPINFOW si;
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;

    // initializing the variables
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    BOOL cpBool = CreateProcessW(
        NULL,     // creating an independent process
        &path[0], // passing in the path of the executable
        NULL,     // don't want the process handle to be inherited
        NULL,     // don't want the thread handle to be inherited
        FALSE,    // setting inheritance handle to false
        0x0,      // normal creation flags
        NULL,     // using parent process environment config
        NULL,     // the new process will have the same current drive and directory as the calling process
        &si,      // passing in the startup information
        &pi       // passing in the process information
    );

    if (cpBool)
    {
        _tprintf(_T("Process Created: %d\n"), pi.dwProcessId);
    }
    else
    {
        _tprintf(_T("Unable to create process\n"));
        return 1;
    }

    return 0;
}

LPCSTR ConvertWideCharToLPCSTR(const wchar_t *wideCharStr)
{
    int length = wcslen(wideCharStr);

    int size = WideCharToMultiByte(CP_UTF8, 0, wideCharStr, length, NULL, 0, NULL, NULL);
    char *buffer = new char[size + 1];

    WideCharToMultiByte(CP_UTF8, 0, wideCharStr, length, buffer, size, NULL, NULL);
    buffer[size] = '\0';

    return buffer;
}

int createProcessByName(string name)
{
    // Open the process by name
    HINSTANCE hInstance = ShellExecute(nullptr, "open", name.c_str(), nullptr, nullptr, SW_SHOW);
    if ((intptr_t)hInstance > 32)
    {
        std::cout << "Process opened successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to open process." << std::endl;
    }

    return 0;
}

int closeProcess(DWORD processID)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess != nullptr)
    {
        if (TerminateProcess(hProcess, 2))
        {
            std::cout << "Kill Process: " << processID << " Successfully!" << std::endl;
            return 1;
        }
        else
        {
            std::cout << "Failed to terminate process. Error code: " << GetLastError() << std::endl;
            return 0;
        }

        CloseHandle(hProcess);
    }
    else
    {
        std::cout << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return 0;
    }
}

int closeProcess(wstring nameProcess)
{
    wcout << "nameProcess: " << nameProcess << endl;
    vector<Process> processes = enumerateAllProcess();
    for (auto &process : processes)
    {
        wcout << process.Name() << ' ' << nameProcess << ' ' << (process.Name() == nameProcess) << endl;
        if (process.Name() == nameProcess)
        {
            cout << "Close: " << process.ID() << endl;
            if (closeProcess(process.ID()))
            {
                wcout << "Failed to Close " << nameProcess << endl;
                return 0;
            }
        }
    }
    return 1;
}

// #include "Process.h"

// #define MAX_PROCESS 2000
// #include <windows.h>
// #include <vector>
// #include <string>
// #include <stdio.h>
// #include <iostream>
// #include <tchar.h>
// #include <psapi.h>

// using namespace std;

// wstring getProcessNameByID(DWORD processID)
// {
//     TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

//     HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
//     if (processHandle)
//     {
//         HMODULE hModule;
//         DWORD bytesNeeded;
//         if (EnumProcessModules(processHandle, &hModule, sizeof(hModule), &bytesNeeded))
//         {
//             GetModuleBaseName(processHandle, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
//         }
//     }

//     CloseHandle(processHandle);

//     // Convert TCHAR* to wstring
//     wstring ws(szProcessName);
//     return ws;
// }

// wstring getProcessPathByID(DWORD processID)
// {

//     // TCHAR is same as WCHAR (wchar_t)
//     TCHAR procName[MAX_PATH] = _T("<Unknown>");

//     // opening the process
//     // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
//     HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

//     // checking if process is opened
//     // the processes running in same privilege will only be opened
//     if (process != NULL)
//     {
//         HMODULE hMod;
//         DWORD cbNeeded;

//         GetProcessImageFileNameW(process, &procName[0], MAX_PATH);
//     }

//     // closing process handle
//     // https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
//     CloseHandle(process);
//     //_tprintf(_T("[%d] %s\n"), pId, procName);
//     wstring path(procName);
//     return path;
// }

// Process getProcessByID(DWORD pId)
// {
//     // TCHAR is same as WCHAR (wchar_t)
//     TCHAR procName[MAX_PATH] = _T("<Unknown>");

//     // opening the process
//     // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
//     HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pId);

//     // checking if process is opened
//     // the processes running in same privilege will only be opened
//     if (process != NULL)
//     {
//         HMODULE hMod;
//         DWORD cbNeeded;

//         // enumerating the process modules
//         // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
//         if (EnumProcessModules(process, &hMod, sizeof(hMod),
//                                &cbNeeded))
//         {
//             // getting module base name, which is the name of the process
//             // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmappedfilenamew
//             GetModuleBaseName(process, hMod, procName,
//                               sizeof(procName) / sizeof(TCHAR));
//         }
//     }

//     // closing process handle
//     // https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
//     CloseHandle(process);
//     //_tprintf(_T("[%d] %s\n"), pId, procName);
//     wstring name(procName);
//     return Process(pId, name);
// }

// vector<Process> enumerateAllProcess()
// {
//     vector<Process> processObjects;

//     DWORD processes[MAX_PROCESS], bytesNeeded;
//     if (!EnumProcesses(processes, sizeof(processes), &bytesNeeded))
//     {
//         cerr << "Failed to enumerate processes" << std::endl;
//         exit(-1);
//     }

//     auto numProcess = bytesNeeded / sizeof(DWORD);
//     for (int i = 0; i < numProcess; i++)
//     {
//         if (processes[i] == 0)
//             continue;
//         processObjects.push_back(getProcessByID(processes[i]));
//     }

//     return processObjects;
// }

// wstring Process::print()
// {
//     wstringstream wss;
//     wss << (*this);
//     return wss.str();
// }

// int createProcessByPath(wstring path)
// {
//     STARTUPINFO si;
//     si.cb = sizeof(si);

//     PROCESS_INFORMATION pi;

//     // initializing the variables
//     ZeroMemory(&si, sizeof(si));
//     ZeroMemory(&pi, sizeof(pi));

//     BOOL cpBool = CreateProcessW(
//         NULL,     // creating an independent process
//         &path[0], // passing in the path of the executable
//         NULL,     // don't want the process handle to be inherited
//         NULL,     // don't want the thread handle to be inherited
//         FALSE,    // setting inheritance handle to false
//         0x0,      // normal creation flags
//         NULL,     // using parent process environment config
//         NULL,     // the new process will have the same current drive and directory as the calling process
//         &si,      // passing in the startup information
//         &pi       // passing in the process information
//     );

//     if (cpBool)
//     {
//         _tprintf(_T("Process Created: %d"), pi.dwProcessId);
//     }
//     else
//     {
//         _tprintf(_T("Unable to create process\n"));
//         return 1;
//     }

//     return 0;
// }

// LPCSTR ConvertWideCharToLPCSTR(const wchar_t *wideCharStr)
// {
//     int length = wcslen(wideCharStr);

//     int size = WideCharToMultiByte(CP_UTF8, 0, wideCharStr, length, NULL, 0, NULL, NULL);
//     char *buffer = new char[size + 1];

//     WideCharToMultiByte(CP_UTF8, 0, wideCharStr, length, buffer, size, NULL, NULL);
//     buffer[size] = '\0';

//     return buffer;
// }

// int createProcessByName(wstring name)
// {
//     // Open the process by name
//     HINSTANCE hInstance = ShellExecute(nullptr, L"open", name.c_str(), nullptr, nullptr, SW_SHOW);
//     if ((intptr_t)hInstance > 32)
//     {
//         std::cout << "Process opened successfully." << std::endl;
//     }
//     else
//     {
//         std::cerr << "Failed to open process." << std::endl;
//     }

//     return 0;
// }

// int closeProcess(DWORD processID)
// {
//     HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
//     if (hProcess != nullptr)
//     {
//         if (TerminateProcess(hProcess, 2))
//         {
//             std::cout << "Kill Process: " << processID << "Successfully!" << std::endl;
//             return 0;
//         }
//         else
//         {
//             std::cout << "Failed to terminate process. Error code: " << GetLastError() << std::endl;
//             return 2;
//         }

//         CloseHandle(hProcess);
//     }
//     else
//     {
//         std::cout << "Failed to open process. Error code: " << GetLastError() << std::endl;
//         return 1;
//     }
// }

// int closeProcess(wstring nameProcess, vector<Process> &processes)
// {
//     for (auto &process : processes)
//     {
//         if (process.Name() == nameProcess)
//         {
//             cout << "Close: " << process.ID() << endl;
//             if (closeProcess(process.ID()))
//             {
//                 wcout << "Failed to Close " << nameProcess << endl;
//                 return 1;
//             }
//         }
//     }
//     return 0;
// }