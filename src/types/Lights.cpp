#include "Lights.hpp"
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{
    uint32_t LightArray::addLight(Light &&light)
    {
        light.index = active_lights;
        light_content[light.index] = light;
        //light_index[active_lights] = active_lights;
        active_lights++;
        return light.index;
    }

    void LightArray::update()
    {
        m_manager->getResource(m_buffer).copyData(light_content.data(),
                                                  active_lights * sizeof(Light));
    }

}