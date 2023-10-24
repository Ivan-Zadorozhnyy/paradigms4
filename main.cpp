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
    HINSTANCE hGetProcIDDLL = LoadLibrary("libCaesarCipher.dll");
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

    int mode;
    char inputPath[1024], outputPath[1024];
    std::cout << "Select mode:\n1. Normal\n2. Secret\nEnter choice: ";
    std::cin >> mode;
    std::cin.ignore();

    if (mode == 1) {
        int operation;
        std::cout << "1. Encrypt\n2. Decrypt\nEnter choice: ";
        std::cin >> operation;
        std::cin.ignore();

        std::cout << "Enter input file path: ";
        std::cin.getline(inputPath, sizeof(inputPath));

        std::cout << "Enter output file path: ";
        std::cin.getline(outputPath, sizeof(outputPath));

        int key;
        std::cout << "Enter key (for encryption/decryption): ";
        std::cin >> key;

        try {
            std::vector<std::string> chunks = reader->read(inputPath);
            std::vector<std::string> processedChunks;
            for (const auto& chunk : chunks) {
                char* result;
                if (operation == 1) {
                    result = encryptFunc(const_cast<char*>(chunk.c_str()), key);
                } else {
                    result = decryptFunc(const_cast<char*>(chunk.c_str()), key);
                }
                processedChunks.push_back(std::string(result));
                delete[] result;
            }
            writer->write(outputPath, processedChunks);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    } else if (mode == 2) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(1, 256);
        int randomKey = dist(mt);

        std::cout << "Enter input file path: ";
        std::cin.getline(inputPath, sizeof(inputPath));

        std::cout << "Enter output file path: ";
        std::cin.getline(outputPath, sizeof(outputPath));

        try {
            std::vector<std::string> chunks = reader->read(inputPath);
            std::vector<std::string> processedChunks;
            for (const auto& chunk : chunks) {
                char* result = encryptFunc(const_cast<char*>(chunk.c_str()), randomKey);
                processedChunks.push_back(std::string(result));
                delete[] result;
            }
            writer->write(outputPath, processedChunks);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Invalid mode selected!" << std::endl;
    }

    delete reader;
    delete writer;
    FreeLibrary(hGetProcIDDLL);
    return 0;
}