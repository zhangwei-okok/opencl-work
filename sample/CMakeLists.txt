cmake_minimum_required(VERSION 3.5)

project(sample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCL REQUIRED)

set(CHAPTERS
    started
    optimization
)

set(started
    first
    dp-mul
)

set(optimization
   imgmirror
   smoothing
)


function(create_project_from_sources chapter demo)
        file(GLOB SOURCE
            "${chapter}/${demo}/*.h"
            "${chapter}/${demo}/*.cpp"
            "${chapter}/${demo}/*.cl"
          )
    set(NAME "${demo}")
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME}
        dl
        OpenCL::OpenCL
        cl-framwork
        dlfcn
        assimp m rt dl GLX X11 pthread)
    file(GLOB SHADERS
             "${chapter}/${demo}/*.cl"
             "${chapter}/${demo}/*.bmp"
    )
    set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}")
    foreach(SHADER ${SHADERS})
            file(COPY ${SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/bin/${chapter})
    endforeach(SHADER)
endfunction()

foreach(CHAPTER ${CHAPTERS})
    foreach(DEMO ${${CHAPTER}})
                create_project_from_sources(${CHAPTER} ${DEMO})
    endforeach(DEMO)
endforeach(CHAPTER)

include_directories(${CMAKE_SOURCE_DIR}/includes)
