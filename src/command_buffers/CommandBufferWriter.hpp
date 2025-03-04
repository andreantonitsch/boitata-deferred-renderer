#pragma once

#include "CommandBufferWriterStructs.hpp"

namespace boitatah::command_buffers{

    template<class T>
    class CommandBufferWriter {
        
        friend T;

        protected:
            CommandWriterTraits<T>::CommandBufferType   m_buffer;
            CommandWriterTraits<T>::SemaphoreType       m_signal;
            CommandWriterTraits<T>::SemaphoreType       m_wait;
            CommandWriterTraits<T>::FenceType           m_fence;

        public:
            using CommandBufferType =           typename CommandWriterTraits<T>::CommandBufferType;
            using SemaphoreType =               typename CommandWriterTraits<T>::SemaphoreType;
            using FenceType =                   typename CommandWriterTraits<T>::FenceType;
            using BeginCommand =                typename CommandWriterTraits<T>::BeginCommand;
            using ResetCommand =                typename CommandWriterTraits<T>::ResetCommand;
            using EndCommand =                  typename CommandWriterTraits<T>::EndCommand;
            using SubmitCommand =               typename CommandWriterTraits<T>::SubmitCommand;
            using DrawCommand =                 typename CommandWriterTraits<T>::DrawCommand;
            using BindPipelineCommand =         typename CommandWriterTraits<T>::BindPipelineCommand;
            using CopyImageCommand =            typename CommandWriterTraits<T>::CopyImageCommand;
            using CopyBufferCommand =           typename CommandWriterTraits<T>::CopyBufferCommand;
            using TransitionLayoutCommand =     typename CommandWriterTraits<T>::TransitionLayoutCommand;
            using CopyBufferToImageCommand =    typename CommandWriterTraits<T>::CopyBufferToImageCommand;

            T& self(){return *static_cast<T*>(this);};

            void setCommandBuffer(CommandBufferType buffer){
                m_buffer = buffer;
            };

            void setFence(FenceType fence){
                m_fence = fence;
            };

            void setSignal(SemaphoreType semaphore){
                m_signal = semaphore;
            };
            void setWait(SemaphoreType semaphore){
                m_wait = semaphore;
            };

            void begin(const BeginCommand &command){
                self().__imp_begin(command, m_buffer);
            };

            void reset(const ResetCommand &command){
                self().__imp_reset(command, m_buffer);
            };

            void end(const EndCommand &command){
                self().__imp_end(command, m_buffer);
            };

            void submit(const SubmitCommand &command){
                self().__imp_submit(command, m_buffer);
            };

            void draw(const DrawCommand &command){
                self().__imp_draw(command, m_buffer);
            };

            void bindPipeline(const BindPipelineCommand &command){
                self().__imp_bindPipeline(command, m_buffer);
            };
            
            void copyImage(const CopyImageCommand &command){
                self().__imp_copyImage(command, m_buffer);
            };

            void copyBuffer(const CopyBufferCommand &command){
                self().__imp_copyBuffer(command, m_buffer);
            };
            
            void transitionLayout(const TransitionLayoutCommand  &command){
                self().__imp_transitionLayout(command, m_buffer);
            };

            bool checkTransfers(){
                return self().__imp_checkTransfers();
            }

            void waitForTransfers(){
                self().__imp_waitForTransfers();
            }

            void transitionImage(const TransitionLayoutCommand &command){
                self().__imp_transitionImage(command, m_buffer);
            }

            void copyBufferToImage(const CopyBufferToImageCommand& command){
                self().__imp_copyBufferToImage(command, m_buffer);
            }

            CommandWriterTraits<T>::SemaphoreType* getSignal(){return &(self().m_signal);}

    };

};

