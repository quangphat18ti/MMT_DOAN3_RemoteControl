#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
using namespace std;

class Process
{
private:
    friend class boost::serialization::access;

    DWORD m_processID;
    wstring m_processName;

    template <class Archive>
    void serialize(Archive &a, const unsigned int version)
    {
        a &m_processID;
        a &m_processName;
    }

public:
    DWORD ID() { return m_processID; }
    wstring Name() { return m_processName; }

public:
    bool operator==(const Process &B)
    {
        return m_processID == B.m_processID && m_processName == B.m_processName;
    }

public:
    Process()
    {
        m_processID = 0;
    }
    Process(DWORD processID, wstring processName) : m_processID(processID), m_processName(processName) {}

public:
    friend wostream &operator<<(wostream &outDev, const Process &p)
    {
        outDev << "processID: " << p.m_processID;
        outDev << " ; "
               << "Name: " << p.m_processName;

        return outDev;
    }

    wstring print();
};

wstring getProcessNameByID(DWORD processID);
wstring getProcessPathByID(DWORD processID);
Process getProcessByID(DWORD pId);
vector<Process> enumerateAllProcess();

int closeProcess(DWORD processID);
int closeProcess(wstring nameProcess);

int createProcessByPath(wstring path);
int createProcessByName(string name);