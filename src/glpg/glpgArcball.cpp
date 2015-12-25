#include <glpgArcball.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

using namespace glpg;

Arcball::Arcball(const int& width, const int& height)
    : mIsDown(false)
{
    mSSWidth = (float)width;
    mSSHeight = (float)height;
    mMouseCurrentPos = glm::vec2(0,0);
    mMouseLastPos = glm::vec2(0,0);
}

Arcball::~Arcball()
{
    
}

void Arcball::onMouseButton(const int& button, const int& action, const int& x, const int& y)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        mMouseLastPos = mMouseCurrentPos = glm::vec2((float)x, (float)y);
        mIsDown = true;
    }
    else
    {
        mIsDown = false;
    }
}

void Arcball::onMouseMove(const int& x, const int& y)
{
    if(mIsDown)
    {
        mMouseCurrentPos = glm::vec2((float)x, (float)y);
    }
}

void Arcball::onResize(const int& width, const int& height)
{
    mSSWidth = (float)width;
    mSSHeight = (float)height;
}

void Arcball::update(const glm::mat4& viewTransform, glm::mat4& object2World)
{
    if (mMouseCurrentPos != mMouseLastPos)
    {
        glm::vec3 va = getArcballVector(getWindowCoords(mMouseLastPos));
        glm::vec3 vb = getArcballVector(getWindowCoords(mMouseCurrentPos));
        
        float angle = acosf(glm::min(1.0f, glm::dot(va, vb)));
        
        // given where we take the point (screen space) this can only be in camera space
        glm::vec3 axisCameraSpace = glm::cross(va, vb);
        
        glm::mat3 camera2object = glm::inverse(glm::mat3(viewTransform) * glm::mat3(object2World));
        
        glm::vec3 axisObjectCoord = camera2object * axisCameraSpace;
        
        object2World = glm::rotate(object2World, angle, axisObjectCoord);
        
        mMouseLastPos = mMouseCurrentPos;
    }
}

// get point on sphere given window coordinates cursor position
glm::vec3 Arcball::getArcballVector(const glm::vec3& wPos)
{
    glm::vec3 P = wPos;
    P[1] = -P[1];
    
    // along radius
    float OPsq = P[0] * P[0] + P[1] * P[1];
    
    // if distance along radius falls outside the sphere, we need to normalize to the (unit) sphere
    if(OPsq <= 1.0f)
    {
        P[2] = sqrtf(1.0f - OPsq);
    }
    else
    {
        P = glm::normalize(P);
    }
    
    return P;
}

// returns coordinates in [-1.0, 1.0] space
glm::vec3 Arcball::getWindowCoords(const glm::vec2& pos)
{
    return glm::vec3((pos[0] / mSSWidth - 0.5f) * 2.0f,
                      (pos[1] / mSSHeight - 0.5f) * 2.0f,
                      0);
}