#pragma once

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace boitatah{

    struct CameraUniforms {
        glm::mat4 vp;
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec3 viewPos;
        float aspect;
    };

    struct CameraDesc{
        glm::vec3 lookAtTarget = glm::vec3(0.0,0.0,0.0);
        glm::vec3 position = glm::vec3(0.0,0.0,-10.0);
        float far = 100.0f;
        float near = 0.1f;
        float aspect = 16.0/10.0;
        float fov = glm::radians(90.0);
    };


    class Camera{
        public:
            Camera(const CameraDesc& desc);
            CameraUniforms getCameraUniforms();
            void translate(glm::vec3 direction);
            void setPosition(glm::vec3 position);
            void lookAt(glm::vec3 target);
            void rotate(glm::quat rotation);
            void rotate(glm::vec3 eulerAngles);
            void roll(float roll);
            void orbit(glm::vec3 point, glm::vec3 axis);
            glm::mat4 getProjection() ;
            glm::mat4 getView() ;
            glm::vec3 getDirection() ;
            void updateView();
            void updateProj();
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
            glm::vec3 m_direction;
            glm::mat4 m_projection;
            glm::mat4 m_view;
            
            bool m_dirty_view = true;
            bool m_dirty_proj = true;
            void dirty_view();
            void dirty_proj();
            void updateMatrices();
    };

};

