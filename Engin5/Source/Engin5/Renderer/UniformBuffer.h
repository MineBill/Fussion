#pragma once
#include "Buffer.h"
#include "Device.h"
#include "Vulkan/Common.h"

namespace Engin5
{
    template<typename T>
    class UniformBuffer
    {
    public:
        UniformBuffer()
        {
            const auto buffer_spec = BufferSpecification{
                .Label = "Uniform Buffer",
                .Usage = BufferUsage::Uniform,
                .Size = sizeof(T),
                .Mapped = true,
            };
            m_Buffer = Device::Instance()->CreateBuffer(buffer_spec);
        }

        ~UniformBuffer()
        {

        }

        void Flush()
        {
            m_Buffer->SetData(&m_Data, sizeof(T));
        }

        T& GetData() { return m_Data; }
        Ref<Buffer> const& GetBuffer() const { return m_Buffer; }
    private:
        Ref<Buffer> m_Buffer{};
        T m_Data{};
    };
}