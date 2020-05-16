#pragma once

#include "renderer/common.h"

namespace estun
{

class ShaderManager
{
private:
    static ShaderManager *instance;

    std::unordered_map<std::string, VkShaderModule> loadedShaders;

public:
    ~ShaderManager();

    void CleanUp();

    static ShaderManager *GetInstance();

    VkShaderModule GetShaderModule(const std::string &filename);

private:
    ShaderManager() = default;

    static std::vector<char> ReadFile(const std::string &filename);

    static VkShaderModule CreateShaderModule(const std::vector<char> &code);
};

} // namespace estun
