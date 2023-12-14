#ifndef BOITATAH_CAMERA_HPP
#define BOITATAH_CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
namespace boitatah{

    struct CameraDesc{
        glm::vec3 lookAtTarget = glm::vec3(0.0,0.0,0.0);
        glm::vec3 position = glm::vec3(0.5,0.5,0.5);
        float far = 100.0f;
        float near = 0.001f;
        float aspect = 16.0/10.0;
        float fov = glm::radians(27);
    };

    class Camera{
        public:
            Camera(const CameraDesc& desc);
            void translate(glm::vec3 direction);
            void lookAt(glm::vec3 target);
            void rotate(glm::quat rotation);
            void rotate(glm::vec3 eulerAngles);
            void roll(float roll);
            glm::mat4 getProjection() ;
            glm::mat4 getView() ;
            glm::vec3 getDirection() ;
            void setFar(float far);
            void setNear(float near);
            void setFoV(float fov);
            void setAspect(float aspect);
            bool perspective = true;
        private:
            float m_fov;
            float m_aspect;
            float m_farPlane;
            float m_nearPlane;
            glm::vec3 m_position;
            glm::quat m_rotation;
            glm::vec3 m_up;

            glm::mat4 m_projection;
            glm::mat4 m_view;
            
            bool m_dirty = true;
            void dirty();
            void computeMatrices();
    };

}


#endif //BOITATAH_CAMERA_HPP