#include "Lights.hpp"
#include <renderer/modules/GPUResourceManager.hpp>
#include <algorithm>
namespace boitatah{
    LightArray::LightArray(uint32_t light_capacity, Light &&default_light, std::shared_ptr<GPUResourceManager> manager)
    : m_light_capacity(light_capacity),
      m_default_light(default_light),
      m_manager(manager){
        
        light_content.resize(light_capacity);
        std::fill(light_content.begin(), light_content.end(), default_light);
        m_lightmetada = m_manager->create(GPUBufferCreateDescription{
            .size = sizeof(uint32_t) * 100,
            .usage = BUFFER_USAGE::UNIFORM_BUFFER,
            .sharing_mode = SHARING_MODE::EXCLUSIVE
        });

        m_light_buffer = m_manager->create(GPUBufferCreateDescription{
            .size = static_cast<uint32_t>(sizeof(Light)) * m_light_capacity,
            .usage = BUFFER_USAGE::UNIFORM_BUFFER,
            .sharing_mode = SHARING_MODE::EXCLUSIVE
        });

        std::cout << "light array created at buffers " << m_light_buffer.i << " and " << m_lightmetada.i << std::endl;
        m_active_lights = 0;
    };

    uint32_t LightArray::addLight(Light &&light)
    {
        light.index = m_active_lights;
        light_content[light.index] = light;
        //light_index[active_lights] = active_lights;
        m_active_lights++;
        return light.index;
    }

    void LightArray::update()
    {
        m_manager->getResource(m_light_buffer).copyData(light_content.data(),
                                                        m_active_lights * sizeof(Light));
        m_manager->getResource(m_lightmetada).copyData(&m_active_lights, sizeof(uint32_t));
    }

    Handle<GPUBuffer> LightArray::metadata()
    {
        return m_lightmetada;
    }

    Handle<GPUBuffer> LightArray::light_array()
    {
        return m_light_buffer;
    }
}