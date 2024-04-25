cmake_minimum_required(VERSION 3.5)
project(app)
include_directories(
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/glad
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/glm
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/imgui
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/imgui/backends
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/ImGuizmo

	 ${PROJECT_SOURCE_DIR}/glframework/include

	 #${PROJECT_SOURCE_DIR}/wrapper/include
	 ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#递归将本文件夹下所有cpp放到FUNCS中
file(GLOB_RECURSE APP ./src  *.cpp)

file(GLOB_RECURSE HEADER_FILES 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(app ${APP} ${HEADER_FILES} )
target_link_libraries(app ${CMAKE_SOURCE_DIR}/build/glframework/Debug/fw.lib)

#message(${CMAKE_SOURCE_DIR}22)