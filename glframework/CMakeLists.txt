cmake_minimum_required(VERSION 3.5)

include_directories(
	 ${PROJECT_SOURCE_DIR}/thirdParty/include
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/glm
	 ${PROJECT_SOURCE_DIR}/thirdParty/include/glad
	 #${PROJECT_SOURCE_DIR}/thirdParty/include/zlib

)

message(${PROJECT_SOURCE_DIR}/thirdParty/include/)
include_directories(
	 #${PROJECT_SOURCE_DIR}/thirdParty/include/spdlog/include/
	 ${PROJECT_SOURCE_DIR}/wrapper/include
	 ${PROJECT_SOURCE_DIR}/application/include
	 ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE HEADER_FILES 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

file(GLOB_RECURSE FW ./src/  *.cpp)

add_library(fw ${FW} ${HEADER_FILES})
