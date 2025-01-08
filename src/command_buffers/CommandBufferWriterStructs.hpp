#ifndef BOITATAH_COMMAND_BUFFER_WRITER_STRUCTS_HPP
#define BOITATAH_COMMAND_BUFFER_WRITER_STRUCTS_HPP

namespace boitatah::command_buffers{

    template<class T>
    class CommandWriterTraits{};

    template<typename T>
    struct WrappedBuffer {
        using Type = CommandWriterTraits<T>::CommandBufferType;
        Type m_buffer;

        Type& unwrap(){
            return m_buffer;
        };
        void set(Type& buffer){
            m_buffer = buffer;
        }
    };
};


#endif