
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>

#include <spine/SkeletonJson.h>
#include <spine/SkeletonBinary.h>

#include "sdl_spine_loader.h"
#include "spine_file_verifier.h"


namespace sdl_spine_loader
{
	static std::shared_ptr<spine::SkeletonData> ReadJsonSkeletonFromMemory(const char* skeletonFileData, size_t dataSize, spine::Atlas* pAtlas)
	{
		spine::SkeletonJson jsonSkeletonParser(pAtlas);
		jsonSkeletonParser.setScale(1.f);

		spine::SkeletonData* pSkeletonData = jsonSkeletonParser.readSkeletonData(skeletonFileData);
		if (pSkeletonData == nullptr)
		{
			::SDL_Log(jsonSkeletonParser.getError().buffer());

			return nullptr;
		}

		return std::shared_ptr<spine::SkeletonData>(pSkeletonData);
	}

	static std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonFileData, size_t dataSize, spine::Atlas* pAtlas)
	{
		spine::SkeletonBinary binarySkeletonParser(pAtlas);
		binarySkeletonParser.setScale(1.f);

		spine::SkeletonData* pSkeletonData = binarySkeletonParser.readSkeletonData(skeletonFileData, static_cast<int>(dataSize));
		if (pSkeletonData == nullptr)
		{
			::SDL_Log(binarySkeletonParser.getError().buffer());

			return nullptr;
		}

		return std::shared_ptr<spine::SkeletonData>(pSkeletonData);
	}
} /* sdl_spine_loader */

std::shared_ptr<spine::Atlas> sdl_spine_loader::ReadAtlasFromFile(const char* filePath, spine::TextureLoader* pTextureLoader)
{
	size_t fileSize = 0;
	auto pBlob = std::unique_ptr<char, decltype (&::SDL_free)>
		(
			static_cast<char*>(::SDL_LoadFile(filePath, &fileSize)),
			::SDL_free
		);

	const char* dir = filePath;
	for (;;)
	{
		const char* pos = ::SDL_strpbrk(dir, "\\/");
		if (pos == nullptr)break;
		dir = pos + 1;
	}

	const auto textureDirectory = std::unique_ptr<char, decltype (&::SDL_free)>
		(
			[&filePath, dir]
			{
				size_t dirLength = dir - filePath;
				char* p = static_cast<char*>(::SDL_malloc(dirLength + 1));
				::SDL_memcpy(p, filePath, dirLength);
				p[dirLength] = '\0';
				return p;
			}(),
			::SDL_free
		);

	return ReadAtlasFromMemory(pBlob.get(), fileSize, textureDirectory.get(), pTextureLoader);
}

std::shared_ptr<spine::Atlas> sdl_spine_loader::ReadAtlasFromMemory(const char* atlasFileData, size_t dataSize, const char* textureDirectory, spine::TextureLoader* pTextureLoader)
{
	return std::make_shared<spine::Atlas>(atlasFileData, static_cast<int>(dataSize), textureDirectory, pTextureLoader);
}

std::shared_ptr<spine::SkeletonData> sdl_spine_loader::ReadSkeletonFromFile(const char* filePath, spine::Atlas* pAtlas)
{
	size_t fileSize = 0;
	auto pBlob = std::unique_ptr<unsigned char, decltype (&::SDL_free)>
		(
			static_cast<unsigned char*>(::SDL_LoadFile(filePath, &fileSize)),
			::SDL_free
		);

	return ReadSkeletonFromMemory(pBlob.get(), fileSize, pAtlas);
}

std::shared_ptr<spine::SkeletonData> sdl_spine_loader::ReadSkeletonFromMemory(const unsigned char* skeletonFileData, size_t dataSize, spine::Atlas* pAtlas)
{
	using namespace spine_file_verifier;
	SkeletonMetadata skeletonMetaData = VerifySkeletonFileData(skeletonFileData, dataSize);
	switch (skeletonMetaData.skeletonFormat)
	{
	case SkeletonFormat::Json: return ReadJsonSkeletonFromMemory(reinterpret_cast<const char*>(skeletonFileData), dataSize, pAtlas);
	case SkeletonFormat::Binary: return ReadBinarySkeletonFromMemory(skeletonFileData, dataSize, pAtlas);
	default:break;
	}

	return nullptr;
}
