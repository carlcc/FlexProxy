#pragma once

#include <cstdint>
#include <string>
#include <fstream>

namespace FS::MiscUtils {

inline void Encrypt(uint8_t* buf, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        buf[i] ^= 0xA5;
    }
}

inline uint8_t XorCheck(const uint8_t* buf, size_t len)
{
    uint8_t result { 0 };
    for (size_t i = 0; i < len; ++i) {
        result ^= buf[i];
    }
    return result;
}

template <class T>
inline bool LoadFileContent(T& outContent, const std::string& filePath)
{
    std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
    if (!ifs) {
        return false;
    }
    ifs.seekg(0, std::ios::end);
    outContent.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*)outContent.data(), std::streamsize(outContent.size()));
    return ifs.gcount() == outContent.size();
}

inline bool StoreFileContent(void* data, size_t length, const std::string& filePath)
{
    std::ofstream ofs(filePath, std::ios::out | std::ios::binary);
    if (!ofs) {
        return false;
    }
    ofs.write((const char*)data, (std::streamsize)length);
    return ofs.tellp() == length;
}

}