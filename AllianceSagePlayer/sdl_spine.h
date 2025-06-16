#ifndef SDL_SPINE_H_
#define SDL_SPINE_H_

#include <spine/spine.h>
#include <SDL2/SDL.h>

class CSdlSpineDrawer
{
public:
	CSdlSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CSdlSpineDrawer();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;

	bool isAlphaPremultiplied = true;
	bool isToForceBlendModeNormal = false;

	void Update(float fDelta);
	void Draw(SDL_Renderer* pSdlRenderer, float fOffsetX = 0.f, float fOffsetY = 0.f);

	void SetLeaveOutList(spine::Vector<spine::String>& list);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	SDL_FRect GetBoundingBox() const;
private:
	bool m_hasOwnAnimationStateData = false;

	spine::SkeletonClipping m_clipper;

	spine::Vector<SDL_Vertex> m_sdlVertices;
	spine::Vector<int> m_sdlIndices;
	spine::Vector<float> m_worldVertices;
	spine::Vector<unsigned short> m_quadIndices;

	spine::Vector<spine::String> m_leaveOutList;

	bool IsToBeLeftOut(const spine::String& slotName);
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSdlTextureLoader : public spine::TextureLoader
{
public:
	CSdlTextureLoader() {};
	virtual ~CSdlTextureLoader() {};

	void SetRenderer(SDL_Renderer* pSdlRenderer) { m_pSdlRenderer = pSdlRenderer; };

	virtual void load(spine::AtlasPage& atlasPage, const spine::String& path);
	virtual void unload(void* texture);
private:
	SDL_Renderer* m_pSdlRenderer = nullptr;
	spine::String m_SdlErrorMassage;
};

#endif //!SDL_SPINE_H_
