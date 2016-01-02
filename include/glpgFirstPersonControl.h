//
//  glpgFirstPersonControl.h
//  opengl-playground
//
//  Created by Andrea Melle on 26/12/2015.
//
//

#ifndef glpgFirstPersonControl_h
#define glpgFirstPersonControl_h

#include <glm/glm.hpp>

namespace glpg
{
    class FirstPersonControl
    {
    public:
        FirstPersonControl();//FirstPersonControl
        virtual ~FirstPersonControl();
        
        void onMouseButton(const int& button, const int& action, const int& x, const int& y);
        void onMouseMove(const int& x, const int& y);
        void onKey(const int& key, const int& action) { }
        //void onResize(const int& width, const int& height);
        
        void update(double currentTime, const int& mouseX, const int& mouseY, glm::mat4& view);
        
    private:
        
        
//        float mSSWidth;
//        float mSSHeight;
//        
//        glm::vec2 mMouseLastPos;
//        glm::vec2 mMouseCurrentPos;
//        bool mIsDown;
        
    };
};


#endif /* glpgFirstPersonControl_h */
