#include "Camera.hpp"

namespace boitatah
{
    Camera::Camera(const CameraDesc &desc) : m_aspect(desc.aspect),
                                             m_fov(desc.fov),
                                             m_nearPlane(desc.near),
                                             m_farPlane(desc.far),
                                             m_position(desc.position)
    {
        lookAt(desc.lookAtTarget);
    }

    void Camera::translate(glm::vec3 direction)
    {
        dirty();
        m_position += direction;
    }

    void Camera::lookAt(glm::vec3 target)
    {
        dirty();
        glm::vec3 direction = glm::normalize(target - m_position);
        float d = glm::dot(direction, m_up);
        if (glm::abs(d) > 0.999)
        {
            m_rotation = glm::quat(1, 0, 0, 0);
            return;
        }
        glm::vec3 a = glm::cross(direction, m_up);

        m_rotation = glm::quat(glm::root_two<float>() + d, a);
    }

    void Camera::rotate(glm::quat rotation)
    {
        dirty();
        m_rotation = rotation * m_rotation;
    }

    void Camera::rotate(glm::vec3 eulerAngles)
    {
        rotate(glm::quat(eulerAngles));
    }

    void Camera::roll(float rollAngle)
    {
        dirty();
        glm::vec3 direction = glm::normalize(getDirection());
        glm::quat roll_quat = glm::angleAxis(rollAngle, direction);
        m_up = roll_quat * m_up;
    }

    glm::mat4 Camera::getProjection()
    {
        if (!m_dirty)
        {
            return m_projection;
        }
        computeMatrices();
        return m_projection;
    }

    glm::mat4 Camera::getView()
    {
        if (!m_dirty)
        {
            return m_view;
        }
        computeMatrices();
        return m_view;
    }

    glm::vec3 Camera::getDirection()
    {
        // Z is foward.
        glm::vec3 direction(0.0f, 0.0f, 1.0f);
        return m_rotation * direction;
    }
    void Camera::setFar(float far)
    {
        dirty();
        m_farPlane = far;
    }
    void Camera::setNear(float near)
    {
        dirty();
        m_nearPlane = near;
    }

    void Camera::setFoV(float fov)
    {
        dirty();
        m_fov = fov;
    }

    void Camera::setAspect(float aspect)
    {
        dirty();
        m_aspect = aspect;
    }

    void Camera::dirty()
    {
        m_dirty = true;
    }

    void Camera::computeMatrices()
    {
        m_view = glm::lookAt(m_position,
                             m_position + getDirection(),
                             m_up);
        m_projection = glm::perspective(m_fov, m_aspect, m_nearPlane, m_farPlane);
    }
}