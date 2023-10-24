#include <iostream>
#include <fstream>
#include <windows.h>
#include <random>
#include <vector>

constexpr size_t CHUNK_SIZE = 128;

typedef char* (*encrypt_ptr_t)(char*, int);
typedef char* (*decrypt_ptr_t)(char*, int);

class IReader {
public:
    virtual ~IReader() {}
    virtual std::vector<std::string> read(const char* path) = 0;
};

class FileReader : public IReader {
public:
    std::vector<std::string> read(const char* path) override {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("File does not exist!");
        }

        std::vector<std::string> chunks;
        char buffer[CHUNK_SIZE + 1] = {0};

        while (file.read(buffer, CHUNK_SIZE)) {
            chunks.push_back(std::string(buffer, CHUNK_SIZE));
        }

        if (file.gcount() > 0) {
            chunks.push_back(std::string(buffer, file.gcount()));
        }

        file.close();
        return chunks;
    }
};

class IWriter {
public:
    virtual ~IWriter() {}
    virtual void write(const char* path, const std::vector<std::string>& chunks) = 0;
};

class FileWriter : public IWriter {
public:
    void write(const char* path, const std::vector<std::string>& chunks) override {
        std::ifstream checkFile(path);
        if (checkFile.is_open()) {
            checkFile.close();
            throw std::runtime_error("File already exists!");
        }

        std::ofstream file(path, std::ios::binary);
        for (const auto& chunk : chunks) {
            file.write(chunk.c_str(), chunk.size());
        }
        file.close();
    }
};






int main() {
    int choice, key, mode;
    std::string inputPath, outputPath;
    char* buffer;
    std::streamsize size;

    HINSTANCE hGetProcIDDLL = LoadLibrary("caesar.dll");
    if (!hGetProcIDDLL) {
        std::cerr << "Could not load the dynamic library" << std::endl;
        return EXIT_FAILURE;
    }

    encrypt_ptr_t encryptFunc = (encrypt_ptr_t) GetProcAddress(hGetProcIDDLL, "encrypt");
    decrypt_ptr_t decryptFunc = (decrypt_ptr_t) GetProcAddress(hGetProcIDDLL, "decrypt");
    if (!encryptFunc || !decryptFunc) {
        std::cerr << "Failed to get function addresses from DLL." << std::endl;
        FreeLibrary(hGetProcIDDLL);
        return EXIT_FAILURE;
    }

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