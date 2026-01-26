#ifndef SDL_SPINE_LOADER_H_
#define SDL_SPINE_LOADER_H_

#include <memory>

#include <spine/Atlas.h>
#include <spine/SkeletonData.h>

namespace sdl_spine_loader
{
	std::shared_ptr<spine::Atlas> ReadAtlasFromFile(const char* filePath, spine::TextureLoader* pTextureLoader);
	std::shared_ptr<spine::Atlas> ReadAtlasFromMemory(const char* atlasFileData, size_t dataSize, const char* textureDirectory, spine::TextureLoader* pTextureLoader);

	std::shared_ptr<spine::SkeletonData> ReadSkeletonFromFile(const char* filePath, spine::Atlas* pAtlas);
	std::shared_ptr<spine::SkeletonData> ReadSkeletonFromMemory(const unsigned char* skeletonFileData, size_t dataSize, spine::Atlas* pAtlas);
}
#endif // !SDL_SPINE_LOADER_H_
