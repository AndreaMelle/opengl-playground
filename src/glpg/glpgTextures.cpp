#include <glpgTextures.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glpgExt.h"

#define FOURCC_DXT1 0x31545844 // "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // "DXT5" in ASCII

extern GLuint glpg::loadDDS(const char* filepath)
{
    unsigned char header[124];
    
    FILE *fp;
    
    fp = fopen(filepath, "rb");
    
    if(fp == NULL)
    {
        printf("%s could not be opened.", filepath);
        return 0;
    }
    
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0)
    {
        fclose(fp);
        printf("%s DDS format error.", filepath);
        return 0;
    }
    
    fread(&header, 124, 1, fp);
    
    //pointer to 8th byte, converted to pointer to uint, then read the uint
    //technique to interpret raw memory
    unsigned int height         = *(unsigned int*)&(header[8]);
    unsigned int width          = *(unsigned int*)&(header[12]);
    unsigned int linearSize     = *(unsigned int*)&(header[16]);
    unsigned int mipmapCount    = *(unsigned int*)&(header[24]);
    unsigned int fourCC         = *(unsigned int*)&(header[80]);
    
    unsigned char* buffer;
    unsigned int buffsize;
    
    // all mipmaps take as much as the first level
    buffsize = mipmapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(buffsize * sizeof(unsigned char));
    fread(buffer, 1, buffsize, fp);
    
    fclose(fp);
    
    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    
    switch (fourCC) {
        case FOURCC_DXT1:
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            break;
        case FOURCC_DXT3:
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case FOURCC_DXT5:
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        default:
            free(buffer);
            printf("%s DDS unsupported format.", filepath);
            return 0;
    }
    
    GLuint texID;
    glGenTextures(1, &texID);
    
    glBindTexture(GL_TEXTURE_2D, texID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;
    
    for(unsigned int level = 0; level < mipmapCount && (width || height); ++level)
    {
        unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);
        
        offset += size;
        width /= 2;
        height /= 2;
        
        if(width < 1) width = 1;
        if (height < 1) height = 1;
    }
    
    free(buffer);
    return  texID;
}
