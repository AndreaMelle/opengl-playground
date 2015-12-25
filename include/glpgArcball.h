//
//  glpgArcball.h
//  opengl-playground
//
//  Created by Andrea Melle on 24/12/2015.
//
//

#ifndef glpgArcball_h
#define glpgArcball_h

#include <glm/glm.hpp>

namespace glpg
{
    class Arcball
    {
    public:
        Arcball(const int& width, const int& height);
        virtual ~Arcball();
        
        void onMouseButton(const int& button, const int& action, const int& x, const int& y);
        void onMouseMove(const int& x, const int& y);
        void onResize(const int& width, const int& height);
        
        void update(const glm::mat4& viewTransform, glm::mat4& object2World);
        
    private:
        
        glm::vec3 getWindowCoords(const glm::vec2& pos);
        glm::vec3 getArcballVector(const glm::vec3& wPos);
        
        float mSSWidth;
        float mSSHeight;
        
        glm::vec2 mMouseLastPos;
        glm::vec2 mMouseCurrentPos;
        bool mIsDown;
        
    };
};


#endif /* glpgArcball_h */
