#include <glpgApp.h>

using namespace glpg;

class helloApp : public App
{
    virtual void settings()
    {
        mTitle = "Hello App";
    }

    virtual void setup()
    {

    }

    virtual void render(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, green);
    }

    virtual void shutdown()
    {

    }
};

DECLARE_MAIN(helloApp)
