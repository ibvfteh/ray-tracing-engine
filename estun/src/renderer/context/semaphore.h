#pragma once

#include "renderer/common.h"

namespace estun
{

class Semaphore
{
public:
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

    explicit Semaphore();
    Semaphore(Semaphore &&other) noexcept;
    ~Semaphore();

    VkSemaphore GetSemaphore() const;

private:
    VkSemaphore semaphore;
};

} // namespace estun