#ifndef __GLPG_APP_H__
#define __GLPG_APP_H__

#include "GL/gl3w.h"

#ifdef WIN32
  #define _CRT_SECURE_NO_WARNINGS 1
  #define WIN32_LEAN_AND_MEAN 1
  #include <Windows.h>
#else
  #include <unistd.h>
  #define Sleep(t) sleep(t)
#endif



#define GLFW_NO_GLU 1
#define GLFW_INCLUDE_GLCOREARB 1

#include "GLFW/glfw3.h"

#include "glpgExt.h"

#include <stdio.h>
#include <string>
#include <math.h>

namespace glpg
{
  class App
  {
  public:
    App() { }
    virtual ~App() { }
    void run();

    // gives a chance to override settings?
    // gets called before the gl initialization
    virtual void settings() { }
    virtual void setup() { }
    virtual void render(double currentTime) { }
    virtual void shutdown() { }

    // event callbacks
    virtual void onResize(const int& width, const int& height) { }
    virtual void onKey(const int& key, const int& action) { }
    virtual void onMouseButton(const int& button, const int& action) { }
    virtual void onMouseMove(const int& x, const int& y) { }
    virtual void onMouseWheel(const int& pos) { }

    // setters
    void setWindowTitle(const std::string& title);

    // getters - should use a Point structure eventually...
    void getMousePosition(int& x, int& y) const;

  protected:
    void init();

    void handleResize(const int& width, const int& height);
    void setVsync(bool enable);
    void handleDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message);

    static void glfw_onResize(GLFWwindow* window, int w, int h);
    static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void glfw_onMouseMove(GLFWwindow* window, double x, double y);
    static void glfw_onMouseWheel(GLFWwindow* window, double xoffset, double yoffset);

    static void APIENTRY debug_callback(GLenum source,
                                        GLenum type,
                                        GLuint id,
                                        GLenum severity,
                                        GLsizei length,
                                        const GLchar* message,
                                        GLvoid* userParam);

  protected:
    static glpg::App* gApp;
    
    std::string mTitle;
    unsigned int mWindowWidth;
    unsigned int mWindowHeight;
    unsigned int mGLMajorVersion;
    unsigned int mGLMinorVersion;
    unsigned int mSamples;

    unsigned int mFullscreen;
    unsigned int mVsync;
    unsigned int mCursor;
    unsigned int mStereo;
    unsigned int mDebug;
    unsigned int mRobust;

    GLFWwindow* mWindowHandle;
  };
};

#if defined _WIN32
#define DECLARE_MAIN(APP_TYPE)                      \
glpg::App *app = 0;                                 \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    APP_TYPE* app = new APP_TYPE;                   \
    app->run();                                     \
    delete app;                                     \
    return 0;                                       \
}
#elif defined _LINUX || defined __APPLE__
#define DECLARE_MAIN(APP_TYPE)                      \
int main(int argc, const char ** argv)              \
{                                                   \
    APP_TYPE* app = new APP_TYPE;                   \
    app->run();                                     \
    delete app;                                     \
    return 0;                                       \
}
#else
#error Platform not supported.
#endif

#endif //__GLPG_APP_H__
