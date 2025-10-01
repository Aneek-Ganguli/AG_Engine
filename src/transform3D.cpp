#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Transform3D.hpp"
#include "Window.hpp"

using namespace AG_Engine;

Transform3D::Transform3D(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle)
:position(p_position),rotate(p_rotation),scale(p_scale),rotateAngle(glm::radians(p_rotateAngle)),M(NULL),transform(NULL){};

UBO* Transform3D::getUBOData() {
    return &transform;
}

Uint32 Transform3D::getUBOSize() {
    return sizeof(transform);
}

void Transform3D::translate(glm::mat4  view,glm::mat4 projection,float deltaTime) {


    M = glm::mat4(1.0f);


    M = glm::translate(M, position);
    // scale
    M = glm::scale(M, scale);
    glm::vec3 rot0 = {0,0,0};
    if (rotate != rot0 || rotateAngle != 0.0f) {
        M = glm::rotate(M, rotateAngle * deltaTime, rotate);
    }

    // multiply P * M -> MVP
    transform.mvp = projection * view * M;
    // print_mat4(M);
}

bool Transform3D::checkCollisionX(Transform3D otherObject) {
    bool xCollision = otherObject.position.x >= position.x && otherObject.position.x <= position.x + scale.x;

    if (xCollision) {
        return true;
    }
    else {
        return false;
    }
}

bool Transform3D::checkCollisionY(Transform3D otherObject) {
    bool yCollision = otherObject.position.y >= position.y && otherObject.position.y <= position.y + scale.y;
    if (yCollision) {
        return true;
    }
    else {
        return false;
    }
}

bool Transform3D::checkCollisionZ(Transform3D otherObject) {
    bool zCollision = otherObject.position.z >= position.z && otherObject.position.z <= position.z + scale.z;
    if (zCollision) {
        return true;
    }
    else {
        return false;
    }
}

bool Transform3D::checkCollision(Transform3D otherObject) {
    bool zCollision = otherObject.position.z >= position.z && otherObject.position.z <= position.z + scale.z;
    bool yCollision = otherObject.position.y >= position.y && otherObject.position.y <= position.y + scale.y;
    bool xCollision = otherObject.position.x >= position.x && otherObject.position.x <= position.x + scale.x;
    if (xCollision||yCollision||zCollision) {
        return true;
    }
    else {
        return false;
    }
}