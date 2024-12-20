#ifndef BOITATAH_COMMAND_BUFFER_WRITER_HPP
#define BOITATAH_COMMAND_BUFFER_WRITER_HPP

namespace boitatah::command_buffers{

    template<class T>
    struct WrappedCommandBuffer {};

    template<class T>
    struct  WriterDrawCommand{};

    template<class T>
    struct WriterBeginCommand {};

    template<class T>
    struct WriterResetCommand {};

    template<class T>
    struct WriterBindPipelineCommand {};

    template<class T>
    struct WriterCopyImageCommand {};

    template<class T>
    struct WriterCopyBufferCommand {};

    template<class T>
    struct WriterTransitionLayoutCommand {};


    template<class T>
    class CommandBufferWriter {
        
        friend T;

        protected:
            WrappedCommandBuffer<T> bufferWrapper;

        public:
            T& self(){return *static_cast<T*>(this);};

            void setCommandBuffer(WrappedCommandBuffer<T> buffer){
                bufferWrapper = buffer;
            };

            void begin(WriterBeginCommand<T> &command){
                self().__imp_begin(command);
            };

            void reset(WriterResetCommand<T> &command){
                self().__imp_reset(command);
            };

            void draw(WriterDrawCommand<T> &command){
                self().__imp_draw(command, bufferWrapper);
            };

            void bindPipeline(WriterBindPipelineCommand<T> &command){
                self().__imp_bindPipeline(command, bufferWrapper);
            };
            
            void copyImage(WriterCopyImageCommand<T> &command){
                self().__imp_copyImage(command, bufferWrapper);
            };

            void copyBuffer(WriterCopyBufferCommand<T> &command){
                self().__imp_copyBuffer(command, bufferWrapper);
            };
            
            void transitionLayout(WriterTransitionLayoutCommand<T> &command){
                self().__imp_transitionLayout(command, bufferWrapper);
            };

    };

};


#endif