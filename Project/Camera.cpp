#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUp));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //return the camera target
    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    //return the camera position
    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    };

    //return the camera front direction
    glm::vec3 Camera::getCameraFrontDirection()
    {
        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        return this->cameraFrontDirection;
    };

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        if (direction == MOVE_FORWARD)
        {
            this->cameraPosition += this->cameraFrontDirection * speed;
            this->cameraTarget += this->cameraFrontDirection * speed;
        }
        if (direction == MOVE_BACKWARD)
        {
            this->cameraPosition -= this->cameraFrontDirection * speed;
            this->cameraTarget -= this->cameraFrontDirection * speed;
        }
        if (direction == MOVE_RIGHT)
        {
            this->cameraPosition += this->cameraRightDirection * speed;
            this->cameraTarget += this->cameraRightDirection * speed;
        }
        if (direction == MOVE_LEFT)
        {
            this->cameraPosition -= this->cameraRightDirection * speed;
            this->cameraTarget -= this->cameraRightDirection * speed;
        }
        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw)
    {
        this->cameraTarget.x = this->cameraPosition.x + cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraTarget.y = this->cameraPosition.y + sin(glm::radians(pitch));
        this->cameraTarget.z = this->cameraPosition.z + sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    // The scene showcase Animation
    void Camera::moveAnimation(glm::vec3 position, glm::vec3 direction)
    {
        this->cameraPosition = position;
        this->cameraTarget = direction;
        this->cameraFrontDirection = glm::normalize(direction - position);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUpDirection));
    }
}