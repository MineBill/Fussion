#include "FussionPCH.h"
#include "RenderHandle.h"

namespace Fussion::RHI {
RenderHandle::RenderHandle()
{
    m_Id = 123;
}

bool RenderHandle::Equals(const RenderHandle& other) const
{
    return m_Id == other.m_Id;
}

bool RenderHandle::Equals(const Ref<RenderHandle>& other) const
{
    return Equals(*other.get());
}
}
