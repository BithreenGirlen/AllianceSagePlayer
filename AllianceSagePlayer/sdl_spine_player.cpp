

#include "sdl_spine_player.h"
#include "spine_loader.h"

CSdlSpinePlayer::CSdlSpinePlayer(SDL_Window* pSdlWindow, SDL_Renderer* pSdlRenderer)
	:m_pSdlWindow(pSdlWindow), m_pSdlRenderer(pSdlRenderer)
{
	m_textureLoader.SetRenderer(pSdlRenderer);
}

CSdlSpinePlayer::~CSdlSpinePlayer()
{

}
/*çƒï`âÊ*/
void CSdlSpinePlayer::Redraw(float fDelta)
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->Update(fDelta);
		m_drawables.at(i).get()->Draw(m_pSdlRenderer);
	}
}
/*èâä˙é⁄ìxéZèo*/
void CSdlSpinePlayer::WorkOutDefaultScale()
{
	if (m_skeletonData.empty())return;

	m_fDefaultWindowScale = 1.f;
	m_fDefaultOffset = Fpoint2{};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

	SDL_DisplayMode sSdlDisPlayMode{};
	::SDL_GetCurrentDisplayMode(0, &sSdlDisPlayMode);
	int iDesktopWidth = sSdlDisPlayMode.w;
	int iDesktopHeight = sSdlDisPlayMode.h;

	if (iSkeletonWidth > iDesktopWidth || iSkeletonHeight > iDesktopHeight)
	{
		constexpr float kfMinScale = 0.425f;
		float fScaleX = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
		if (fScaleX < kfMinScale)fScaleX = kfMinScale;
		float fScaleY = static_cast<float>(iDesktopHeight) / iSkeletonHeight;
		if (fScaleY < kfMinScale)fScaleY = kfMinScale;

		if (iDesktopWidth > iDesktopHeight)
		{
			m_fDefaultWindowScale = fScaleY;
		}
		else
		{
			m_fDefaultWindowScale = fScaleX;
		}
		constexpr float kfMinOffsetScale = 0.5f;
		float fOffsetScale = m_fDefaultWindowScale;
		if (fOffsetScale < kfMinOffsetScale)fOffsetScale = kfMinOffsetScale;
		m_fDefaultOffset.x = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) * fOffsetScale : 0.f;
		m_fDefaultOffset.y = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) * fOffsetScale : 0.f;
	}
}

void CSdlSpinePlayer::ResizeWindow()
{
	if (m_pSdlWindow != nullptr)
	{
		::SDL_SetWindowSize(m_pSdlWindow, static_cast<int>(m_fBaseSize.x * m_fSkeletonScale), static_cast<int>(m_fBaseSize.y * m_fSkeletonScale));
	}
}
