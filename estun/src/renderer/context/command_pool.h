#pragma once

#include "renderer/common.h"

namespace estun
{

enum CommandPoolType
{
    Graphics = 0,
    Compute = 1,
    Transfer = 2,
};

class CommandPool
{
private:
    VkCommandPool commandPool;

public:
    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) = delete;

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = delete;

    CommandPool(CommandPoolType type);
    ~CommandPool();

    VkCommandPool GetCommandPool() const;

    void Reset();
};

class CommandPoolLocator
{
public:
    static CommandPool &GetGraphicsPool();
    static CommandPool &GetComputePool();
    static CommandPool &GetTransferPool();

    static void Provide(
        CommandPool *graphicsCommandPool,
        CommandPool *computeCommandPool,
        CommandPool *transferCommandPool);

private:
    static CommandPool *currGraphicsCommandPool;
    static CommandPool *currComputeCommandPool;
    static CommandPool *currTransferCommandPool;
};

} // namespace estun
