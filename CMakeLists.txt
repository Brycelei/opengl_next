#需求的最低cmake程序版本
cmake_minimum_required(VERSION 3.12)
add_definitions (-DDEBUG)

#本工程的名字
project(Hoho3)

#本工程支持的C++版本
set(CMAKE_CXX_STANDARD 17)


file(GLOB ASSETS "./assets" )

file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB SHADER_FILES CONFIGURE_ DEPENDS
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.frag
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.vert
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.geom)

include_directories(
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/glframework/include
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include/spdlog/include
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/application/include
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/wrapper/include
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include/glad/include
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include

)


link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib
)

add_subdirectory(wrapper)

add_subdirectory(application)
add_subdirectory(glframework)

set_property(SOURCE ${SHADER_FILES} PROPERTY VS_TOOL_OVERRIDE "shader") 

#本工程所有cpp文件编译链接，生成exe
add_executable(${CMAKE_PROJECT_NAME} ${SHADER_FILES} "main.cpp" )


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # 使用微软编译器
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/glfw3.lib)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/assimp-vc142-mtd.lib)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/opencv_world412d.lib) 
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/yaml-cppd.lib)

else()
    # 使用 Mingw 编译器
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/WindowsSystem/glfw-3.3.8.bin.WIN64/lib-mingw-w64/libglfw3.a)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}  wrapper app fw)

