#include "e5pch.h"
#include "RenderHandle.h"

Fussion::RenderHandle::RenderHandle()
{
    m_Id = 123;
}

bool Fussion::RenderHandle::Equals(const RenderHandle& other) const
{
    return m_Id == other.m_Id;
}

bool Fussion::RenderHandle::Equals(const Ref<RenderHandle>& other) const
{
    return Equals(*other.get());
}