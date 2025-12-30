#include "LocalVFS.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;


std::vector<uint8_t> LocalVFS::get_binary(const std::string& src)
{
    fs::path data_path(src);
    
    if (!fs::exists(data_path)) 
    { 
        data_path = "." / data_path;
        if(!fs::exists(data_path)){ return {}; }

    }
    
    
    
    std::ifstream file(data_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << data_path << "\n";
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "读取失败" << data_path << "\n";
    }

    return buffer;

    return std::vector<uint8_t>();
}