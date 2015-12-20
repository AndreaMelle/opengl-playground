#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgPrimitives.h>
#include <iostream>

using namespace glpg;
using namespace glpg::math;

class blinnPhongSpheres : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;
    
    GLuint programBlinnPhongID;
    GLuint programPhongID;
    
    struct uniforms_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };
    
    GLuint uniforms_buffer;
    
    struct
    {
        GLint light_pos; //vec4
        GLint light_intensity; //vec3
        GLint attenuation; //vec3 => Constant, Linear, Exp
    } lightParams;
    
    struct
    {
        GLint ambient_color; //vec3
        GLint diffuse_color; //vec3
        GLint specular_color; //vec3
        GLint specular_power; //float
    } matParams;

    IMeshRenderer* sphereMesh;

    float aspect;
    math::mat4 projection;
    
    bool toggleBlinnPhong;

    virtual void settings()
    {
        mTitle = "BlinnPhong Teapot";
        mWindowWidth = 800;
        mWindowHeight = 800;
        mSamples = 2;
    }

    virtual void setup()
    {
        // Set the clear color
        clearColor[0] = 0.2f;
        clearColor[1] = 0.2f;
        clearColor[2] = 0.2f;
        clearColor[3] = 0.0f;
        clearDepth = 1.0f;

        // Create and compile shader
        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/phong/phong.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/phong/phong.frag", GL_FRAGMENT_SHADER, true);

        if (shaders[0] == 0 || shaders[1] == 0)
        {
            std::cout << "Failed to load shaders" << std::endl;
            exit(-1);
        }

        programPhongID = program::linkShaders(shaders, 2, true, true);
        if(programPhongID == 0)
        {
          std::cout << "Failed to link program" << std::endl;
          exit(-1);
        }

        // Get the uniforms from the shader
        lightParams.light_pos = glGetUniformLocation(programPhongID, "light_pos");
        lightParams.light_intensity = glGetUniformLocation(programPhongID, "lightParams.light_intensity");
        lightParams.attenuation = glGetUniformLocation(programPhongID, "lightParams.attenuation");
        
        matParams.ambient_color = glGetUniformLocation(programPhongID, "matParams.ambient_color");
        matParams.diffuse_color = glGetUniformLocation(programPhongID, "matParams.diffuse_color");
        matParams.specular_color = glGetUniformLocation(programPhongID, "matParams.specular_color");
        matParams.specular_power = glGetUniformLocation(programPhongID, "matParams.specular_power");
        
        glUseProgram(programBlinnPhongID);
        
        shaders[0] = shader::fromFile("../shaders/blinnphong/blinnphong.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/blinnphong/blinnphong.frag", GL_FRAGMENT_SHADER, true);
        
        if (shaders[0] == 0 || shaders[1] == 0)
        {
            std::cout << "Failed to load shaders" << std::endl;
            exit(-1);
        }
        
        programBlinnPhongID = program::linkShaders(shaders, 2, true, true);
        if(programBlinnPhongID == 0)
        {
            std::cout << "Failed to link program" << std::endl;
            exit(-1);
        }
        
        // Get the uniforms from the shader
        lightParams.light_pos = glGetUniformLocation(programBlinnPhongID, "light_pos");
        lightParams.light_intensity = glGetUniformLocation(programBlinnPhongID, "lightParams.light_intensity");
        lightParams.attenuation = glGetUniformLocation(programBlinnPhongID, "lightParams.attenuation");
        
        matParams.ambient_color = glGetUniformLocation(programBlinnPhongID, "matParams.ambient_color");
        matParams.diffuse_color = glGetUniformLocation(programBlinnPhongID, "matParams.diffuse_color");
        matParams.specular_color = glGetUniformLocation(programBlinnPhongID, "matParams.specular_color");
        matParams.specular_power = glGetUniformLocation(programBlinnPhongID, "matParams.specular_power");
        
        math::vec3 diffuseColor(1.0f, 0.0f, 1.0f);
        math::vec3 ambientColor(0.1f);
        math::vec3 specularColor(1.0f);
        float specularPower = 128.0f;
        
        math::vec4 lightPosition(-1.0, -1.0, -1.0f, 0.0);
        math::vec3 lightIntensity(1.0f);
        
        glUseProgram(programPhongID);
        
        glUniform3fv(matParams.ambient_color, 1, ambientColor);
        glUniform3fv(matParams.diffuse_color, 1, diffuseColor);
        glUniform3fv(matParams.specular_color, 1, specularColor);
        glUniform1f(matParams.specular_power, specularPower);
        
        glUniform4fv(lightParams.light_pos, 1, lightPosition);
        glUniform3fv(lightParams.light_intensity, 1, lightIntensity);
        
        glUseProgram(0);
        
        glUseProgram(programBlinnPhongID);
        
        glUniform3fv(matParams.ambient_color, 1, ambientColor);
        glUniform3fv(matParams.diffuse_color, 1, diffuseColor);
        glUniform3fv(matParams.specular_color, 1, specularColor);
        glUniform1f(matParams.specular_power, specularPower);
        
        glUniform4fv(lightParams.light_pos, 1, lightPosition);
        glUniform3fv(lightParams.light_intensity, 1, lightIntensity);
        
        glUseProgram(0);
        
        // Generate the uniform buffer
        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
        // Defined dynamic because we animate
        glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);
        
        sphereMesh = new BaseMeshRenderer(UnitSphere::VertexCount(),
                                          0,
                                          UnitSphere::Vertices(),
                                          UnitSphere::UVs(),
                                          UnitSphere::Normals(),
                                          UnitSphere::DrawMode());
        
        if(sphereMesh == NULL)
        {
            std::cout << "Failed to load model." << std::endl;
            exit(-1);
        }
        

        //cull front faces
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        //standard depth test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);

        glUseProgram(toggleBlinnPhong ?  programBlinnPhongID : programPhongID);
        
        int numSpheres = 7;
        
        float gap = 2.5f;
        float min = -gap * floorf((float)numSpheres / 2.0f);
        
        // rows: from 0 to 1 specular color
        // cols: from 4.0 to 256.0f specular power
        
        for(int i = 0; i < numSpheres; ++i)
        {
            for(int j = 0; j < numSpheres; ++j)
            {
                float x = min + (float)i * gap;
                float y = min + (float)j * gap;
                
                
                glUniform3fv(matParams.specular_color, 1, math::vec3((float)i / 9.0f + 1.0f / 9.0f));
                glUniform1f(matParams.specular_power, powf(2.0f, (float)j + 2.0f));
                
                math::mat4 model = math::translate(x, y, -20.0f) * math::scale(1.0f);
                math::mat4 MVP = projection * model;
                math::mat4 normalMatrix = model.inverse().transpose();
                
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
                uniforms_block* block = (uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                          0,
                                                                          sizeof(uniforms_block),
                                                                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                
                block->modelview = model;
                block->MVP = MVP;
                block->normalmatrix = normalMatrix;
                
                glUnmapBuffer(GL_UNIFORM_BUFFER);
                
                sphereMesh->render();
            }
        }
        
        glUseProgram(0);
    }
    
    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case 'N':
                    toggleBlinnPhong = !toggleBlinnPhong;
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = math::perspective(60.0f, aspect, 0.1f, 1000.0f);
    }

    virtual void shutdown()
    {
        delete sphereMesh;
        
        glDeleteProgram(programBlinnPhongID);
        glDeleteProgram(programPhongID);
        glDeleteBuffers(1, &uniforms_buffer);
    }
};

DECLARE_MAIN(blinnPhongSpheres)
