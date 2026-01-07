

#include "sdl_spine_player.h"

CSdlSpinePlayer::CSdlSpinePlayer(SDL_Window* pSdlWindow, SDL_Renderer* pSdlRenderer)
	:m_pSdlWindow(pSdlWindow), m_pSdlRenderer(pSdlRenderer)
{
#ifdef SDL_SPINE_CPP
	m_textureLoader.SetRenderer(pSdlRenderer);
#endif
}

CSdlSpinePlayer::~CSdlSpinePlayer()
{

}
/*再描画*/
void CSdlSpinePlayer::Redraw()
{
	float fX = 0.f;
	float fY = 0.f;
	//if (m_pSdlWindow != nullptr && m_pSdlRenderer != nullptr)
	//{
	//	int iWindowWidth = 0;
	//	int iWindowHeight = 0;
	//	::SDL_GetWindowSize(m_pSdlWindow, &iWindowWidth, &iWindowHeight);

	//	fX = (m_fBaseSize.x - iWindowWidth / m_fSkeletonScale) / 2.f;
	//	fY = (m_fBaseSize.y - iWindowHeight / m_fSkeletonScale) / 2.f;

	//	::SDL_RenderSetScale(m_pSdlRenderer, m_fSkeletonScale, m_fSkeletonScale);
	//}

	/* For usual case, use SDL's scale. */
	for (const auto& drawble : m_drawables)
	{
		drawble->skeleton->setScaleX(m_fSkeletonScale);
		drawble->skeleton->setScaleY(m_fSkeletonScale);
	}

	if (!m_bDrawOrderReversed)
	{
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables[i]->Draw(-fX, -fY);
		}
	}
	else
	{
		for(long long i = m_drawables.size() - 1; i >= 0;--i)
		{
			m_drawables[i]->Draw(-fX, -fY);
		}
	}

	//if (m_pSdlRenderer != nullptr)
	//{
	//	::SDL_RenderSetScale(m_pSdlRenderer, 1.f, 1.f);
	//}
}
/*初期尺度算出*/
void CSdlSpinePlayer::WorkOutDefaultScale()
{
	/*
	* The following calculation is game-specific measure to adapt to its too long height. 
	* When used as runtime for other games, use commented-out code instead of the calculation below.
	*/
	m_fDefaultScale = 1.f;
	m_fDefaultOffset = {};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

	SDL_DisplayMode displayMode{};
	::SDL_GetCurrentDisplayMode(0, &displayMode);
	int iDesktopWidth = displayMode.w;
	int iDesktopHeight = displayMode.h;

	if (iSkeletonWidth > iDesktopWidth || iSkeletonHeight > iDesktopHeight)
	{
		float fScaleX = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDesktopHeight) / iSkeletonHeight;

		constexpr float kfMinScale = 0.425f;
		if (fScaleX < kfMinScale)fScaleX = kfMinScale;
		if (fScaleY < kfMinScale)fScaleY = kfMinScale;

		m_fDefaultScale = iDesktopWidth < iDesktopHeight ? fScaleY : fScaleX;
		constexpr float kfMinOffsetScale = 0.5f;
		float fOffsetScale = m_fDefaultScale;
		if (fOffsetScale < kfMinOffsetScale)fOffsetScale = kfMinOffsetScale;

		m_fDefaultOffset.x = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) * fOffsetScale : 0.f;
		m_fDefaultOffset.y = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) * fOffsetScale : 0.f;
	}

	//if (iSkeletonWidth > iDesktopWidth || iSkeletonHeight > iDesktopHeight)
	//{
	//	float fScaleX = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
	//	float fScaleY = static_cast<float>(iDesktopHeight) / iSkeletonHeight;

	//	m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	//}
}

void CSdlSpinePlayer::WorkOutDefaultOffset()
{
	//m_fDefaultOffset = {};

	//float fMinX = FLT_MAX;
	//float fMinY = FLT_MAX;

	//for (const auto& pDrawable : m_drawables)
	//{
	//	const auto& rect = pDrawable->GetBoundingBox();
	//	fMinX = (std::min)(fMinX, rect.x);
	//	fMinY = (std::min)(fMinY, rect.y);
	//}

	//m_fDefaultOffset.x = fMinX / 2;
	//m_fDefaultOffset.y = fMinY / 2;
}

