#pragma once
#include "Buffer.h"
#include "Device.h"

namespace Engin5
{
    template<typename T>
    class UniformBuffer
    {
        explicit UniformBuffer(std::string const& label)
        {
            const auto buffer_spec = BufferSpecification{
                .Label = label,
                .Usage = BufferUsage::Uniform,
                .Size = sizeof(T),
                .Mapped = true,
            };
            m_Buffer = Device::Instance()->CreateBuffer(buffer_spec);
        }
    public:
        UniformBuffer() = default;

        static UniformBuffer Create(std::string const& label = "Uniform Buffer")
        {
            return UniformBuffer(label);
        }

        void Flush()
        {
            m_Buffer->SetData(&Data, sizeof(T));
        }

        Ref<Buffer> const& GetBuffer() const { return m_Buffer; }

        T Data{};
    private:
        Ref<Buffer> m_Buffer{};
    };
}