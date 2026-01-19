#pragma once
#include "VirtualFileSystem.h"

class LocalVFS : public VirtualFileSystem
{


public:
    LocalVFS() = default;

    std::vector<uint8_t> get_binary(const std::string& path) override;
    std::string get_string(const std::string& path) override;
    std::string get_extension(const std::string& path) override;

    bool set_current_path(const std::string& path) override;
    std::string get_parent_path(const std::string& path) override;
    std::string get_current_path() override;
};