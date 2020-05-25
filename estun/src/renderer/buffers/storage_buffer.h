#pragma once

#include "renderer/common.h"
#include "renderer/context/command_buffers.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/device_memory.h"
#include "renderer/buffers/buffer.h"
#include "renderer/material/descriptable.h"
#include "renderer/buffers/vertex.h"
#include "renderer/model.h"

namespace estun
{

    //class Vertex;
    //class Buffer;
    //class DeviceMemory;

    template <class T>
    class StorageBuffer : public Descriptable
    {
    public:
        StorageBuffer(const StorageBuffer &) = delete;
        StorageBuffer &operator=(const StorageBuffer &) = delete;
        StorageBuffer &operator=(StorageBuffer &&) = delete;

        StorageBuffer(const std::vector<T> &storage,
                      VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) // | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        {
            const auto bufferSize = sizeof(storage[0]) * storage.size();
            buffer.reset(new Buffer(bufferSize, usage));
            memory.reset(new DeviceMemory(buffer->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true)));
            buffer->CopyFromStagingBuffer<T>(storage);
        }

        StorageBuffer(StorageBuffer &&other) noexcept
            : buffer(other.buffer.release()),
              memory(other.memory.release())
        {
        }

        ~StorageBuffer()
        {
            buffer.reset();
            memory.reset();
        }

        DescriptableInfo GetInfo() override
        {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = buffer->GetBuffer();
            bufferInfo.range = VK_WHOLE_SIZE;

            DescriptableInfo info;
            info.bI = bufferInfo;

            return info;
        }

        VkDeviceAddress GetDeviceAddress() const
        {
            return buffer->GetDeviceAddress();
        }

        const Buffer &GetBuffer() const
        {
            return *buffer;
        }

    protected:
        std::unique_ptr<Buffer> buffer;
        std::unique_ptr<DeviceMemory> memory;
    };

    class VertexBuffer : public StorageBuffer<Vertex>
    {
    public:
        VertexBuffer(const std::vector<Vertex> &storage)
            : StorageBuffer(storage, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {}
        void Bind(VkCommandBuffer &commandBuffer)
        {
            VkBuffer vertexBuffers[] = {buffer->GetBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        }
    };

    class IndexBuffer : public StorageBuffer<uint32_t>
    {
    public:
        IndexBuffer(const std::vector<uint32_t> &storage)
            : StorageBuffer(storage, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {}
        void Bind(VkCommandBuffer &commandBuffer)
        {
            const VkBuffer indexBuffer = buffer->GetBuffer();
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    };

} // namespace estun
