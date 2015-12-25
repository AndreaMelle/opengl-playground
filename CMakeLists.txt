cmake_minimum_required (VERSION 2.6)

project (opengl-playground)

LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib )
LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/deps/glfw/lib/osx )
LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/deps/assimp/lib/osx )

if (APPLE)
LINK_DIRECTORIES( /usr/local/lib )
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

find_package(OpenGL)

set (CMAKE_DEBUG_POSTFIX "_d")

if(WIN32)
set(COMMON_LIBS glpg assimp SOIL AntTweakBar optimized glfw3 debug glfw3_d ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES})
elseif (APPLE)
find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(IOKIT_LIBRARY IOKit REQUIRED)
find_library(COREVID_LIBRARY CoreVideo REQUIRED)
find_library(GL_LIBRARY OpenGL REQUIRED)
set(COMMON_LIBS glpg glfw3 assimp SOIL AntTweakBar ${COCOA_LIBRARY} ${GL_LIBRARY} ${IOKIT_LIBRARY} ${COREVID_LIBRARY})
elseif (UNIX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
set(COMMON_LIBS glpg glfw3 assimp SOIL AntTweakBar X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL rt dl)
else()
set(COMMON_LIBS glpg)
endif()

set(COMMON_LIBS ${COMMON_LIBS} ${EXTRA_LIBS})

add_library(SOIL
            deps/soil/src/image_helper.c
            deps/soil/src/stb_image_aug.c
            deps/soil/src/image_DXT.c
            deps/soil/src/SOIL.c
)

add_library(glpg
            src/glpg/glpgApp.cpp
            src/glpg/glpgArcball.cpp
            src/glpg/glpgAssImpLoader.cpp
            src/glpg/glpgBaseMeshLoader.cpp
            src/glpg/glpgBaseMeshRenderer.cpp
            src/glpg/glpgCompositeIndexdedMeshRenderer.cpp
            src/glpg/glpgExt.cpp
            src/glpg/glpgShader.cpp
            src/glpg/glpgTextures.cpp
            include/glpgApp.h
            include/glpgArcball.h
            include/glpgAssImpLoader.h
            include/glpgBaseMeshLoader.h
            include/glpgBaseMeshRenderer.h
            include/glpgCompositeIndexdedMeshRenderer.h
            include/glpgExt.h
            include/glpgIMeshLoader.h
            include/glpgIMeshRenderer.h
            include/glpgMath.h
            include/glpgPrimitives.h
            include/glpgShader.h
            include/glpgTextures.h
            deps/gl3w/src/gl3w.c
)

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(EXAMPLES
  helloApp
  firstTriangle
  lambertCube
  blinnPhongTeapot
  blinnPhongSpheres
  cellTeapot
  deferredDragon
  packingStuff
  textureCube
  envMapSampling
  nprShading
)

#find_package(OpenMP)
#if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#endif()

foreach(EXAMPLE ${EXAMPLES})
  add_executable(${EXAMPLE} WIN32 src/${EXAMPLE}/${EXAMPLE}.cpp)
  set_property(TARGET ${EXAMPLE} PROPERTY DEBUG_POSTFIX _d)
  target_link_libraries(${EXAMPLE} ${COMMON_LIBS})
  if(MSVC)
    configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.vcxproj.user @ONLY)
  endif(MSVC)
endforeach(EXAMPLE)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories( include )
include_directories(deps/glfw/include)
include_directories(deps/gl3w/include)
include_directories(deps/soil/src)
include_directories(deps/assimp/include)
include_directories(deps/glm)

if (APPLE)
include_directories(/usr/local/include)
endif()

ADD_CUSTOM_TARGET(debug ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})
ADD_CUSTOM_TARGET(release ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})
