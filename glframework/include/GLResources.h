
#ifndef GL_RESOURCES_HPP
#define GL_RESOURCES_HPP

#include <string>
#include <vector>

class GLResources
{
public:
	static uint32_t CreateTexture2D(const std::string& path, bool gamma);
	static uint32_t CreateTexture2D(const std::vector<unsigned char>& data, int width, int height, int channel, bool gamma);
	static uint32_t LoadPresetImage(const std::string& path, int numExtension);
};

#endif