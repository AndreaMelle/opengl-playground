#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgPrimitives.h>
#include <iostream>
#include <SOIL.h>

using namespace glpg;
using namespace glpg::math;

GLuint LoadLUT(const char* filename, math::vec2& scaleoffset)
{
    GLuint lutTex;
    
    int lut_width, lut_height, lut_channels;
    unsigned char* lut_data;
    
    unsigned char* lut_data_raw = SOIL_load_image(filename, &lut_width, &lut_height, &lut_channels, SOIL_LOAD_RGB);
    
    int dim = lut_height;
    
    assert(lut_width == dim * dim);
    
    lut_data = (unsigned char*)malloc(dim * dim * dim * 3 * sizeof(unsigned char));
    
    
    for(int i = 0; i < dim; i++)
    {
        for(int j = 0; j < dim; j++)
        {
            for(int k = 0; k < dim; k++)
            {
                int j_ = j;//dim-j-1; // this is not Direct3D
                lut_data[0 + 3*i + 3*(j*dim) + 3*(k*dim*dim)] = lut_data_raw[3*k*dim+ 0 + 3*i+3*j_*dim*dim];
                lut_data[1 + 3*i + 3*(j*dim) + 3*(k*dim*dim)] = lut_data_raw[3*k*dim+ 1 + 3*i+3*j_*dim*dim];
                lut_data[2 + 3*i + 3*(j*dim) + 3*(k*dim*dim)] = lut_data_raw[3*k*dim+ 2 + 3*i+3*j_*dim*dim];
            }
        }
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &lutTex);
    
    glBindTexture(GL_TEXTURE_3D, lutTex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, dim, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, lut_data);
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_3D, 0);
    
    free(lut_data);
    SOIL_free_image_data(lut_data_raw);
    
    float scale = ((float)dim - 1.0) / (float)dim;
    float offset = 1.0 / (2.0 * (float)dim);
    
    scaleoffset = math::vec2(scale, offset);
    
    return lutTex;
}

class lutColorCorrection : public App
{
    GLfloat clearColor[4];
    
    GLuint imageTex;
    GLuint lutTex;
    
    IMeshRenderer *fsQuad;
    float aspect;
    
    struct
    {
        GLuint progID;
        GLint u_Img;
        GLint u_Lut;
        GLint u_Aspect;
        GLint u_ScaleOffset;
        GLint u_apply; //uint
        GLint u_radius; //float
        GLint u_softness; //float
        GLint u_mix_strength; //float
    } matParams;

    virtual void settings()
    {
        mTitle = "LUT Color Correction";
        mSamples = 0;
    }

    virtual void setup()
    {
        
        // Set the clear color
        clearColor[0] = 0.0f; clearColor[1] = 0.0f; clearColor[2] = 0.0f; clearColor[3] = 1.0f;
        
        math::vec2 scaleoffset;
        //lutTex = LoadLUT("../textures/LUTs/amaro_lut.tga", scaleoffset);
        lutTex = LoadLUT("../textures/LUTs/nashville_lut.tga", scaleoffset);
        
        GLuint shaders[2];
        
        shaders[0] = shader::fromFile("../shaders/imgprocessing/fsquad.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/imgprocessing/fsquad.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        matParams.progID = program::linkShaders(shaders, 2, true, true);
        assert(matParams.progID != 0);
        matParams.u_Img = glGetUniformLocation(matParams.progID, "tex");
        matParams.u_Lut = glGetUniformLocation(matParams.progID, "lut");
        matParams.u_Aspect = glGetUniformLocation(matParams.progID, "screen_aspect");
        matParams.u_ScaleOffset = glGetUniformLocation(matParams.progID, "scaleoffset");
        
        matParams.u_apply = glGetUniformLocation(matParams.progID, "vignette.apply");; //uint
        matParams.u_radius = glGetUniformLocation(matParams.progID, "vignette.radius");; //float
        matParams.u_softness = glGetUniformLocation(matParams.progID, "vignette.softness");; //float
        matParams.u_mix_strength = glGetUniformLocation(matParams.progID, "vignette.mix_strength");; //float
        
        glUseProgram(matParams.progID);
        glUniform1i(matParams.u_Img, 0);
        glUniform1i(matParams.u_Lut, 1);
        glUniform2fv(matParams.u_ScaleOffset, 1, scaleoffset);
        glUniform1ui(matParams.u_apply, 1);
        glUniform1f(matParams.u_radius, 0.75f); //0.5 is a circle fitting the screen
        glUniform1f(matParams.u_softness, 0.45f);
        glUniform1f(matParams.u_mix_strength, 0.5f);
        glUseProgram(0);
        
        fsQuad = glpg::NormalizedQuad::Create(3264.0f, 2448.0f);
        assert(NULL != fsQuad);
        
        imageTex = SOIL_load_OGL_texture("../textures/source-1.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        assert(0 != imageTex);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imageTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisable(GL_DEPTH_TEST); //we don't use depth test to render a quad on screen
        
        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
        
        glClearBufferfv(GL_COLOR, 0, clearColor);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imageTex);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, lutTex);
        
        glUseProgram(matParams.progID);
        
        vec2 ar(1.0f, aspect);
        glUniform2fv(matParams.u_Aspect, 1, ar);
        
        fsQuad->render();
        
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case '0':
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
    }

    virtual void shutdown()
    {
        glDeleteTextures(1, &imageTex);
        glDeleteProgram(matParams.progID);
    }
};

DECLARE_MAIN(lutColorCorrection)
