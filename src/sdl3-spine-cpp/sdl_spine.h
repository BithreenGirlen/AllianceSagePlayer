#ifndef SDL_SPINE_H_
#define SDL_SPINE_H_

#include <spine/spine.h>
#include <SDL3/SDL.h>

class CSdlSpineDrawable
{
public:
	CSdlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CSdlSpineDrawable();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;

	void PremultiplyAlpha(bool toPremultiply);
	bool IsAlphaPremultiplied() const;

	void ForceBlendModeNormal(bool toForce);
	bool IsBlendModeNormalForced() const;

	void Update(float fDelta);
	void Draw(float fOffsetX = 0.f, float fOffsetY = 0.f);

	void SetLeaveOutList(spine::Vector<spine::String>& list);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	SDL_FRect GetBoundingBox() const;
	SDL_FRect GetBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
private:
	bool m_hasOwnAnimationStateData = false;
	bool m_isAlphaPremultiplied = true;
	bool m_toForceBlendModeNormal = false;

	spine::SkeletonClipping m_clipper;

	spine::Vector<SDL_Vertex> m_sdlVertices;
	spine::Vector<int> m_sdlIndices;
	spine::Vector<float> m_worldVertices;
	spine::Vector<unsigned short> m_quadIndices;

	spine::Vector<spine::String> m_leaveOutList;

	bool IsSlotToBeLeftOut(const spine::String& slotName);
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSdlTextureLoader : public spine::TextureLoader
{
public:
	CSdlTextureLoader() {};
	virtual ~CSdlTextureLoader() {};

	void SetRenderer(SDL_Renderer* pSdlRenderer);

	void load(spine::AtlasPage& atlasPage, const spine::String& path) override;
	void unload(void* texture) override;
private:
	SDL_Renderer* m_pSdlRenderer = nullptr;
#ifdef _DEBUG
	spine::String m_SdlErrorMassage;
#endif
};

#endif //!SDL_SPINE_H_
