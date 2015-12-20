#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgPrimitives.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <iostream>

using namespace glpg;
using namespace glpg::math;

class lambertCube : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;
    
    GLuint programID;
    
    struct uniforms_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };
    
    GLuint uniforms_buffer;
    
    struct
    {
        GLint ambient_color;
        GLint diffuse_albedo;
        GLint light_pos;
    } uniforms;
    
    GLuint normalVizProgramID;
    GLint normalVizMVPUniform;

    IMeshRenderer* cubeMesh;

    float aspect;
    math::mat4 projection;
    
    bool showDebugNormals;

    virtual void settings()
    {
        mTitle = "Lambert Cube";
    }

    virtual void setup()
    {
        showDebugNormals = false;
        
        // Set the clear color
        clearColor[0] = 0.5f;
        clearColor[1] = 0.5f;
        clearColor[2] = 0.5f;
        clearColor[3] = 0.0f;
        clearDepth = 1.0f;

        // Create and compile shader
        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/lambert/lambert.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/lambert/lambert.frag", GL_FRAGMENT_SHADER, true);

        if (shaders[0] == 0 || shaders[1] == 0)
        {
            std::cout << "Failed to load shaders" << std::endl;
            exit(-1);
        }

        programID = program::linkShaders(shaders, 2, true, true);
        if(programID == 0)
        {
          std::cout << "Failed to link program" << std::endl;
          exit(-1);
        }

        // Get the uniforms from the shader
        uniforms.ambient_color = glGetUniformLocation(programID, "ambient_color");
        uniforms.diffuse_albedo = glGetUniformLocation(programID, "diffuse_albedo");
        uniforms.light_pos = glGetUniformLocation(programID, "light_pos");
        
        // Create and compile the normal viz helper shader
        GLuint normalShaders[3];
        normalShaders[0] = shader::fromFile("../shaders/normalViz/normal.vert", GL_VERTEX_SHADER, true);
        normalShaders[1] = shader::fromFile("../shaders/normalViz/normal.geom", GL_GEOMETRY_SHADER, true);
        normalShaders[2] = shader::fromFile("../shaders/normalViz/normal.frag", GL_FRAGMENT_SHADER, true);
        
        if (normalShaders[0] == 0 || normalShaders[1] == 0 || normalShaders[2] == 0)
        {
            std::cout << "Failed to load normal viz shaders" << std::endl;
            exit(-1);
        }
        
        normalVizProgramID = program::linkShaders(normalShaders, 3, true, true);
        if(normalVizProgramID == 0)
        {
            std::cout << "Failed to link normal viz program" << std::endl;
            exit(-1);
        }
        
        normalVizMVPUniform = glGetUniformLocation(normalVizProgramID, "MVP");
        
        // Generate the uniform buffer
        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
        // Defined dynamic because we animate
        glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

        cubeMesh = new BaseMeshRenderer(UnitCube::VertexCount(), 0, UnitCube::Vertices(), UnitCube::UVs(), UnitCube::Normals());
        

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

        glUseProgram(programID);

        // set the uniforms buffer
        
        math::mat4 model = math::translate(0.0f, -0.25f, -2.0f)
                            * math::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
                            * math::scale(0.25f);
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
        
        glUniform3fv(uniforms.ambient_color, 1, math::vec3(0.0));
        glUniform3fv(uniforms.diffuse_albedo, 1, math::vec3(1.0, 0, 0));
        glUniform4fv(uniforms.light_pos, 1, math::vec4(0.25, -0.25, -0.25, 0.0));
        
        cubeMesh->render();
        
        if(showDebugNormals)
        {
            glUseProgram(normalVizProgramID);
            glUniformMatrix4fv(normalVizMVPUniform, 1, GL_FALSE, MVP);
            cubeMesh->render();
        }
        
        //GLint error = glGetError();

    }
    
    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case 'N':
                    showDebugNormals = !showDebugNormals;
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = math::perspective(50.0f, aspect, 0.1f, 1000.0f);
    }

    virtual void shutdown()
    {
        delete cubeMesh;
        
        glDeleteProgram(programID);
        glDeleteBuffers(1, &uniforms_buffer);
    }
};

DECLARE_MAIN(lambertCube)
