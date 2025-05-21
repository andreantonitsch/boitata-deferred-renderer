#include <boitatah/modules/Camera.hpp>

#include <iostream>
namespace boitatah
{
    Camera::Camera(const CameraDesc &desc) : m_aspect(desc.aspect),
                                             m_fov(desc.fov),
                                             m_nearPlane(desc.near),
                                             m_farPlane(desc.far),
                                             m_position(desc.position)
    {
        m_rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
        m_direction = glm::vec3(0, 0, 1);
        m_up = glm::vec3(0, 1, 0);
    }

    CameraUniforms Camera::getCameraUniforms()
    {
        updateMatrices();
        return CameraUniforms{
            .vp = getProjection()* getView() ,
            .projection = getProjection(),
            .view = getView(),
            .viewPos = m_position,
            .aspect = m_aspect,
        };
    }

    void Camera::translate(glm::vec3 direction)
    {
        dirty_view();
        m_position += direction;
    }

    void Camera::setPosition(glm::vec3 position)
    {
        dirty_view();
        m_position = position;
    }

    void Camera::lookAt(glm::vec3 target)
    {
        dirty_view();
        m_direction = glm::normalize(target - m_position);
    }

    void Camera::rotate(glm::quat rotation)
    {
        dirty_view();
        m_direction = rotation * m_direction;
    }

    void Camera::rotate(glm::vec3 eulerAngles)
    {
        dirty_view();
        m_direction = glm::rotate(m_rotation, glm::length(eulerAngles), glm::normalize(eulerAngles)) * m_direction;
    }

    void Camera::roll(float rollAngle)
    {
        dirty_view();
        glm::vec3 direction = glm::normalize(getDirection());
        glm::quat roll_quat = glm::angleAxis(rollAngle, direction);
        m_up = roll_quat * m_up;
    }

    void Camera::orbit(glm::vec3 point, glm::vec3 axis)
    {
        //TODO not implemented yet
    }

    glm::mat4 Camera::getProjection()
    {
        if (!m_dirty_proj)
        {
            return m_projection;
        }
        updateMatrices();
        return m_projection;
    }

    glm::mat4 Camera::getView()
    {
        if (!m_dirty_view)
        {
            return m_view;
        }
        updateMatrices();
        return m_view;
    }

    glm::vec3 Camera::getDirection()
    {
        // Z is foward.
        //glm::vec3 direction(0.0f, 0.0f, 1.0f);
        //auto dir = m_rotation * direction;
        return m_direction;
    }

    void Camera::setFar(float far)
    {
        dirty_proj();
        m_farPlane = far;
    }
    
    void Camera::setNear(float near)
    {
        dirty_proj();
        m_nearPlane = near;
    }

    void Camera::setFoV(float fov)
    {
        dirty_proj();
        m_fov = fov;
    }

    void Camera::setAspect(float aspect)
    {
        dirty_proj();
        m_aspect = aspect;
    }

    void Camera::dirty_view()
    {
        m_dirty_view = true;
    }

    void Camera::dirty_proj()
    {
        m_dirty_proj = true;
    }

    void Camera::updateView(){
        m_view = glm::lookAtLH(m_position,
                              m_position + getDirection(),
                              m_up);
        m_dirty_view = false;
    }
    void Camera::updateProj() {
        m_projection = glm::perspectiveLH(m_fov, m_aspect, m_farPlane, m_nearPlane); //m_nearPlane, m_farPlane);
        m_dirty_proj = false;
    };

    void Camera::updateMatrices()
    {
        if(m_dirty_view)
            updateView();
        if(m_dirty_proj)
            updateProj();
        
    }
}