#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;

class IReader
{
public:
    virtual ~IReader() {}
    virtual void read(const string& filePath, char*& buffer, streamsize& size) = 0;
};

class FileReader : public IReader
{
public:
    virtual void read(const string& filePath, char*& buffer, streamsize& size)
    {
        ifstream file(filePath, ios::binary | ios::ate);
        if (!file.is_open())
        {
            throw runtime_error("File not found or failed to open!");
        }

        size = file.tellg();
        file.seekg(0, ios::beg);
        buffer = new char[size];
        file.read(buffer, size);
        file.close();
    }
};

class IWriter {
public:
    virtual ~IWriter() {}
    virtual void write(const std::string &path, const char* data, std::streamsize size) = 0;
};

class FileWriter : public IWriter {
public:
    void write(const std::string &path, const char* data, std::streamsize size) override {
        std::ofstream file(path, std::ios::binary);
        if (file.is_open()) {
            throw std::runtime_error("File already exists!");
        }
        file.write(data, size);
        file.close();
    }
};