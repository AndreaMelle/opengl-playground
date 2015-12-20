#include <glpgApp.h>
#include <glpgShader.h>

using namespace glpg;

class firstTriangle : public App
{
    GLfloat clearColor[4];

    GLuint programID;

    GLuint vertexBuffer;
    GLuint vertexArrayID;

    virtual void settings()
    {
        mTitle = "First Triangle";
    }

    virtual void setup()
    {
        // Set the clear color
        clearColor[0] = 0.0f;
        clearColor[1] = 0.0f;
        clearColor[2] = 0.4f;
        clearColor[3] = 0.0f;

        //Temporary object to hold vertex data in CPU memory
        // Will be loaded from file or procedurally generated in the future
        static const GLfloat gVertexBufferData[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

        // Create and compile shader
        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/simple.vertexshader", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/simple.fragmentshader", GL_FRAGMENT_SHADER, true);

        if (shaders[0] == 0 || shaders[1] == 0)
        {
            // error
            // exit
        }

        programID = program::linkShaders(shaders, 2, true, true);
        if(programID == 0)
        {
            // error
            // exit
        }

        // Create a Vertex Array Object
        glGenVertexArrays(1, &vertexArrayID);

        // Bind it as current one
        glBindVertexArray(vertexArrayID);

        // Create a buffer for the vertex array, for static draw
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gVertexBufferData), gVertexBufferData, GL_STATIC_DRAW);

    }

    virtual void render(double currentTime)
    {

        glClearBufferfv(GL_COLOR, 0, clearColor);

        // Bind shader
        glUseProgram(programID);

        //First attribute buffer is vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, // attribute 0. Must match the layout in the shader
                              3, // size
                              GL_FLOAT, //type
                              GL_FALSE, //not normalized
                              0, //stride
                              (void*)0); //offset

        // draw 3 indices strarting at 0
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //int glErr = glGetError();

        glDisableVertexAttribArray(0);
    }

    virtual void shutdown()
    {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteProgram(programID);
    }
};

DECLARE_MAIN(firstTriangle)
