#include <thirdparty/glad/include/glad/glad.h>
#include "OpenGLVertexArrayObject.h"

namespace CHR
{
    OpenGLVertexArrayObject::OpenGLVertexArrayObject()
    {
        glGenVertexArrays(1, &m_renderer_id);
    }
    OpenGLVertexArrayObject::~OpenGLVertexArrayObject()
    {
        glDeleteVertexArrays(1, &m_renderer_id);
    }
    void OpenGLVertexArrayObject::Bind() const
    {
        glBindVertexArray(m_renderer_id);
    }
    void OpenGLVertexArrayObject::Unbind() const
    {
        glBindVertexArray(0);
    }
    void OpenGLVertexArrayObject::AddVertexBuffer(std::shared_ptr<OpenGLVertexBuffer> vertex_buffer)
    {
        CH_ASSERT(vertex_buffer->GetBufferLayout().GetElements().size(), "Buffer has no layout!");

        Bind();
        vertex_buffer->Bind();


        const auto& layout = vertex_buffer->GetBufferLayout().GetElements();
        for (VertexAttribute element : layout)
        {
            glVertexAttribPointer(element.shader_layout_index,
                GetShaderDataTypeCount(element.data_type),
                ShaderDataTypeToOpenGLBaseType(element.data_type),
                element.normalized,
                vertex_buffer->GetBufferLayout().GetStride(),
                (const void*)element.offset);

            glEnableVertexAttribArray(element.shader_layout_index);
        }
        m_vertex_buffers.push_back(vertex_buffer);
        Unbind();
    }

    void OpenGLVertexArrayObject::SetIndexBuffer(std::shared_ptr<OpenGLIndexBuffer> index_buffer)
    {
        Bind();
        index_buffer->Bind();

        m_index_buffer = index_buffer;
        Unbind();
    }



    GLenum OpenGLVertexArrayObject::ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
        case CHR::ShaderDataType::Float:    return GL_FLOAT;
        case CHR::ShaderDataType::Float2:   return GL_FLOAT;
        case CHR::ShaderDataType::Float3:   return GL_FLOAT;
        case CHR::ShaderDataType::Float4:   return GL_FLOAT;
        case CHR::ShaderDataType::Mat3:     return GL_FLOAT;
        case CHR::ShaderDataType::Mat4:     return GL_FLOAT;
        case CHR::ShaderDataType::Int:      return GL_INT;
        case CHR::ShaderDataType::Int2:     return GL_INT;
        case CHR::ShaderDataType::Int3:     return GL_INT;
        case CHR::ShaderDataType::Int4:     return GL_INT;
        case CHR::ShaderDataType::Bool:     return GL_BOOL;
        }

        CH_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }
}