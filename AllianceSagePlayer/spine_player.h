#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/*Base-type spine player regardless of rendering library.*/

#include <string>
#include <vector>
#include <memory>

/*SDL*/
#include "sdl_spine_cpp.h"
using Fpoint2 = SDL_FPoint;
using CSpineDrawable = CSdlSpineDrawer;
using CTextureLoader = CSdlTextureLoader;

class CSpinePlayer
{
public:
	CSpinePlayer();
	~CSpinePlayer();

	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	void RescaleSkeleton(bool bUpscale);
	void RescaleTime(bool bHasten);
	void ResetScale();

	void MoveViewPoint(int iX, int iY);
	void ShiftAnimation();
	void ShiftSkin();

	virtual void Redraw(float fDelta);

	void SwitchPma();
	void SwitchBlendModeAdoption();

	std::vector<std::string> GetSlotList();
	std::vector<std::string> GetSkinList() const;
	std::vector<std::string> GetAnimationList() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSpineDrawable>> m_drawables;

	Fpoint2 m_fBaseSize = Fpoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultWindowScale = 1.f;
	Fpoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	Fpoint2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	void ClearDrawables();
	bool SetupDrawer();

	void WorkOutDefualtSize();
	virtual void WorkOutDefaultScale() = 0;
	virtual void ResizeWindow() = 0;

	void UpdateScaletonScale();
	void UpdateTimeScale();

	void ClearAnimationTracks();
};

#endif // !SPINE_PLAYER_H_
