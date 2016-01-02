#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgPrimitives.h>
#include <glpgArcball.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glpgGeometry.h>
#include <limits>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/random.hpp>
#include <SOIL.h>
#include <sstream>
#include <iomanip>

using namespace glpg;
using namespace glpg::math;

void minmaxVector(const std::vector<glm::vec4>& v, int& maxZindex, int& minZindex)
{
    assert(v.size() > 0);
    
    float maxZ = v[0].z;
    float minZ = v[0].z;
    maxZindex = 0;
    minZindex = 0;
    
    for(int i = 1; i < v.size(); ++i)
    {
        if(v[i - 1].z < v[i].z)
        {
            if(v[i - 1].z < minZ)
            {
                minZ = v[i - 1].z;
                minZindex = i - 1;
            }
            
            if(v[i][2] > maxZ)
            {
                maxZ = v[i][2];
                maxZindex = i;
            }
        }
        else
        {
            if(v[i].z < minZ)
            {
                minZ = v[i].z;
                minZindex = i;
            }
            
            if(v[i - 1].z > maxZ)
            {
                maxZ = v[i - 1].z;
                maxZindex = i - 1;
            }
        }
    }
}

class volumetricRenderingSlicing : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;
    
    float aspect;
    glm::mat4 view;
    glm::mat4 model;
    glm::mat4 projection;
    
    std::vector<math::vec3> mRefAxisVertices;
    std::vector<math::vec3> mRefAxisColors;
    IMeshRenderer* mRefAxisMesh;
    
    std::vector<glm::vec4> mUnitCube;
    
    struct
    {
        GLuint progID;
        GLint color_uniform;
        math::vec3 color;
        GLint transform_uniform;
    } unlitMat;
    
    struct
    {
        GLuint progID;
        GLint tex_uniform;
        GLint transform_uniform;
    } volumeMat;
    
    typedef struct Submesh
    {
        int size;
        int offset;
        GLenum mode;
    } Submesh;
    
    GLuint vao;
    GLuint vertexBuffer;
    GLuint colorBuffer;
    std::vector<Submesh> slices;
    
    int POSITION_LOC = 0;
    int COLOR_LOC = 3;
    int POSITION_SIZE = 3;
    int COLOR_SIZE = 3;
    float samplingRate = 0.003f;
    int maxVertexCount;
    float z_depth = 1.0f;//0.42578125f;
    
    GLuint tex;
    
    //Arcball* arcball;

    virtual void settings()
    {
        mTitle = "Volumetric Rendering by Slicing";
    }
    
    glm::mat4 m;
    
    int sliceToRender;
    float cubeRotY;
    
    void update()
    {
        view = glm::lookAt(glm::vec3(1,1.5,3), glm::vec3(0.5,0.5,0), glm::vec3(0,1,0));
        
        //arcball->update(glm::mat4(), m);
        
        model = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0)) * glm::rotate( cubeRotY, glm::vec3(0,1,0)) * glm::translate(glm::mat4(), glm::vec3(-0.5f, -0.5f, 0));
        
        glm::mat4 modelview = view * model;
        glm::mat4 modelviewInv = glm::inverse(modelview);
        
        // Transform the volume bounding box vertices into view coordinates using the modelview matrix
        std::vector<glm::vec4> mUnitCubeModelViewSpace;
        
        for(int i = 0; i < mUnitCube.size(); ++i)
        {
            glm::vec4 p = modelview * mUnitCube[i];
            mUnitCubeModelViewSpace.push_back(p);
        }
        
        // Find the minimum and maximum z coordinates of the transformed vertices.
        int maxZindex, minZindex;
        minmaxVector(mUnitCubeModelViewSpace, maxZindex, minZindex);
        
        // Compute the number of sampling planes used between these two values using equidistant spacing from the view origin. The sampling distance is computed from the voxel size and current sampling rate.
        
        std::vector<float> samplingPlaneModelViewSpace;
        
        float sp = mUnitCubeModelViewSpace[minZindex][2] + samplingRate;
        
        for(; sp <= mUnitCubeModelViewSpace[maxZindex][2]; sp += samplingRate)
        {
            samplingPlaneModelViewSpace.push_back(sp);
        }
        
        // The unit cube as aabb max min
        glm::vec3 aabbMin(0,0,0);
        glm::vec3 aabbMax(1,1,z_depth);
        
        // The plane we test intersections against
        geometry::Plane plane;
        glm::vec4 n(0,0,1.0f,0); // direction in modelview space
        plane.normal = glm::vec3(modelviewInv * n);
        
        // Estimate max number of vertices we but in vertex buffer
        // A simple worst case scenario is 6 intersections + center of fan along the diagonal
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        
        std::vector<glm::vec3> colors;
        
        int mVertexCount = 0;
        slices.clear();
        std::vector<glm::vec3> vertices;
        
        //For each plane in front-to-back:
        for(int i = samplingPlaneModelViewSpace.size() - 1; i >= 0; --i)
        {
            glm::vec4 p(0,0,samplingPlaneModelViewSpace[i], 1.0f); //plane point in modelview space
            plane.point = glm::vec3(modelviewInv * p);
            std::vector<glm::vec3> intersections;
            
            // We are using front to back, so we can discard when vertex buffer full. If back to front is needed, we just draw in reverse
            // 1. Test for intersections with the edges of the bounding box.
            /* This is better if axis aligned box is used -> use model space
             * Unity cube is 6 planes with equations
             * x = 0; y = 0; z = 0; x = 1; y = 1; z = 1;
             */
            
            // 2. Add each intersection point to a temporary vertex list.
            // Up to six intersections are generated, so the maximum size of the list is fixed.
            
            geometry::ComputePlaneAABBIntersection(aabbMin, aabbMax, plane, intersections);
            
            //4. Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane and computing their angle around the center, with the first vertex or the x axis as the reference.
            geometry::SortPointsOnPlane(intersections, plane);
            
            //5. Tessellate the proxy polygon into triangles and add the resulting vertices to the output vertex array.
            //The slice polygon can be tessellated into a triangle strip or a triangle fan using the center.
            
            if(intersections.size() > 6 || intersections.size() < 3) continue; // not enougth to form a triangle
            
            int offset;
            int size;
            GLenum mode;
            
            // Intersection points are in model space, and it's fine
            // we need model space to sample the 3D texture.
            // the program will transform to modeviewprojection space for rendering
            
            
            
            
            glm::vec3 color((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
            
            if (intersections.size() == 3)
            {
                // this is just a triangle
                mode = GL_TRIANGLES;
                
                offset = mVertexCount;
                size = intersections.size();
                
                if (offset + size >= maxVertexCount)
                {
                    // Not enough space in buffer, we are done
                    return;
                }
                
                vertices.push_back(intersections[0]);
                vertices.push_back(intersections[1]);
                vertices.push_back(intersections[2]);
                
                colors.push_back(color);
                colors.push_back(color);
                colors.push_back(color);
            }
            else
            {
                // we use a triangle fan
                mode = GL_TRIANGLE_FAN;
                
                offset = mVertexCount;
                size = intersections.size() + 2;
                
                if (offset + size >= maxVertexCount)
                {
                    // Not enough space in buffer, we are done
                    return;
                }
                
                //3. Compute the center of the proxy polygon by averaging the intersection points.
                glm::vec3 center = geometry::ComputeCenter(intersections);
                
                vertices.push_back(center);
                colors.push_back(color);
                
                for(int i = 0; i < intersections.size(); ++i)
                {
                    vertices.push_back(intersections[i]);
                    colors.push_back(color);
                }
                
                vertices.push_back(intersections[0]);
                colors.push_back(color);
                
            }
            
            //glBufferSubData(GL_ARRAY_BUFFER, offset * POSITION_SIZE * sizeof(float), size * POSITION_SIZE * sizeof(float), &vertices[0]);
            
            
            
            Submesh slice;
            slice.mode = mode;
            slice.offset = offset;
            slice.size = size;
            slices.push_back(slice);
            
            mVertexCount += size;
            
        }
        
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * POSITION_SIZE * sizeof(float), &vertices[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
    }

    virtual void setup()
    {
        clearColor[0] = 0.5f; clearColor[1] = 0.5f; clearColor[2] = 0.5f; clearColor[3] = 0.0f;
        clearDepth = 1.0f;
        
        view = glm::lookAt(glm::vec3(1.8,1.5,2), glm::vec3(0,0,0), glm::vec3(0,1,0));
        model = glm::mat4();
        
        z_depth = 1.0f / 1.5f;
        
        mUnitCube.push_back(glm::vec4(0,0,0,1));
        mUnitCube.push_back(glm::vec4(1,0,0,1));
        mUnitCube.push_back(glm::vec4(1,1,0,1));
        mUnitCube.push_back(glm::vec4(0,1,0,1));
        mUnitCube.push_back(glm::vec4(0,0,z_depth,1));
        mUnitCube.push_back(glm::vec4(1,0,z_depth,1));
        mUnitCube.push_back(glm::vec4(1,1,z_depth,1));
        mUnitCube.push_back(glm::vec4(0,1,z_depth,1));
        
        maxVertexCount = (int)ceilf(2.0f / samplingRate) * 8;
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, maxVertexCount * POSITION_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(POSITION_LOC);
        glVertexAttribPointer(POSITION_LOC, POSITION_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
//        glGenBuffers(1, &colorBuffer);
//        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
//        glBufferData(GL_ARRAY_BUFFER, colors.size() * COLOR_SIZE * sizeof(float), &colors[0], GL_DYNAMIC_DRAW);
//        glEnableVertexAttribArray(COLOR_LOC);
//        glVertexAttribPointer(COLOR_LOC, COLOR_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        sliceToRender = 0;
        cubeRotY = 0;
        
        math::vec3 green(0.0, 1.0, 0.0);
        math::vec3 red(1.0, 0.0, 0.0);
        math::vec3 blue(0.0, 0.0, 1.0);
        math::vec3 white(1.0, 1.0, 1.0);
        
        mRefAxisVertices.push_back(math::vec3(0,0,0)); mRefAxisVertices.push_back(math::vec3(1,0,0));
        mRefAxisColors.push_back(green); mRefAxisColors.push_back(green);
        
        mRefAxisVertices.push_back(math::vec3(0,0,0)); mRefAxisVertices.push_back(math::vec3(0,1,0));
        mRefAxisColors.push_back(red); mRefAxisColors.push_back(red);
        
        mRefAxisVertices.push_back(math::vec3(0,0,0)); mRefAxisVertices.push_back(math::vec3(0,0,z_depth));
        mRefAxisColors.push_back(blue); mRefAxisColors.push_back(blue);
        
        mRefAxisVertices.push_back(math::vec3(1,1,z_depth)); mRefAxisVertices.push_back(math::vec3(1,1,0));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(1,1,z_depth)); mRefAxisVertices.push_back(math::vec3(0,1,z_depth));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(1,1,z_depth)); mRefAxisVertices.push_back(math::vec3(1,0,z_depth));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(0,0,z_depth)); mRefAxisVertices.push_back(math::vec3(0,1,z_depth));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(1,0,0)); mRefAxisVertices.push_back(math::vec3(1,1,0));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(1,0,z_depth)); mRefAxisVertices.push_back(math::vec3(1,1,z_depth));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(0,0,z_depth)); mRefAxisVertices.push_back(math::vec3(1,0,z_depth));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(0,1,z_depth)); mRefAxisVertices.push_back(math::vec3(0,1,0));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(0,1,0)); mRefAxisVertices.push_back(math::vec3(1,1,0));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisVertices.push_back(math::vec3(1,0,z_depth)); mRefAxisVertices.push_back(math::vec3(1,0,0));
        mRefAxisColors.push_back(white); mRefAxisColors.push_back(white);
        
        mRefAxisMesh = new BaseMeshRenderer(mRefAxisVertices.size(),
                                            0,
                                            mRefAxisVertices,
                                            std::vector<math::vec2>(),
                                            std::vector<math::vec3>(),
                                            GL_LINES,
                                            mRefAxisColors);
        
        assert(mRefAxisMesh != NULL);
        
        // create 3D texture from slice images
        
        //000x
        const char* basepath = "../textures/volumes/FemaleHead/MRI-Woman-";
        const char* ext = ".png";
        int depth = 109;
        
        int width, height, channels;
        int temp_width, temp_height, temp_channels;
        unsigned char* temp_data;
        unsigned char* tex_buffer = 0;
        
        for (int i = 0; i < depth; ++i)
        {
            std::stringstream ss;
            ss << basepath << std::setfill('0') << std::setw(4) << (i + 1) << ext;
            std::string filename = ss.str();
            
            temp_data = SOIL_load_image(filename.c_str(), &temp_width, &temp_height, &temp_channels, SOIL_LOAD_AUTO);
            
            if (tex_buffer == 0)
            {
                // first assertion and allocation
                assert(temp_data && temp_width == temp_height && temp_channels == 1);
                
                width = temp_width;
                height = temp_height;
                channels = temp_channels;
                
                tex_buffer = (unsigned char*)malloc(width * height * depth * sizeof(unsigned char));
                
            }
            else
            {
                // Assert against previous dimensions
                assert(temp_data && temp_width == width && temp_height == height && temp_channels == channels);
            }
            
            // copy data in buffer
            memcpy(tex_buffer + (width * height * i), temp_data, width * height * sizeof(unsigned char));
            
            SOIL_free_image_data(temp_data);
        }
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &tex);
        
        glBindTexture(GL_TEXTURE_3D, tex);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_UNSIGNED_BYTE, tex_buffer);
        
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        glBindTexture(GL_TEXTURE_3D, 0);
        
        free(tex_buffer);

        // Unlit shader
        GLuint unlitShaders[2];
        unlitShaders[0] = shader::fromFile("../shaders/unlit/pass.vert", GL_VERTEX_SHADER, true);
        unlitShaders[1] = shader::fromFile("../shaders/unlit/unlit_color.frag", GL_FRAGMENT_SHADER, true);
        assert(unlitShaders[0] != 0 && unlitShaders[1] != 0);
        unlitMat.progID = program::linkShaders(unlitShaders, 2, true, true);
        assert(unlitMat.progID != 0);
        unlitMat.color_uniform = glGetUniformLocation(unlitMat.progID, "color");
        unlitMat.transform_uniform = glGetUniformLocation(unlitMat.progID, "MVP");
        unlitMat.color = math::vec3(1.0f, 1.0f, 1.0f);
        glUseProgram(unlitMat.progID);
        glUniform3fv(unlitMat.color_uniform, 1, unlitMat.color);
        glUseProgram(0);
        
        unlitShaders[0] = shader::fromFile("../shaders/unlit/vol.vert", GL_VERTEX_SHADER, true);
        unlitShaders[1] = shader::fromFile("../shaders/unlit/vol.frag", GL_FRAGMENT_SHADER, true);
        assert(unlitShaders[0] != 0 && unlitShaders[1] != 0);
        volumeMat.progID = program::linkShaders(unlitShaders, 2, true, true);
        assert(volumeMat.progID != 0);
        volumeMat.transform_uniform = glGetUniformLocation(volumeMat.progID, "MVP");
        volumeMat.tex_uniform = glGetUniformLocation(volumeMat.progID, "tex");
        glUseProgram(volumeMat.progID);
        glUniform1i(volumeMat.tex_uniform, 0);
        glUseProgram(0);
        
        
        glDisable(GL_CULL_FACE);
        //glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        //arcball = new Arcball(mWindowWidth, mWindowHeight);

        onResize(mWindowWidth, mWindowHeight);
        
        update();
        //sliceToRender = slices.size() - 1;
    }
    
    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);
        
        glm::mat4 MVP = projection * view * model;
        
        glUseProgram(unlitMat.progID);
        glUniformMatrix4fv(unlitMat.transform_uniform, 1, GL_FALSE, glm::value_ptr(MVP));
        
        mRefAxisMesh->render();
        
        //grid.mesh->render();
        
        glUseProgram(0);
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        glUseProgram(volumeMat.progID);
        
        glUniformMatrix4fv(volumeMat.transform_uniform, 1, GL_FALSE, glm::value_ptr(MVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex);
        
        glBindVertexArray(vao);

        //for (int i = slices.size() - 1; i >= 0; i--)
        //for (int i = 0; i < slices.size(); i++)
        for (int i = slices.size() - 1; i >= sliceToRender; i--)
        {
        //int i = sliceToRender;
            glDrawArrays(slices[i].mode, slices[i].offset, slices[i].size);
        }
        
        glBindVertexArray(0);
        
        glUseProgram(0);
        
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        glDisable(GL_BLEND);
        //glm::mat4 model = m * glm::rotate(glm::mat4(), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        //* glm::scale(glm::mat4(), glm::vec3(0.01f, 0.01f, 0.01f));
        
        //MVP = projection * view * model;
        
    }
    
    virtual void onMouseButton(const int& button, const int& action)
    {
        int x, y;
        getMousePosition(x, y);
        //arcball->onMouseButton(button, action, x, y);
    }
    
    virtual void onMouseMove(const int& x, const int& y)
    {
        //arcball->onMouseMove(x, y);
    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case 'A':
                    sliceToRender = (sliceToRender - 1) % slices.size();
                    break;
                case 'S':
                    sliceToRender = (sliceToRender + 1) % slices.size();
                    break;
                case 'Z':
                    cubeRotY -= 0.1f;
                    update();
                    break;
                case 'X':
                    cubeRotY += 0.1f;
                    update();
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
        //projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 1000.0f);
        
        //arcball->onResize(width, height);
    }

    virtual void shutdown()
    {
        //delete arcball;

        glDeleteProgram(unlitMat.progID);
    }
};

DECLARE_MAIN(volumetricRenderingSlicing)
