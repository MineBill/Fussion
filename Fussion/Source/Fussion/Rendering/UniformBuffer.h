#pragma once
#include "tracy/Tracy.hpp"

#include <Fussion/GPU/GPU.h>

namespace Fussion {
    template<typename T>
    class UniformBuffer {
        explicit UniformBuffer(GPU::Device const& device, Maybe<std::string_view> const& label)
            : m_device(device)
        {
            auto buffer_spec = GPU::BufferSpec {
                .label = label.value_or("Uniform Buffer<T>"),
                .usage = GPU::BufferUsage::Uniform | GPU::BufferUsage::CopyDst,
                .size = sizeof(T),
                .mapped = false,
            };

            m_buffer = m_device.create_buffer(buffer_spec);
        }

    public:
        constexpr static size_t TypeSize = sizeof(T);

        UniformBuffer() = default;

        static UniformBuffer create(GPU::Device const& device, Maybe<std::string_view> label = None())
        {
            return UniformBuffer(device, label);
        }

        void flush()
        {
            ZoneScopedN("Uniform Buffer Flush");
            VERIFY(m_buffer.handle != nullptr, "Ensure you created the buffer with UnifromBuffer<T>::Create");

            m_device.write_buffer(m_buffer, 0, &data, sizeof(T));
        }

        static size_t size() { return TypeSize; }

        GPU::Buffer const& buffer() const
        {
            VERIFY(m_buffer.handle != nullptr, "Ensure you created the buffer with UnifromBuffer<T>::Create");
            return m_buffer;
        }

        T data {};

    private:
        GPU::Buffer m_buffer {};

        GPU::Device m_device {};
    };
}
