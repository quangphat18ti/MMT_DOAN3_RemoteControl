#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
using namespace std;

template <class T>
class VectorSerialize
{

private:
    friend class boost::serialization::access;
    vector<T> m_data;

public:
    VectorSerialize();
    VectorSerialize(const vector<T> &data);
    VectorSerialize(T *data, int n);
    VectorSerialize(T data);

public:
    int size();
    vector<T> rawData();

public:
    string writeToFile(string filename);
    VectorSerialize readFromFile(string filename);

    string Serialize();
    VectorSerialize DeSerialize(string receivedDataStr);
};

template <class T>
inline VectorSerialize<T>::VectorSerialize()
{
}

template <class T>
inline VectorSerialize<T>::VectorSerialize(T data)
{
    m_data.push_back(data);
}

template <class T>
inline VectorSerialize<T>::VectorSerialize(T *data, int n)
{
    m_data.resize(n);
    for (int i = 0; i < n; i++)
        m_data[i] = data[i];
}

template <class T>
inline VectorSerialize<T>::VectorSerialize(const vector<T> &data)
{
    m_data = data;
}

template <class T>
inline int VectorSerialize<T>::size()
{
    return m_data.size();
}

template <class T>
inline string VectorSerialize<T>::writeToFile(string filename)
{
    try
    {
        ofstream outfile(filename, ios::binary);
        if (!outfile.is_open())
        {
            cerr << "File " << filename << " Cannot write!";
            exit(-1);
        }
        boost::archive::binary_oarchive archive(outfile);
        archive << m_data;
        outfile.close();
    }
    catch (const boost::archive::archive_exception &ex)
    {
        std::cerr << "Serialization/Deserialization error: " << ex.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception occurred: " << ex.what() << std::endl;
    }
    return "";
}

template <class T>
inline VectorSerialize<T> VectorSerialize<T>::readFromFile(string filename)
{
    try
    {
        ifstream infile(filename, ios::binary);
        if (!infile.is_open())
        {
            cerr << "File " << filename << " Cannot read!";
            exit(-1);
        }
        boost::archive::binary_iarchive archive(infile);

        archive >> m_data;

        infile.close();
    }
    catch (const boost::archive::archive_exception &ex)
    {
        std::cerr << "Serialization/Deserialization error: " << ex.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception occurred: " << ex.what() << std::endl;
    }

    return *this;
}

template <class T>
inline vector<T> VectorSerialize<T>::rawData()
{
    return m_data;
}

template <class T>
inline string VectorSerialize<T>::Serialize()
{
    std::ostringstream oss;
    boost::archive::binary_oarchive binaryOutArchive(oss);
    binaryOutArchive << m_data;

    string serializedData = oss.str();
    return serializedData;
}

template <class T>
inline VectorSerialize<T> VectorSerialize<T>::DeSerialize(string receivedDataStr)
{
    istringstream iss(receivedDataStr);
    boost::archive::binary_iarchive binaryInArchive(iss);
    binaryInArchive >> m_data;
    return *this;
}

template <class T>
string configSend(T data)
{
    VectorSerialize<T> vectorSerialize(data);
    string Data = vectorSerialize.Serialize();
    return Data;
}

template <class T>
string configSend(vector<T> data)
{
    VectorSerialize<T> vectorSerialize(data);
    string Data = vectorSerialize.Serialize();
    return Data;
}

template <class T>
vector<T> configRecv(string recv, vector<T> data)
{
    VectorSerialize<T> vectorSerialize(data);
    vectorSerialize.DeSerialize(recv);
    data = vectorSerialize.rawData();
    return data;
}

template <class T>
T configRecv(string recv, T data)
{
    VectorSerialize<T> vectorSerialize(data);
    vectorSerialize.DeSerialize(recv);
    data = vectorSerialize.rawData()[0];
    return data;
}