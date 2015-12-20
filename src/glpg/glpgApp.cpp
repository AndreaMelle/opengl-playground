#include "glpgApp.h"

using namespace glpg;

App* App::gApp = 0;

void App::run()
{
  bool running = true;
  gApp = this;

  if (!glfwInit())
  {
      fprintf(stderr, "Failed to initialize GLFW\n");
      return;
  }

  init();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mGLMajorVersion);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mGLMinorVersion);

#ifndef _DEBUG
  if (mDebug)
#endif /* _DEBUG */
  {
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  }
  if (mRobust)
  {
      glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, mSamples);
  glfwWindowHint(GLFW_STEREO, mStereo ? GL_TRUE : GL_FALSE);
  
  mWindowHandle = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle.c_str(), mFullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
  if (!mWindowHandle)
  {
      fprintf(stderr, "Failed to open window\n");
      return;
  }
  

  glfwMakeContextCurrent(mWindowHandle);

  glfwSetWindowSizeCallback(mWindowHandle, glfw_onResize);
  glfwSetKeyCallback(mWindowHandle, glfw_onKey);
  glfwSetMouseButtonCallback(mWindowHandle, glfw_onMouseButton);
  glfwSetCursorPosCallback(mWindowHandle, glfw_onMouseMove);
  glfwSetScrollCallback(mWindowHandle, glfw_onMouseWheel);
  if (!mCursor)
  {
      glfwSetInputMode(mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }

  gl3wInit();

#ifdef _DEBUG
  fprintf(stderr, "VENDOR: %s\n", (char *)glGetString(GL_VENDOR));
  fprintf(stderr, "VERSION: %s\n", (char *)glGetString(GL_VERSION));
  fprintf(stderr, "RENDERER: %s\n", (char *)glGetString(GL_RENDERER));
#endif

  if (mDebug)
  {
      if (gl3wIsSupported(4, 3))
      {
          glDebugMessageCallback((GLDEBUGPROC)debug_callback, this);
          glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      }
      else if (glpgIsExtensionSupported("GL_ARB_debug_output"))
      {
          glDebugMessageCallbackARB((GLDEBUGPROC)debug_callback, this);
          glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
      }
  }

  setup();

  do
  {
      render(glfwGetTime());

      glfwSwapBuffers(mWindowHandle);
      glfwPollEvents();

      running &= (glfwGetKey(mWindowHandle, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
      running &= (glfwWindowShouldClose(mWindowHandle) != GL_TRUE);
  } while (running);

  shutdown();

  glfwDestroyWindow(mWindowHandle);
  glfwTerminate();
}

void App::init()
{
  mTitle = "";
  mWindowWidth = 800;
  mWindowHeight = 600;

#ifdef __APPLE__
  mGLMajorVersion = 4;
  mGLMinorVersion = 1;
#else
  mGLMajorVersion = 4;
  mGLMinorVersion = 3;
#endif

  mSamples = 0;
  mCursor = 1;

#ifdef _DEBUG
  mDebug = 1;
#endif
    
    mFullscreen = 0;
    mStereo = 0;

  this->settings();
}

void App::handleResize(const int& width, const int& height)
{
    mWindowWidth = width;
    mWindowHeight = height;
    onResize(mWindowWidth, mWindowHeight);
}

void App::setVsync(bool enable)
{
  mVsync = enable ? 1 : 0;
  glfwSwapInterval((int)mVsync);
}

void App::setWindowTitle(const std::string& title)
{
    glfwSetWindowTitle(mWindowHandle, title.c_str());
}

void App::getMousePosition(int& x, int& y) const
{
    double dx, dy;
    glfwGetCursorPos(mWindowHandle, &dx, &dy);

    x = static_cast<int>(floor(dx));
    y = static_cast<int>(floor(dy));
}

void App::handleDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message)
{
#ifdef _WIN32
  OutputDebugStringA(message);
  OutputDebugStringA("\n");
#endif /* _WIN32 */
}

void App::glfw_onResize(GLFWwindow* window, int w, int h)
{
  gApp->handleResize(w, h);
}

void App::glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  gApp->onKey(key, action);
}

void App::glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
  gApp->onMouseButton(button, action);
}

void App::glfw_onMouseMove(GLFWwindow* window, double x, double y)
{
  gApp->onMouseMove(static_cast<int>(x), static_cast<int>(y));
}

void App::glfw_onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
  gApp->onMouseWheel(static_cast<int>(yoffset));
}

void APIENTRY App::debug_callback(GLenum source
  , GLenum type
  , GLuint id
  , GLenum severity
  , GLsizei length
  , const GLchar* message
  , GLvoid* userParam)
{
    reinterpret_cast<App*>(userParam)->handleDebugMessage(source, type, id, severity, length, message);
}

//        if (info.flags.fullscreen)
//        {
//            if (info.windowWidth == 0 || info.windowHeight == 0)
//            {
//                GLFWvidmode mode;
//                glfwGetDesktopMode(&mode);
//                info.windowWidth = mode.Width;
//                info.windowHeight = mode.Height;
//            }
//
//            glfwOpenWindow(info.windowWidth, info.windowHeight, 8, 8, 8, 0, 32, 0, GLFW_FULLSCREEN);
//            glfwSwapInterval((int)info.flags.vsync);
//        }
//        else
