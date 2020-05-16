#pragma once

#include "renderer/common.h"
#include "includes/glm.h"
#include "renderer/material/descriptable.h"
#include "renderer/buffers/buffer.h"
#include "renderer/device_memory.h"

namespace estun
{

    class Buffer;
    class DeviceMemory;

    template <class T>
    class UniformBuffer : public Descriptable
    {
    public:
        UniformBuffer(const UniformBuffer &) = delete;
        UniformBuffer &operator=(const UniformBuffer &) = delete;
        UniformBuffer &operator=(UniformBuffer &&) = delete;

        explicit UniformBuffer()
        {
            const auto bufferSize = sizeof(T);

            buffer.reset(new Buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
            memory.reset(new DeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
        }

        UniformBuffer(UniformBuffer &&other) noexcept
            : buffer(other.buffer.release()),
              memory(other.memory.release())
        {
        }

        ~UniformBuffer()
        {
            buffer.reset();
            memory.reset();
        }

        DescriptableInfo GetInfo() override
        {
            VkDescriptorBufferInfo uniformBufferInfo = {};
            uniformBufferInfo.buffer = buffer->GetBuffer();
            uniformBufferInfo.range = VK_WHOLE_SIZE;

            DescriptableInfo info;
            info.bI = uniformBufferInfo;

            return info;
        }

        const Buffer &GetBuffer() const
        {
            return *buffer;
        }

        void SetValue(const T &ubo)
        {
            const auto data = memory->Map(0, sizeof(T));
            std::memcpy(data, &ubo, sizeof(ubo));
            memory->Unmap();
        }

    private:
        std::unique_ptr<Buffer> buffer;
        std::unique_ptr<DeviceMemory> memory;
    };

} // namespace estun