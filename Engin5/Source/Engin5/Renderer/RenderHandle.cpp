#include "e5pch.h"
#include "RenderHandle.h"

Engin5::RenderHandle::RenderHandle()
{
    m_Id = 123;
}

bool Engin5::RenderHandle::Equals(const RenderHandle& other) const
{
    return m_Id == other.m_Id;
}

bool Engin5::RenderHandle::Equals(const Ref<RenderHandle>& other) const
{
    return Equals(*other.get());
}