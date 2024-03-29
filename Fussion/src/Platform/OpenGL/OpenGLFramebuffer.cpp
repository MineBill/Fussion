#include "OpenGLFramebuffer.h"
#include "Fussion/Core/Core.h"
#include <glad/glad.h>

namespace Fussion
{
    OpenGLFrameBuffer::OpenGLFrameBuffer(u32 width, u32 height)
    {
        resize(width, height);
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_id);
    }

    void OpenGLFrameBuffer::resize(u32 width, u32 height)
    {
        FSN_CORE_ASSERT(width > 0, "Width cannot be zero or negative");
        FSN_CORE_ASSERT(height > 0, "Height cannot be zero or negative");

        if (m_id) {
            glDeleteFramebuffers(1, &m_id);
            glDeleteTextures(1, &m_colorTextureId);
        }
        m_width = width;
        m_height = height;
        glCreateFramebuffers(1, &m_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_colorTextureId);
        glBindTexture(GL_TEXTURE_2D, m_colorTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<i32>(width), static_cast<i32>(height), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTextureParameteri(m_colorTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_colorTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureId, 0);

        if (auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
            FSN_CORE_ASSERT(false, "Framebuffer is not complete: {}", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFrameBuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
        glViewport(0, 0, static_cast<i32>(m_width), static_cast<i32>(m_height));
    }

    void OpenGLFrameBuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} // namespace Fussion
