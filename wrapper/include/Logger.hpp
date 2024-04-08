/**************************************************************************************************
 * @file Logger.hpp
 * @brief log util.
 * 
 * @details
 * 
 *  HISTORY
 *  -----------------------------------------------------------------------------------------------
 *  Version   Date        Author    Description
 *  -----------------------------------------------------------------------------------------------
 *  0.1       2023-04-19  TDZ       Initial.
 * 
 * 
**************************************************************************************************/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
namespace BaseLib{

#define LOGGER_E(format, ...) printf("[error] @file:%s @line:%d @function:%s ", __BASE_FILE__, __LINE__, __FUNCTION__);printf(format, ##__VA_ARGS__);printf("\n");
#define LOGGER_I(format, ...) printf("[ info] ");printf(format, ##__VA_ARGS__);printf("\n");
#define LOGGER_D(format, ...) printf("[debug] @file:%s @line:%d @function:%s ", __BASE_FILE__, __LINE__, __FUNCTION__);printf(format, ##__VA_ARGS__);printf("\n");

#define LOGGER_GLE(format, ...) {GLint error;for (error = glGetError(); error; error = glGetError()) {printf("[GL error] glError(0x%x):", error); printf(format, ##__VA_ARGS__);printf("\n");}}

};
#endif /* LOGGER_HPP */
