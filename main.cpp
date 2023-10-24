#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <filesystem>

extern "C" {
char* encrypt(char* rawText, int key);
char* decrypt(char* encryptedText, int key);
}

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

int main() {
    int choice, key, mode;
    std::string inputPath, outputPath;
    char* buffer;
    std::streamsize size;

    IReader* reader = new FileReader();
    IWriter* writer = new FileWriter();

    std::cout << "Choose an operating mode:" << std::endl;
    std::cout << "1. Normal mode" << std::endl;
    std::cout << "2. Secret mode" << std::endl;
    std::cout << "Enter mode choice: ";
    std::cin >> mode;
    std::cin.ignore();

    if (mode == 1) {
        std::cout << "Choose an option:" << std::endl;
        std::cout << "1. Encrypt a file" << std::endl;
        std::cout << "2. Decrypt a file" << std::endl;
        std::cout << "Enter choice: ";
        std::cin >> choice;
        std::cin.ignore();

        std::cout << "Enter the input file path: ";
        std::getline(std::cin, inputPath);

        reader->read(inputPath, buffer, size);

        std::cout << "Enter the output file path: ";
        std::getline(std::cin, outputPath);

        std::cout << "Enter the key for operation: ";
        std::cin >> key;

        if (choice == 1) {
            char* encrypted = encrypt(buffer, key);
            writer->write(outputPath, encrypted, size);
            delete[] encrypted;
        } else if (choice == 2) {
            char* decrypted = decrypt(buffer, key);
            writer->write(outputPath, decrypted, size);
            delete[] decrypted;
        }

    } else if (mode == 2) {
        std::cout << "Secret mode activated." << std::endl;
        std::cout << "Enter the path of the file to encrypt: ";
        std::getline(std::cin, inputPath);

        reader->read(inputPath, buffer, size);

        key = generateRandomKey();
        std::cout << "Using generated encryption key: " << key << std::endl;
        char* encrypted = encrypt(buffer, key);

        std::cout << "Enter the path to save the encrypted file: ";
        std::getline(std::cin, outputPath);
        writer->write(outputPath, encrypted, size);

        delete[] encrypted;
    }

    delete[] buffer;
    delete reader;
    delete writer;

    return 0;
}