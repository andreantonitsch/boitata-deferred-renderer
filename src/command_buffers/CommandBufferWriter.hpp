#pragma once

#include <vector>

#include "CommandBufferWriterStructs.hpp"

namespace boitatah::command_buffers{

    template<class T>
    class CommandBufferWriter {
        
        friend T;

        protected:
            CommandWriterTraits<T>::CommandBufferType   m_buffer;
            CommandWriterTraits<T>::SemaphoreType       m_signal;
            CommandWriterTraits<T>::FenceType           m_fence;
            std::vector<typename CommandWriterTraits<T>::SemaphoreType> m_wait;

        public:
            using CommandBufferType =           typename CommandWriterTraits<T>::CommandBufferType;
            using SemaphoreType =               typename CommandWriterTraits<T>::SemaphoreType;
            using FenceType =                   typename CommandWriterTraits<T>::FenceType;

            using BeginCommand =                typename CommandWriterTraits<T>::BeginCommand;
            using ResetCommand =                typename CommandWriterTraits<T>::ResetCommand;
            using EndCommand =                  typename CommandWriterTraits<T>::EndCommand;
            using SubmitCommand =               typename CommandWriterTraits<T>::SubmitCommand;

            using BeginRenderpassCommand =      typename CommandWriterTraits<T>::BeginRenderpassCommand;
            using EndRenderpassCommand =        typename CommandWriterTraits<T>::EndRenderpassCommand;
            using BindPipelineCommand =         typename CommandWriterTraits<T>::BindPipelineCommand;
            using BindIndexBufferCommand =      typename CommandWriterTraits<T>::BindIndexBufferCommand;
            using BindVertexBufferCommand =     typename CommandWriterTraits<T>::BindVertexBufferCommand;
            using BindSetCommand =              typename CommandWriterTraits<T>::BindSetCommand;

            using DrawCommand =                 typename CommandWriterTraits<T>::DrawCommand;

            using CopyImageCommand =            typename CommandWriterTraits<T>::CopyImageCommand;
            using CopyBufferCommand =           typename CommandWriterTraits<T>::CopyBufferCommand;
            using TransitionLayoutCommand =     typename CommandWriterTraits<T>::TransitionLayoutCommand;
            using CopyBufferToImageCommand =    typename CommandWriterTraits<T>::CopyBufferToImageCommand;

            T& self(){return *static_cast<T*>(this);};

            void set_commandbuffer(CommandBufferType buffer) {
                m_buffer = buffer;
            };

            void setFence(FenceType fence) {
                m_fence = fence;
            };

            void setSignal(SemaphoreType semaphore) {
                m_signal = semaphore;
            };
            void setWait(std::vector<SemaphoreType> semaphores) {
                m_wait = semaphores;
            };

            void begin(const BeginCommand &command) {
                self().__imp_begin(command, m_buffer);
            };

            void reset(const ResetCommand &command) {
                self().__imp_reset(command, m_buffer);
            };

            void end(const EndCommand &command) {
                self().__imp_end(command, m_buffer);
            };

            void submit(const SubmitCommand &command) {
                self().__imp_submit(command, m_buffer);
            };

            void begin_renderpass(const BeginRenderpassCommand &command) {
                self().__imp_begin_renderpass(command, m_buffer);
            };

            void end_renderpass(const EndRenderpassCommand &command) {
                self().__imp_end_renderpass(command, m_buffer);
            }

            void draw(const DrawCommand &command) {
                self().__imp_draw(command, m_buffer);
            };

            void bind_set(const BindSetCommand &command){
                self().__imp_bind_set(command, m_buffer);
            };

            void bind_vertexbuffers(const BindVertexBufferCommand &command) {
                self().__imp_bind_vertexbuffer(command, m_buffer);
            };

            void bind_indexbuffer(const BindIndexBufferCommand &command) {
                self().__imp_bind_indexbuffer(command, m_buffer);
            };
            
            void bind_pipeline(const BindPipelineCommand &command) {
                self().__imp_bind_pipeline(command, m_buffer);
            }

            void copy_image(const CopyImageCommand &command) {
                self().__imp_copy_image(command, m_buffer);
            };

            void copy_buffer(const CopyBufferCommand &command) {
                self().__imp_copy_buffer(command, m_buffer);
            };
            
            bool checkTransfers() {
                return self().__imp_check_transfers();
            }

            void waitForTransfers(){
                self().__imp_wait_for_transfers();
            }

            void transition_image(const TransitionLayoutCommand &command) {
                self().__imp_transition_image(command, m_buffer);
            }

            void copy_buffer_to_image(const CopyBufferToImageCommand& command) {
                self().__imp_copy_buffer_to_image(command, m_buffer);
            }


            CommandWriterTraits<T>::SemaphoreType* get_signal(){return &(self().m_signal);}

    };

};

