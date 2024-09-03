#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    template<typename T>
    class UniformBuffer {
        explicit UniformBuffer(GPU::Device const& device, Maybe<std::string_view> const& label) : m_Device(device)
        {
            auto buffer_spec = GPU::BufferSpec{
                .Label = label.ValueOr("Uniform Buffer<T>"),
                .Usage = GPU::BufferUsage::Uniform | GPU::BufferUsage::CopyDst,
                .Size = sizeof(T),
                .Mapped = false,
            };

            m_Buffer = m_Device.CreateBuffer(buffer_spec);
        }

    public:
        constexpr static size_t TypeSize = sizeof(T);

        UniformBuffer() = default;

        static UniformBuffer Create(GPU::Device const& device, Maybe<std::string_view> label = None())
        {
            return UniformBuffer(device, label);
        }

        void Flush()
        {
            VERIFY(m_Buffer.Handle != nullptr, "Ensure you created the buffer with UnifromBuffer<T>::Create");

            m_Device.WriteBuffer(m_Buffer, 0, &Data, sizeof(T));
        }

        size_t Size() const { return TypeSize; }

        GPU::Buffer const& GetBuffer() const { return m_Buffer; }

        T Data{};

    private:
        GPU::Buffer m_Buffer{};

        GPU::Device m_Device{};
    };
}
