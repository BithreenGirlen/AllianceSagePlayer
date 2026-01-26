#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/*Base-type spine player regardless of rendering library.*/

#include <string>
#include <vector>
#include <memory>

#include "sdl_spine.h"
using FPoint2 = SDL_FPoint;
using CSpineDrawable = CSdlSpineDrawable;
using CTextureLoader = CSdlTextureLoader;


class CSpinePlayer
{
public:
	CSpinePlayer();
	virtual ~CSpinePlayer();

	bool LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel);
	bool LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& texturePaths, const std::vector<std::string>& skelData, bool isBinarySkel);

	size_t GetNumberOfSpines() const;
	bool HasSpineBeenLoaded() const;

	void Update(float fDelta);

	void ResetScale();

	void MoveViewPoint(int iX, int iY);

	void ShiftAnimation();
	void ShiftSkin();

	void SetAnimationByIndex(size_t nIndex);
	void SetAnimationByName(const char* animationName);
	void RestartAnimation();

	void SetSkinByIndex(size_t nIndex);
	void SetSkinByName(const char* skinName);
	void SetupSkin();

	void TogglePma();
	void ToggleBlendMode();

	bool PremultiplyAlpha(bool toBePremultiplied, size_t nDrawableIndex = 0);
	bool IsAlphaPremultiplied(size_t nDrawableIndex = 0) const;

	bool ForceBlendModeNormal(bool toForce, size_t nDrawableIndex = 0);
	bool IsBlendModeNormalForced(size_t nDrawableIndex = 0) const;

	void SetDrawOrder(bool toBeReversed);
	bool IsDrawOrderReversed() const;

	const char* GetCurrentAnimationName();
	/// @brief Get animation time actually entried in track.
	/// @param fTrack elapsed time since the track was entried.
	/// @param fLast current timeline position.
	/// @param fStart timeline start position.
	/// @param fEnd timeline end position.
	void GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);

	std::vector<std::string> GetSlotNames();
	const std::vector<std::string>& GetSkinNames() const;
	const std::vector<std::string>& GetAnimationNames() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);

	void SetSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	FPoint2 GetBaseSize() const;
	void SetBaseSize(float fWidth, float fHeight);
	void ResetBaseSize();

	FPoint2 GetOffset() const;

	float GetSkeletonScale() const;
	void SetSkeletonScale(float fScale);

	float GetCanvasScale() const;
	void SetCanvasScale(float fScale);

	float GetTimeScale() const;
	void SetTimeScale(float fTimeScale);
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSpineDrawable>> m_drawables;

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

	void ClearDrawables();
	bool SetupDrawer();

	void WorkOutDefaultSize();
	virtual void WorkOutDefaultScale() = 0;
	virtual void WorkOutDefaultOffset() = 0;

	void UpdatePosition();

	void ClearAnimationTracks();
};

#endif // !SPINE_PLAYER_H_
