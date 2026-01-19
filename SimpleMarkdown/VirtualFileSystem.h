#pragma once
// 在 wxContainer.h 中添加
#include <vector>
#include <string>

class VirtualFileSystem
{
public:
    // 获取文件数据，找不到返回空vector
    virtual std::vector<uint8_t> get_binary(const std::string& path) = 0;
    virtual std::string get_extension(const std::string& path) = 0;
    virtual ~VirtualFileSystem() = default;
};