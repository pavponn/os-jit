#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <vector>
#include <string.h>
#include <string>
#include <sstream>
#include <cstring>
#include <iterator>

uint8_t const DEFAULT_VALUE_1 = 0xFF;
uint8_t const DEFAULT_VALUE_2 = 0x04;

size_t const DEFAULT_INDEX_1 = 5;
size_t const DEFAULT_INDEX_2 = 10;

/*
 * code:
 * push rbp
 * move rbp, rsp
 * mov eax, 0xFF
 * mov ebx, 0x04
 * imul eax, ebx
 * pop rbp
 * ret
 */

using jitFunc = int(*)();

std::vector<uint8_t> code {
        0x55,
        0x48, 0x89, 0xE5,
        0xB8, DEFAULT_VALUE_1, 0x00, 0x00, 0x00,
        0xBB, DEFAULT_VALUE_2, 0x00, 0x00, 0x00,
        0x0F, 0xAF, 0xC3,
        0x5D,
        0xc3
};

void printErrorMessage(std::string const& message, bool useStrerror = true) {
    std::cerr << "Error occurred, " << message << "." << std::endl;
    if (useStrerror) {
        std::cerr<< strerror(errno) << std::endl;
    }
}

void printHelpMessage() {
    std::cout << "It is an educational project, aim is to to understand how JIT compilers work.\n"
                 "Function, provided here, is simple: it multiplies two numbers (255 and 4 by default)\n"
                 "Supported commands:\n"
                 "execute <arg1> <arg2> - executes function with specified arguments, it's possible to specify"
                 "one or none of them;\n"
                 "exit - close program;\n"
                 "help | -help | --help - print this message"
                 << std::endl;
}

void* allocateMemory(size_t len) {
    void* memory = mmap(nullptr, len, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (memory == MAP_FAILED) {
        return nullptr;
    }
    return memory;
}

bool freeMemory(void* addr, size_t len) {
    return munmap(addr, len) == 0;
}

bool changePermissions(void* addr, size_t len, int prot) {
    return mprotect(addr, len, prot) == 0;
}

bool changePermissionsAndCheck(void* addr, size_t len, int prot) {
    if (!changePermissions(addr, len, prot)) {
        printErrorMessage("can't change memory permission using mprotect");
        if (!freeMemory(addr, len)) {
            printErrorMessage("can't free allocated memory using munmap");
        }
        return false;
    }
    return true;
}

std::vector<std::string> splitString(std::string const& str) {
    std::istringstream stream(str);
    std::vector<std::string> result{std::istream_iterator<std::string>(stream),
            std::istream_iterator<std::string>()};
    return result;
}

void setDefault(size_t index, uint8_t value) {
    code[index] = value;
    code[index + 1] = 0x00;
    code[index + 2] = 0x00;
    code[index + 3] = 0x00;
}

void patchCode(size_t index, int argument) {
    code[index + 3] = static_cast<unsigned char>((argument >> 24) & 0xFF);
    code[index + 2] = static_cast<unsigned char>((argument >> 16) & 0xFF);
    code[index + 1] = static_cast<unsigned char>((argument >> 8) & 0xFF);
    code[index] = static_cast<unsigned char>(argument & 0xFF);
}

bool setArgument(std::string const& line, int& argument) {
    std::istringstream firstArgumentStream(line);
    if (firstArgumentStream >> argument) {
        return true;
    }
    return false;
}

int main() {
    std::string str;
    while (std::getline(std::cin, str)) {
        auto command = splitString(str);

        if (command.empty()) {
            continue;
        }
        if (command.size() > 3) {
            printErrorMessage("Not a valid number of arguments", false);
            continue;
        }

        if (command[0] == "exit") {
            break;
        } else if (command[0] == "help" || command[0] == "-help" || command[0] == "--help") {
            printHelpMessage();
        } else if (command[0] == "execute") {
            if (command.size() >= 2) {
                int firstArg = 0;
                if (!setArgument(command[1], firstArg)) {
                    printErrorMessage("Not a valid argument: " + command[1] + ". Use --help", false);
                    continue;
                }
                patchCode(DEFAULT_INDEX_1, firstArg);

                if (command.size() == 3) {
                    int secondArg = 0;
                    if (!setArgument(command[2], secondArg)) {
                        printErrorMessage("Not a valid argument: " + command[2] + ". Use --help", false);
                        continue;
                    }
                    patchCode(DEFAULT_INDEX_2, secondArg);
                }
            }

            void* ptr = allocateMemory(code.size() * sizeof(unsigned char));

            if (ptr == nullptr) {
                printErrorMessage("can't allocate memory using mmap");
                return EXIT_FAILURE;
            }

            std::memcpy(ptr, code.data(), code.size());

            if (!changePermissionsAndCheck(ptr, code.size(), PROT_EXEC | PROT_READ)) {
                return EXIT_FAILURE;
            }

            auto func = (jitFunc)ptr;

            std::cout << "result: " << func() << std::endl;

            if (!changePermissionsAndCheck(ptr, code.size(), PROT_READ | PROT_WRITE)) {
                return EXIT_FAILURE;
            }

            if (!freeMemory(ptr, code.size())) {
                printErrorMessage("can't free allocated memory using munmap");
                return EXIT_FAILURE;
            }
        } else {
            printErrorMessage("not a valid command, use --help", false);
        }

        setDefault(DEFAULT_INDEX_1, DEFAULT_VALUE_1);
        setDefault(DEFAULT_INDEX_2, DEFAULT_VALUE_2);
    }
    return EXIT_SUCCESS;
}