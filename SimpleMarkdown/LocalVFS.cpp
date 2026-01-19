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

std::string LocalVFS::get_string(const std::string& path)
{
    if(!path.empty())
    {
        auto bin = get_binary(path);
        if(!bin.empty())
        {
            return std::string(bin.begin(), bin.end());
        }
    }
    return std::string();
}

std::string LocalVFS::get_extension(const std::string& path)
{
    auto ext = fs::path(path).extension().generic_string();
    if(!ext.empty())
    {
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    return ext;
 
}

bool LocalVFS::set_current_path(const std::string& path)
{
    try
    {
        fs::current_path(path);
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
 
}

std::string LocalVFS::get_parent_path(const std::string& path)
{
    return fs::path(path).parent_path().generic_string();
}

std::string LocalVFS::get_current_path()
{
    return fs::current_path().generic_string();
}
