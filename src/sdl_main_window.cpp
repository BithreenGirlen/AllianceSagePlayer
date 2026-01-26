
#include <SDL3_image/SDL_image.h>

#include "sdl_main_window.h"

CSdlMainWindow::CSdlMainWindow(const char* windowName, EBackEnd eBackEnd, bool transparent)
{
	int iBackEnd = 0;
	switch (eBackEnd)
	{
	case EBackEnd::kOpenGL:
		iBackEnd = SDL_WINDOW_OPENGL;
		break;
	case EBackEnd::kVulkan:
		iBackEnd = SDL_WINDOW_VULKAN;
		break;
	default:
		break;
	}
	m_window = std::shared_ptr<SDL_Window>
		(
			::SDL_CreateWindow(windowName, 200, 200, SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS | iBackEnd | (transparent ? SDL_WINDOW_TRANSPARENT : 0)),
			[](SDL_Window* window)
			{
				::SDL_DestroyWindow(window);
			}
		);

	if (m_window.get() == nullptr)return;

	::SDL_SetWindowPosition(m_window.get(), 0, 0);

	m_renderer = std::shared_ptr<SDL_Renderer>
		(
			::SDL_CreateRenderer(m_window.get(), nullptr),
			[](SDL_Renderer* renderer)
			{
				::SDL_DestroyRenderer(renderer);
			}
		);

	::SDL_SetRenderVSync(m_renderer.get(), 1);

	::SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 0);

	m_sdlSpinePlayer = std::make_unique<CSdlSpinePlayer>(m_window.get(), m_renderer.get());
}

CSdlMainWindow::~CSdlMainWindow()
{

}

bool CSdlMainWindow::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		return m_sdlSpinePlayer.get()->LoadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	}

	return false;
}

void CSdlMainWindow::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		m_sdlSpinePlayer.get()->SetSlotsToExclude(slotNames);
	}
}

void CSdlMainWindow::SetSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		m_sdlSpinePlayer.get()->SetSlotExclusionCallback(pFunc);
	}
}

bool CSdlMainWindow::SetFont(const char* fontFilePath, bool bold, bool italic)
{
	m_fillFont = std::shared_ptr<TTF_Font>
		(
			::TTF_OpenFont(fontFilePath, EFontSize::kFillSize),
			[](TTF_Font* ttfFont)
			{
				::TTF_CloseFont(ttfFont);
			}
		);

	if (m_fillFont.get() == nullptr)return false;

	m_outlineFont = std::shared_ptr<TTF_Font>
		(
			::TTF_OpenFont(fontFilePath, EFontSize::kFillSize),
			[](TTF_Font* ttfFont)
			{
				::TTF_CloseFont(ttfFont);
			}
		);

	::TTF_SetFontStyle(m_fillFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontStyle(m_outlineFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontOutline(m_outlineFont.get(), EFontSize::kOutLineSize);

	return true;
}

void CSdlMainWindow::SetScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_nTextIndex = 0;

	m_animationNames = std::move(animationNames);
	m_nLastAnimationIndex = 0;
}

int CSdlMainWindow::Display()
{
	ResetSpinePlayerScale();
	::SDL_ShowWindow(m_window.get());

	m_pBgPlayer = std::make_unique<CMfMediaPlayer>();
	m_pBgPlayer->SetLoop(true);
	m_pBgPlayer->SetCurrentVolume(0.1);

	int iRet = 0;
	bool toBeQuit = false;
	bool isUnderWindowMove = false;
	bool wasLeftPressed = false;
	bool wasLeftCombinated = false;

	SDL_FPoint mouseStartPos{};

	m_spineClock.Restart();
	m_textClock.Restart();
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
					ShiftMessageText(false);
					break;
				case SDL_SCANCODE_RIGHT:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						ShiftMessageText(true);
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
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->TogglePma();
					}
					break;
				case SDL_SCANCODE_B:
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->ToggleBlendMode();
					}
					break;
				case SDL_SCANCODE_C:
					ToggleTextColour();
					break;
				case SDL_SCANCODE_S:

					break;
				case SDL_SCANCODE_T:
					ToggleTextVisibility();
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
						if (wasLeftCombinated)
						{
							wasLeftCombinated = false;
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

						if (m_sdlSpinePlayer.get() != nullptr)
						{
							if (iX == 0 && iY == 0 && m_animationNames.empty())
							{
								m_sdlSpinePlayer->ShiftAnimation();
							}
						}

						wasLeftPressed = false;
					}
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					ResetSpinePlayerScale();
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
						m_sdlSpinePlayer->MoveViewPoint(iX, iY);

						mouseStartPos = mousePos;

						wasLeftCombinated = true;
					}
				}
			}
			break;
			case SDL_EVENT_MOUSE_WHEEL:
			{
				Uint32 uiButtonState = ::SDL_GetMouseState(nullptr, nullptr);
				if (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
				{
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						constexpr float kTimeScalePortion = 0.05f;

						float timeScale = m_sdlSpinePlayer->GetTimeScale();
						(event.wheel.y < 0) ?
							timeScale += kTimeScalePortion :
							timeScale -= kTimeScalePortion;
						if (timeScale < 0.f)timeScale = 0.f;
						m_sdlSpinePlayer->SetTimeScale(timeScale);

						wasLeftCombinated = true;
					}
				}
				else if (uiButtonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
				{
					ShiftMessageText(event.wheel.y < 0);
				}
				else
				{
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						static constexpr float kScalePortion = 0.025f;
						static constexpr float kMinScale = 0.15f;

						float skeletonScale = m_sdlSpinePlayer->GetSkeletonScale();
						(event.wheel.y < 0) ?
							skeletonScale += kScalePortion :
							skeletonScale -= kScalePortion;
						if (skeletonScale < kMinScale)skeletonScale = kMinScale;
						m_sdlSpinePlayer->SetSkeletonScale(skeletonScale);

						int nKeyCount = 0;
						const bool* pKeyboardState = ::SDL_GetKeyboardState(&nKeyCount);
						if (1 || nKeyCount > SDL_SCANCODE_LCTRL && pKeyboardState[SDL_SCANCODE_LCTRL] == false)
						{
							float canvasScale = m_sdlSpinePlayer->GetCanvasScale();
							(event.wheel.y < 0) ?
								canvasScale += kScalePortion :
								canvasScale -= kScalePortion;
							if (canvasScale < kMinScale)canvasScale = kMinScale;
							m_sdlSpinePlayer->SetCanvasScale(canvasScale);

							//ResizeWindow();
						}
					}
				}
			}
			break;
			default:
				break;
			}
		}

		float deltaTime = m_spineClock.GetElapsedTime();
		m_sdlSpinePlayer->Update(deltaTime);
		m_spineClock.Restart();

		::SDL_RenderClear(m_renderer.get());

		m_sdlSpinePlayer->Redraw();

		RenderText(FormatMessageText());

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

		CheckTimer();
	}

	return iRet;
}

void CSdlMainWindow::ResizeWindow()
{
	if (m_sdlSpinePlayer.get())
	{
		SDL_FPoint fBaseSize = m_sdlSpinePlayer->GetBaseSize();
		float fScale = m_sdlSpinePlayer->GetCanvasScale();
		::SDL_SetWindowSize(m_window.get(), static_cast<int>(fBaseSize.x * fScale), static_cast<int>(fBaseSize.y * fScale));
	}
}

bool CSdlMainWindow::SaveCurrentFrameImage()
{
	if (m_sdlSpinePlayer == nullptr)return false;

	const auto GetFilePathToSave = [this]()
		-> std::string
		{
			const char* pBasePath = ::SDL_GetBasePath();
			if (pBasePath == nullptr)return {};

			const char* pAnimationName = m_sdlSpinePlayer->GetCurrentAnimationName();
			if (pAnimationName == nullptr)return {};

			float fTrackTime = 0.f;
			m_sdlSpinePlayer->GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);

			std::string strPath = pBasePath;
#ifdef _WIN32
			strPath += u8"\\";
#else
			strPath += u8"/";
#endif
			char sBuffer[16]{};
			::SDL_snprintf(sBuffer, sizeof(sBuffer) -1, u8"_%.3f.png", fTrackTime);
			strPath += pAnimationName;
			strPath += sBuffer;

			return strPath;
		};

	std::string strFilePath = GetFilePathToSave();
	if (strFilePath.empty())return false;

	SDL_Rect windowRect{};
	::SDL_GetWindowSize(m_window.get(), &windowRect.w, &windowRect.h);

	constexpr const char slotName[] = "BG";
	SDL_FRect slotRect = m_sdlSpinePlayer->GetCurrentBoundingOfSlot(slotName, sizeof(slotName) - 1);

	auto pSurface = std::unique_ptr<SDL_Surface, decltype(&::SDL_DestroySurface)>
		(
			::SDL_RenderReadPixels(m_renderer.get(), &windowRect),
			::SDL_DestroySurface
		);
	if (pSurface.get() == nullptr)return false;

	return ::IMG_SavePNG(pSurface.get(), strFilePath.c_str());
}

void CSdlMainWindow::ResetSpinePlayerScale()
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		m_sdlSpinePlayer->ResetScale();
		ResizeWindow();
	}
}
/*表示文章移行*/
void CSdlMainWindow::ShiftMessageText(bool forward)
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
	UpdateMessageText();
}

void CSdlMainWindow::UpdateMessageText()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];

	/* Checks if animation has to be switched or not. */
	if (m_nTextIndex == 0 || (m_nLastAnimationIndex != textDatum.nAnimationIndex))
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sdlSpinePlayer->SetAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
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
	m_textClock.Restart();
}
/*表示文章作成*/
std::string CSdlMainWindow::FormatMessageText()
{
	if (m_nTextIndex >= m_textData.size())return {};

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	std::string str = textDatum.strText;
	if (!str.empty() && str.back() != '\n')str += '\n';

	/* size_t is 20 digits at most. */
	char sBuffer[64]{};
	::SDL_snprintf(sBuffer, sizeof(sBuffer) - 1, u8"%zu/%zu", m_nTextIndex + 1, m_textData.size());
	str += sBuffer;
	return str;
}
/*文字色切り替え*/
void CSdlMainWindow::ToggleTextColour()
{
	m_isTextColourReversed ^= true;
}
void CSdlMainWindow::ToggleTextVisibility()
{
	m_isTextHidden ^= true;
}
/*文章描き出し*/
void CSdlMainWindow::RenderText(const std::string& str, int iPosX, int iPosY)
{
	if (m_fillFont.get() == nullptr || m_outlineFont.get() == nullptr)return;

	const SDL_Color kWhite = SDL_Color{ 0xff, 0xff, 0xff, 0xff };
	const SDL_Color kBlack = SDL_Color{ 0x00, 0x00, 0x00, 0xff };

	auto pFillSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_fillFont.get(), str.data(), str.size(), m_isTextColourReversed ? kWhite : kBlack, 0),
			::SDL_DestroySurface
		);

	auto pOutlineSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_outlineFont.get(), str.data(), str.size(), m_isTextColourReversed ? kBlack : kWhite, 0),
			::SDL_DestroySurface
		);

	if (pFillSurface.get() == nullptr || pOutlineSurface.get() == nullptr)return;

	auto pFilledTexture = std::unique_ptr<SDL_Texture, decltype (&::SDL_DestroyTexture)>
		(
			::SDL_CreateTextureFromSurface(m_renderer.get(), pFillSurface.get()),
			::SDL_DestroyTexture
		);

	auto pOutlinedTexture = std::unique_ptr<SDL_Texture, decltype (&::SDL_DestroyTexture)>
		(
			::SDL_CreateTextureFromSurface(m_renderer.get(), pOutlineSurface.get()),
			::SDL_DestroyTexture
		);

	if (pFilledTexture.get() == nullptr || pOutlinedTexture.get() == nullptr)return;

	SDL_FRect textRect{ 
		static_cast<float>(iPosX + EFontSize::kOutLineSize),
		static_cast<float>(iPosY + EFontSize::kOutLineSize),
		static_cast<float>(pOutlineSurface->w),
		static_cast<float>(pOutlineSurface->h)
	};
	::SDL_RenderTexture(m_renderer.get(), pOutlinedTexture.get(), nullptr, &textRect);
	::SDL_RenderTexture(m_renderer.get(), pFilledTexture.get(), nullptr, &textRect);
}

void CSdlMainWindow::CheckTimer()
{
	constexpr float fAutoPlayInterval = 3.f;
	float fSecond = m_textClock.GetElapsedTime();
	if (m_voicePlayer.IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
	}
}
