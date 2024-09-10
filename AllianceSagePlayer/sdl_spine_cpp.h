#ifndef SDL_SPINE_CPP_H_
#define SDL_SPINE_CPP_H_

#include <spine/spine.h>
#include <SDL2/SDL.h>

class CSdlSpineDrawer
{
public:
	CSdlSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CSdlSpineDrawer();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;
	float timeScale = 1.f;

	void Update(float fDelta);
	void Draw(SDL_Renderer* pSdlRenderer = nullptr);

	void SwitchPma() { m_bAlphaPremultiplied ^= true; }
	void SwitchBlendModeAdoption() { m_bForceBlendModeNormal ^= true; }
	void SetLeaveOutList(spine::Vector<spine::String>& list);
private:
	bool m_bHasOwnAnimationStateData = false;
	bool m_bAlphaPremultiplied = true;
	bool m_bForceBlendModeNormal = false;

	spine::Vector<SDL_Vertex> m_sdlVertices;
	spine::Vector<int> m_sdlIndices;

	spine::SkeletonClipping m_clipper;
	spine::Vector<float> m_worldVertices;
	spine::Vector<unsigned short> m_quadIndices;

	spine::Vector<spine::String> m_leaveOutList;

	bool IsToBeLeftOut(const spine::String& slotName);

	SDL_BlendMode m_SdlBlendModeScreen = SDL_BlendMode::SDL_BLENDMODE_NONE;
	SDL_BlendMode m_SdlBlendModeNormalPma = SDL_BlendMode::SDL_BLENDMODE_NONE;
	SDL_BlendMode m_SdlBlendModeAdditivePma = SDL_BlendMode::SDL_BLENDMODE_NONE;
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

#endif //!SDL_SPINE_CPP_H_
