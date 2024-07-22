#pragma once

namespace Fussion::RHI
{
    enum class WrapMode
    {
        ClampToEdge,
        ClampToBorder,
        Repeat,
        MirrorRepeat,
    };

    enum class FilterMode
    {
        Linear,
        Nearest,
    };

    struct SamplerSpecification
    {
        WrapMode Wrap;
        FilterMode Filter;
        bool UseAnisotropy;
        bool UseDepthCompare;
    };

    class Sampler: public RenderHandle
    {
    public:
        virtual void Destroy() = 0;

        virtual SamplerSpecification GetSpec() = 0;
    };
}