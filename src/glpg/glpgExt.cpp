#include "glpgExt.h"

#include <GL/glext.h>
#include <string.h>

GL3WglProc glpgGetProcAddress(const char * funcname)
{
  return gl3wGetProcAddress(funcname);
}

int glpgIsExtensionSupported(const char * extname)
{
  GLint numExtensions;
  GLint i;

  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

  for (i = 0; i < numExtensions; i++)
  {
    const GLubyte * e = glGetStringi(GL_EXTENSIONS, i);
    if (!strcmp((const char *)e, extname))
    {
      return 1;
    }
  }

  return 0;
}
