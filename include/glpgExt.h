#ifndef __GLPG_EXT_H__
#define __GLPG_EXT_H__

#include "GL/gl3w.h"
#include "GL/glext.h"

GL3WglProc glpgGetProcAddress(const char * funcname);
int glpgIsExtensionSupported(const char * extname);

#endif /* __GLPG_EXT_H__ */
