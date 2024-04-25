#需求的最低cmake程序版本
cmake_minimum_required(VERSION 3.12)
add_definitions (-DDEBUG)

#本工程的名字
project(NESI_NEXT_9)

#本工程支持的C++版本
set(CMAKE_CXX_STANDARD 17)


file(GLOB ASSETS "./asset" )
message(${ASSETS})
file(COPY ${ASSETS} DESTINATION ${PROJECT_SOURCE_DIR}/bin)

file(GLOB SHADER_FILES CONFIGURE_ DEPENDS
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.frag
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.vert
   ${PROJECT_SOURCE_DIR}/shaders/pbr/hoho/*.geom)

file(COPY ${PROJECT_SOURCE_DIR}/shaders DESTINATION ${PROJECT_SOURCE_DIR}/bin)

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
add_subdirectory(imgui)

set_property(SOURCE ${SHADER_FILES} PROPERTY VS_TOOL_OVERRIDE "shader") 

#本工程所有cpp文件编译链接，生成exe
add_executable(${CMAKE_PROJECT_NAME} ${SHADER_FILES} main.cpp)


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

target_link_libraries(${CMAKE_PROJECT_NAME}  wrapper app fw imgui)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build)

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib")

install(TARGETS ${CMAKE_PROJECT_NAME} wrapper app fw imgui#等待安装的目标文件 可执行文件 静态库 动态库
    RUNTIME  DESTINATION ${PROJECT_SOURCE_DIR}/build/bin                     # 安装 可执行文件到指定目录下的bin目录
    LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/build/bin                     # 安装 动态库文件到指定目录下的lib目录
    ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/lib) # 安装静态库文件到指定目录下的static_lib目录
   