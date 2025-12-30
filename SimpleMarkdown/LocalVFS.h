#pragma once
#include "VirtualFileSystem.h"

class LocalVFS : public VirtualFileSystem
{


public:
    LocalVFS() = default;

    std::vector<uint8_t> get_binary(const std::string& path) override;

};