

#include <SDL3_image/SDL_image.h>

#include "sdl_main_window.h"

CSdlMainWindow::CSdlMainWindow(const char* windowName, EBackend eBackEnd, bool transparent)
{
	int iBackend = 0;
	switch (eBackEnd)
	{
	case EBackend::kOpenGL:
		iBackend = SDL_WINDOW_OPENGL;
		break;
	case EBackend::kVulkan:
		iBackend = SDL_WINDOW_VULKAN;
		break;
	default:
		break;
	}
	m_window.reset(::SDL_CreateWindow(
		windowName, 200, 200,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS | iBackend | (transparent ? SDL_WINDOW_TRANSPARENT : 0))
	);

	if (m_window == nullptr)return;

	::SDL_SetWindowPosition(m_window.get(), 0, 0);

	m_renderer.reset(::SDL_CreateRenderer(m_window.get(), nullptr));

	::SDL_SetRenderVSync(m_renderer.get(), 1);

	::SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 0);

	m_sdlSpinePlayer = std::make_unique<CSdlSpinePlayer>(m_window.get(), m_renderer.get());
}

CSdlMainWindow::~CSdlMainWindow()
{

}

bool CSdlMainWindow::setSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skelFilePaths)
{
	if (m_sdlSpinePlayer != nullptr)
	{
		bool bRet = m_sdlSpinePlayer->loadSpineFromFile(atlasFilePaths, skelFilePaths);
		if (bRet)
		{
			/* Filename including extension. */
			size_t nPos = atlasFilePaths[0].find_last_of("\\/");
			if (nPos == std::string::npos)nPos = 0;
			else ++nPos;
			::SDL_SetWindowTitle(m_window.get(), &atlasFilePaths[0][nPos]);

			setSpinePlayerSize();
			m_sdlSpinePlayer->setSlotsToExclude({ "frame", "Frame", "AA", reinterpret_cast<const char*>(u8"出血"), "sdvsdv", reinterpret_cast<const char*>(u8"出血框_註解")});

			return true;
		}
	}

	return false;
}

void CSdlMainWindow::setSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	if (m_sdlSpinePlayer != nullptr)
	{
		m_sdlSpinePlayer->setSlotExclusionCallback(pFunc);
	}
}

bool CSdlMainWindow::setFont(const char* fontFilePath, bool bold, bool italic)
{
	float dpi = ::SDL_GetWindowDisplayScale(m_window.get());
	m_fillFont.reset(::TTF_OpenFont(fontFilePath, EFontSize::kFillSize * dpi));

	if (m_fillFont == nullptr)return false;

	m_outlineFont.reset(::TTF_OpenFont(fontFilePath, EFontSize::kFillSize * dpi));

	::TTF_SetFontStyle(m_fillFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontStyle(m_outlineFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontOutline(m_outlineFont.get(), EFontSize::kOutLineSize);

	return true;
}

void CSdlMainWindow::setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_nTextIndex = 0;

	m_animationNames = std::move(animationNames);
	m_nLastAnimationIndex = 0;
}

int CSdlMainWindow::display()
{
	resetSpinePlayerScale();
	::SDL_ShowWindow(m_window.get());

	m_pBgPlayer = std::make_unique<CMfMediaPlayer>();
	m_pBgPlayer->SetLoop(true);
	m_pBgPlayer->SetCurrentVolume(0.1);

	updateMessageText();

	int iRet = 0;
	bool toBeQuit = false;
	bool isUnderWindowMove = false;
	bool wasLeftPressed = false;
	bool wasLeftCombined = false;

	SDL_FPoint mouseStartPos{};

	m_spineClock.restart();
	m_textClock.restart();
	while (!toBeQuit)
	{
		SDL_Event event;
		while (::SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				toBeQuit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode)
				{
				case SDL_SCANCODE_LEFT:
					shiftMessageText(false);
					break;
				case SDL_SCANCODE_RIGHT:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						shiftMessageText(true);
					}
					break;
				default:
					break;
				}
				break;
			case SDL_EVENT_KEY_UP:
				switch (event.key.scancode)
				{
				case SDL_SCANCODE_A:
					if (m_sdlSpinePlayer != nullptr)
					{
						m_sdlSpinePlayer->togglePma();
					}
					break;
				case SDL_SCANCODE_B:
					if (m_sdlSpinePlayer != nullptr)
					{
						m_sdlSpinePlayer->toggleBlendMode();
					}
					break;
				case SDL_SCANCODE_C:
					toggleTextColour();
					break;
				case SDL_SCANCODE_S:
					saveCurrentFrameImage();
					break;
				case SDL_SCANCODE_T:
					toggleTextVisibility();
					break;
				case SDL_SCANCODE_ESCAPE:
					toBeQuit = true;
					break;
				case SDL_SCANCODE_UP:
					iRet = 2;
					toBeQuit = true;
					break;
				case SDL_SCANCODE_DOWN:
					iRet = 1;
					toBeQuit = true;
					break;
				}
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					::SDL_GetMouseState(&mouseStartPos.x, &mouseStartPos.y);
					wasLeftPressed = true;
					break;
				}
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					{
						if (wasLeftCombined)
						{
							wasLeftCombined = false;
							wasLeftPressed = false;
							break;
						}

						Uint32 uiButtonState = ::SDL_GetMouseState(nullptr, nullptr);

						if (isUnderWindowMove || (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)))
						{
							isUnderWindowMove ^= true;
							break;
						}

						SDL_FPoint mouseEndPos{};
						::SDL_GetMouseState(&mouseEndPos.x, &mouseEndPos.y);

						int iX = static_cast<int>(mouseStartPos.x - mouseEndPos.x);
						int iY = static_cast<int>(mouseStartPos.y - mouseEndPos.y);

						if (m_sdlSpinePlayer != nullptr)
						{
							if (iX == 0 && iY == 0 && m_animationNames.empty())
							{
								m_sdlSpinePlayer->shiftAnimation();
							}
						}

						wasLeftPressed = false;
					}
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					resetSpinePlayerScale();
				}
				break;
			case SDL_EVENT_MOUSE_MOTION:
			{
				SDL_FPoint mousePos{};
				Uint32 uiButtonState = ::SDL_GetMouseState(&mousePos.x, &mousePos.y);
				if (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
				{
					if (wasLeftPressed)
					{
						int iX = static_cast<int>(mouseStartPos.x - mousePos.x);
						int iY = static_cast<int>(mouseStartPos.y - mousePos.y);
						m_sdlSpinePlayer->addOffset(iX, iY);

						mouseStartPos = mousePos;

						wasLeftCombined = true;
					}
				}
			}
			break;
			case SDL_EVENT_MOUSE_WHEEL:
			{
				const float scrollSign = (event.wheel.y < 0 ? 1.f : -1.f);
				Uint32 uiButtonState = ::SDL_GetMouseState(nullptr, nullptr);
				if (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
				{
					if (m_sdlSpinePlayer != nullptr)
					{
						constexpr float kTimeScaleDelta = 0.05f;

						float timeScale = m_sdlSpinePlayer->getTimeScale() + kTimeScaleDelta * scrollSign;
						if (timeScale < 0.f)timeScale = 0.f;
						m_sdlSpinePlayer->setTimeScale(timeScale);

						wasLeftCombined = true;
					}
				}
				else if (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
				{
					shiftMessageText(event.wheel.y < 0);
				}
				else
				{
					if (m_sdlSpinePlayer != nullptr)
					{
						static constexpr float kMinScale = 0.15f;

						float skeletonScale = m_sdlSpinePlayer->getSkeletonScale() + kScaleDelta * scrollSign;
						if (skeletonScale < kMinScale)skeletonScale = kMinScale;
						m_sdlSpinePlayer->setSkeletonScale(skeletonScale);

						int nKeyCount = 0;
						const bool* pKeyboardState = ::SDL_GetKeyboardState(&nKeyCount);
						if (nKeyCount > SDL_SCANCODE_LCTRL && pKeyboardState[SDL_SCANCODE_LCTRL] == false)
						{
							float canvasScale = m_sdlSpinePlayer->getCanvasScale() + kScaleDelta * scrollSign;
							if (canvasScale < kMinScale)canvasScale = kMinScale;
							m_sdlSpinePlayer->setCanvasScale(canvasScale);

							resizeWindow();
						}
					}
				}
			}
			break;
			default:
				break;
			}
		}

		float deltaTime = m_spineClock.getElapsedTime();
		m_sdlSpinePlayer->update(deltaTime);
		m_spineClock.restart();

		::SDL_RenderClear(m_renderer.get());

		if (m_spineTexture != nullptr)
		{
			::SDL_SetRenderTarget(m_renderer.get(), m_spineTexture.get());
			::SDL_RenderClear(m_renderer.get());
			m_sdlSpinePlayer->redraw();
			::SDL_SetRenderTarget(m_renderer.get(), nullptr);

			::SDL_RenderTexture(m_renderer.get(), m_spineTexture.get(), nullptr, nullptr);
		}
		if (!m_isTextHidden)
		{
			renderText();
		}

		::SDL_RenderPresent(m_renderer.get());

		if (isUnderWindowMove)
		{
			SDL_FPoint currentMousePos{};
			::SDL_GetGlobalMouseState(&currentMousePos.x, &currentMousePos.y);

			SDL_Rect windowRect{};
			::SDL_GetWindowPosition(m_window.get(), &windowRect.x, &windowRect.y);
			::SDL_GetWindowSize(m_window.get(), &windowRect.w, &windowRect.h);

			SDL_Point windowPosToBe{};
			windowPosToBe.x = static_cast<int>(currentMousePos.x - (windowRect.w / 2.0));
			windowPosToBe.y = static_cast<int>(currentMousePos.y - (windowRect.h / 2.0));

			::SDL_SetWindowPosition(m_window.get(), windowPosToBe.x, windowPosToBe.y);
		}

		checkTimer();
	}

	return iRet;
}

void CSdlMainWindow::resizeWindow()
{
	if (m_sdlSpinePlayer != nullptr && m_window != nullptr)
	{
		SDL_FPoint fBaseSize = m_sdlSpinePlayer->getBaseSize();
		float fScale = m_sdlSpinePlayer->getCanvasScale();

		SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_window.get());
		if (displayId == 0)return;

		const SDL_DisplayMode* pDisplayMode = ::SDL_GetCurrentDisplayMode(displayId);
		if (pDisplayMode == nullptr)return;

		int MaxWindowWidth = static_cast<int>(fBaseSize.x * (fScale - kScaleDelta));
		int MaxwindowHeight = static_cast<int>(fBaseSize.y * (fScale - kScaleDelta));

		if (MaxWindowWidth < pDisplayMode->w || MaxwindowHeight < pDisplayMode->h)
		{
			int windowWidth = static_cast<int>(fBaseSize.x * fScale);
			int windowHeight = static_cast<int>(fBaseSize.y * fScale);

			::SDL_SetWindowSize(m_window.get(), static_cast<int>(fBaseSize.x * fScale), static_cast<int>(fBaseSize.y * fScale));

			m_spineTexture.reset(::SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windowWidth, windowHeight));
		}
	}
}
/* This would actually save one frame before I suppose. */
bool CSdlMainWindow::saveCurrentFrameImage()
{
	if (m_sdlSpinePlayer == nullptr || m_spineTexture == nullptr)return false;

	const auto GetFilePathToSave = [this]()
		-> std::string
		{
			const char* pBasePath = ::SDL_GetBasePath();
			if (pBasePath == nullptr)return {};

			const char* pAnimationName = m_sdlSpinePlayer->getCurrentAnimationName();
			if (pAnimationName == nullptr)return {};

			float fTrackTime = 0.f;
			m_sdlSpinePlayer->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);

			std::string strPath = pBasePath;
			strPath += ::SDL_GetWindowTitle(m_window.get());
			::SDL_CreateDirectory(strPath.c_str());
#ifdef _WIN32
			strPath += '\\';
#else
			strPath += '/';
#endif
			char sBuffer[16]{};
			::SDL_snprintf(sBuffer, sizeof(sBuffer) - 1, "_%.3f.png", fTrackTime);
			strPath += pAnimationName;
			strPath += sBuffer;

			return strPath;
		};

	std::string strFilePath = GetFilePathToSave();
	if (strFilePath.empty())return false;

	::SDL_SetRenderTarget(m_renderer.get(), m_spineTexture.get());

	auto pSurface = std::unique_ptr<SDL_Surface, decltype(&::SDL_DestroySurface)>
		(
			::SDL_RenderReadPixels(m_renderer.get(), nullptr),
			::SDL_DestroySurface
		);
	if (pSurface == nullptr)return false;

	::SDL_SetRenderTarget(m_renderer.get(), nullptr);

	return ::IMG_SavePNG(pSurface.get(), strFilePath.c_str());
}

void CSdlMainWindow::resetSpinePlayerScale()
{
	if (m_sdlSpinePlayer != nullptr)
	{
		m_sdlSpinePlayer->resetScale();

		SDL_FPoint fBaseSize = m_sdlSpinePlayer->getBaseSize();

		SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_window.get());
		if (displayId == 0)return;

		const SDL_DisplayMode* pDisplayMode = ::SDL_GetCurrentDisplayMode(displayId);
		if (pDisplayMode == nullptr)return;

		float fScaleX = pDisplayMode->w / fBaseSize.x;
		float fScaleY = pDisplayMode->h / fBaseSize.y;

		float fScale = fScaleX > fScaleY ? fScaleX : fScaleY;
		m_sdlSpinePlayer->setSkeletonScale(fScale / 0.945f);
		m_sdlSpinePlayer->setCanvasScale(fScale);

		resizeWindow();
	}
}

void CSdlMainWindow::setSpinePlayerSize()
{
	if (m_sdlSpinePlayer != nullptr)
	{
		/*
		* The first slot, of which name varies though, is the background slot,
		* and the last one, mostly named "frame", is mask slot.
		* This applies in most cases, but not always.
		*/

		m_sdlSpinePlayer->setOffset(96, 24);
		/* 16: 9 */
		m_sdlSpinePlayer->setBaseSize(4096.f, 2304.f);
	}
}
/*表示文章移行*/
void CSdlMainWindow::shiftMessageText(bool forward)
{
	if (m_textData.empty())return;

	if (forward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	updateMessageText();
}

void CSdlMainWindow::updateMessageText()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];

	/* Checks if animation has to be switched or not. */
	if (m_nTextIndex == 0 || (m_nLastAnimationIndex != textDatum.nAnimationIndex))
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sdlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
		}
	}

	const std::wstring& wstrVoicePath = textDatum.wstrVoicePath;
	if (!wstrVoicePath.empty())
	{
		if (wstrVoicePath.find(L"_loop") != std::wstring::npos)
		{
			m_pBgPlayer->Play(wstrVoicePath.c_str());
		}
		else
		{
			m_voicePlayer.Play(wstrVoicePath.c_str());
		}
	}

	prepareMessageText();
	m_textClock.restart();
}
/* 描画文字列作成 */
void CSdlMainWindow::prepareMessageText()
{
	formatMessageText();

	const SDL_Color kWhite = SDL_Color{ 0xff, 0xff, 0xff, 0xff };
	const SDL_Color kBlack = SDL_Color{ 0x00, 0x00, 0x00, 0xff };

	auto pFillSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_fillFont.get(), m_messageText.data(), m_messageText.size(), m_isTextColourReversed ? kWhite : kBlack, 0),
			::SDL_DestroySurface
		);

	auto pOutlineSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_outlineFont.get(), m_messageText.data(), m_messageText.size(), m_isTextColourReversed ? kBlack : kWhite, 0),
			::SDL_DestroySurface
		);

	if (pFillSurface == nullptr || pOutlineSurface == nullptr)return;

	m_fillTexture.reset(::SDL_CreateTextureFromSurface(m_renderer.get(), pFillSurface.get()));
	m_outlineTexture.reset(::SDL_CreateTextureFromSurface(m_renderer.get(), pOutlineSurface.get()));
}
/*表示文章作成*/
void CSdlMainWindow::formatMessageText()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	m_messageText.assign(textDatum.strText);
	if (!m_messageText.empty() && m_messageText.back() != '\n')m_messageText += '\n';

	/* size_t is 20 digits at most. */
	char sBuffer[64]{};
	::SDL_snprintf(sBuffer, sizeof(sBuffer) - 1, "%zu/%zu", m_nTextIndex + 1, m_textData.size());
	m_messageText += sBuffer;
}
/*文字色切り替え*/
void CSdlMainWindow::toggleTextColour()
{
	m_isTextColourReversed ^= true;
	prepareMessageText();
}
void CSdlMainWindow::toggleTextVisibility()
{
	m_isTextHidden ^= true;
}
/*文章描画*/
void CSdlMainWindow::renderText(int iPosX, int iPosY)
{
	if (m_fillTexture == nullptr || m_outlineTexture == nullptr)return;

	SDL_FRect outlineRect{};
	::SDL_GetTextureSize(m_outlineTexture.get(), &outlineRect.w, &outlineRect.h);
	outlineRect.x = static_cast<float>(iPosX);
	outlineRect.y = static_cast<float>(iPosY);

	SDL_FRect fillRect{};
	::SDL_GetTextureSize(m_fillTexture.get(), &fillRect.w, &fillRect.h);
	float thickness = (outlineRect.w - fillRect.w) / 2.f;

	fillRect.x += thickness;
	fillRect.y += thickness;

	::SDL_RenderTexture(m_renderer.get(), m_outlineTexture.get(), nullptr, &outlineRect);
	::SDL_RenderTexture(m_renderer.get(), m_fillTexture.get(), nullptr, &fillRect);
}

void CSdlMainWindow::checkTimer()
{
	constexpr float fAutoPlayInterval = 3.f;
	float fSecond = m_textClock.getElapsedTime();
	if (m_voicePlayer.IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			shiftMessageText(true);
		}
	}
}
