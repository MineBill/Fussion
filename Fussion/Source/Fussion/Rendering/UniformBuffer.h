#pragma once
#include <Fussion/GPU/GPU.h>

#include "tracy/Tracy.hpp"

namespace Fussion {
    template<typename T>
    class UniformBuffer {
        explicit UniformBuffer(GPU::Device const& device, Maybe<std::string_view> const& label)
            : m_device(device)
        {
            auto buffer_spec = GPU::BufferSpec {
                .Label = label.value_or("Uniform Buffer<T>"),
                .Usage = GPU::BufferUsage::Uniform | GPU::BufferUsage::CopyDst,
                .Size = sizeof(T),
                .Mapped = false,
            };

            m_buffer = m_device.CreateBuffer(buffer_spec);
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
            VERIFY(m_buffer.Handle != nullptr, "Ensure you created the buffer with UnifromBuffer<T>::Create");

            m_device.WriteBuffer(m_buffer, 0, &Data, sizeof(T));
        }

        static size_t size() { return TypeSize; }

        GPU::Buffer const& buffer() const
        {
            VERIFY(m_buffer.Handle != nullptr, "Ensure you created the buffer with UnifromBuffer<T>::Create");
            return m_buffer;
        }

        T Data {};

    private:
        GPU::Buffer m_buffer {};

        GPU::Device m_device {};
    };
}
