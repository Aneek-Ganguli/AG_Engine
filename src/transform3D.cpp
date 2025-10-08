#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Transform3D.hpp"
#include "Window.hpp"

using namespace AG_Engine;

Transform::Transform(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle)
:position(p_position),rotate(p_rotation),scale(p_scale),rotateAngle(glm::radians(p_rotateAngle)),M(NULL),transform(NULL){};

struct UBO* Transform::getUBOData() {
    return &transform;
}

Uint32 Transform::getUBOSize() {
    return sizeof(transform);
}

void Transform::translate(glm::mat4  view,glm::mat4 projection,float deltaTime) {


    M = glm::mat4(1.0f);


    M = glm::translate(M, position);
    // scale
    M = glm::scale(M, scale);
    glm::vec3 rot0 = {0,0,0};
    if (rotate != rot0 || rotateAngle != 0.0f) {
        M = glm::rotate(M, rotateAngle * deltaTime, rotate);
    }

    transform.mvp = projection * view * M;
}

Transform::Transform(vec2 p_position, vec2 p_rotation, vec2 p_scale, float p_rotateAngle) {
    position.x = p_position.x;
    position.y = p_position.y;
    rotateAngle = p_rotateAngle;
    scale.x = p_scale.x;
    scale.y = p_scale.y;
    rotateAngle = p_rotateAngle;
}


bool Transform::checkCollisionX(const Transform& otherObject) {
    return checkCollision(otherObject) &&
           (position.x < otherObject.position.x + otherObject.scale.x &&
            position.x + scale.x > otherObject.position.x);
}

bool Transform::checkCollisionY(const Transform& otherObject) {
    return checkCollision(otherObject) &&
           (position.y < otherObject.position.y + otherObject.scale.y &&
            position.y + scale.y > otherObject.position.y);
}

bool Transform::checkCollisionZ(const Transform& otherObject) {
    return checkCollision(otherObject) &&
           (position.z < otherObject.position.z + otherObject.scale.z &&
            position.z + scale.z > otherObject.position.z);
}


bool Transform::checkCollision(const Transform& otherObject) {
    return (position.x < otherObject.position.x + otherObject.scale.x &&
            position.x + scale.x > otherObject.position.x &&
            position.y < otherObject.position.y + otherObject.scale.y &&
            position.y + scale.y > otherObject.position.y &&
            position.z < otherObject.position.z + otherObject.scale.z &&
            position.z + scale.z > otherObject.position.z);
}

