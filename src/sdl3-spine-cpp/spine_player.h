#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/*Base-type spine player regardless of rendering library.*/

#include <string>
#include <vector>
#include <memory>

#include "sdl_spine.h"
#include "sdl_spine_loader.h"
using FPoint2 = SDL_FPoint;
using CSpineDrawable = CSdlSpineDrawable;
using CTextureLoader = CSdlTextureLoader;
namespace spine_loader = sdl_spine_loader;


class CSpinePlayer
{
public:
	CSpinePlayer();
	virtual ~CSpinePlayer();

	bool loadSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skeletonFilePaths);
	bool loadSpineFromMemory(const std::vector<std::string>& atlasFileData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& SkeletonFileData);

	size_t getNumberOfSpines() const;
	bool hasSpineBeenLoaded() const;

	void update(float fDelta);

	void resetScale();

	void addOffset(int iX, int iY);

	void shiftAnimation();
	void shiftSkin();

	void setAnimationByIndex(size_t nIndex);
	void setAnimationByName(const char* animationName);
	void restartAnimation();

	void setSkinByIndex(size_t nIndex);
	void setSkinByName(const char* skinName);
	void setupSkin();

	void togglePma();
	void toggleBlendMode();

	bool premultiplyAlpha(bool toBePremultiplied, size_t nDrawableIndex = 0);
	bool isAlphaPremultiplied(size_t nDrawableIndex = 0) const;

	bool forceBlendModeNormal(bool toForce, size_t nDrawableIndex = 0);
	bool isBlendModeNormalForced(size_t nDrawableIndex = 0) const;

	void setDrawOrder(bool toBeReversed);
	bool isDrawOrderReversed() const;

	const char* getCurrentAnimationName();
	/// @brief Get animation time actually entried in track.
	/// @param fTrack elapsed time since the track was entried.
	/// @param fLast current timeline position.
	/// @param fStart timeline start position.
	/// @param fEnd timeline end position.
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);
	float getAnimationDuration(const char* animationName);

	const std::vector<std::string>& getSlotNames() const;
	const std::vector<std::string>& getSkinNames() const;
	const std::vector<std::string>& GetAnimationNames() const;

	void setSlotsToExclude(const std::vector<std::string>& slotNames);
	void mixSkins(const std::vector<std::string>& skinNames);
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false);
	void mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime);
	void clearMixedAnimation();

	void setSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	FPoint2 getBaseSize() const;
	void setBaseSize(float fWidth, float fHeight);
	void resetBaseSize();

	FPoint2 getOffset() const;
	void setOffset(float fWidth, float fHeight);

	float getSkeletonScale() const;
	void setSkeletonScale(float fScale);

	float getCanvasScale() const;
	void setCanvasScale(float fScale);

	float getTimeScale() const;
	void setTimeScale(float fTimeScale);
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::shared_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::unique_ptr<CSpineDrawable>> m_drawables;

	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	bool m_isDrawOrderReversed = false;

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	std::vector<std::string> m_slotNames;

	void clearDrawables();
	bool setupDrawer();

	void workOutDefaultSize();
	virtual void workOutDefaultScale() = 0;
	virtual void workOutDefaultOffset() = 0;

	void updatePosition();

	void clearAnimationTracks();
};

#endif // !SPINE_PLAYER_H_
