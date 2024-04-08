cmake_minimum_required(VERSION 3.5)
#project(module)

include_directories(
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/spdlog/include
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/glad/include

	 ${CMAKE_CURRENT_SOURCE_DIR}/include
)
#message(${PROJECT_SOURCE_DIR}/thirdParty/include/spdlog/include)
file(GLOB_RECURSE HEADER_FILES 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

#递归将本文件夹下所有cpp放到FUNCS中
file(GLOB_RECURSE WRAPPER ./src/  *.cpp *.c)

#将FUNCS中所有cpp编译为funcs这个lib库
add_library(wrapper ${WRAPPER} ${HEADER_FILES} )
