
#include <SDL2_image/SDL_image.h>

#include "sdl_main_window.h"

CSdlMainWindow::CSdlMainWindow(const char* pzWindowName, EBackEnd eBackEnd)
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
			::SDL_CreateWindow(pzWindowName, 0, 0, 200, 200, SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS | iBackEnd),
			[](SDL_Window* window)
			{
				::SDL_DestroyWindow(window);
			}
		);

	if (m_window.get() == nullptr)return;

	m_renderer = std::shared_ptr<SDL_Renderer>
		(
			::SDL_CreateRenderer(m_window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
			[](SDL_Renderer* renderer)
			{
				::SDL_DestroyRenderer(renderer);
			}
		);

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

bool CSdlMainWindow::SetFont(const char* szFontFilePath, bool bBold, bool bItalic)
{
	m_fillFont = std::shared_ptr<TTF_Font>
		(
			::TTF_OpenFont(szFontFilePath, EFontSize::kFillSize),
			[](TTF_Font* ttfFont)
			{
				::TTF_CloseFont(ttfFont);
			}
		);

	if (m_fillFont.get() == nullptr)return false;

	m_outlineFont = std::shared_ptr<TTF_Font>
		(
			::TTF_OpenFont(szFontFilePath, EFontSize::kFillSize),
			[](TTF_Font* ttfFont)
			{
				::TTF_CloseFont(ttfFont);
			}
		);

	::TTF_SetFontStyle(m_fillFont.get(), (bBold ? TTF_STYLE_BOLD : 0) | (bItalic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontStyle(m_outlineFont.get(), (bBold ? TTF_STYLE_BOLD : 0) | (bItalic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontOutline(m_outlineFont.get(), EFontSize::kOutLineSize);

	return true;
}

void CSdlMainWindow::SetTexts(const std::vector<adv::TextDatum>& textData)
{
	m_textData = textData;
	m_nTextIndex = 0;
}

int CSdlMainWindow::Display()
{
	ResetSpinePlayerScale();
	::SDL_ShowWindow(m_window.get());

	m_pBgPlayer = std::make_unique<CMfMediaPlayer>();
	m_pBgPlayer->SwitchLoop();

	int iRet = 0;
	bool bToBeQuit = false;
	bool bOnWindowMove = false;
	bool bLeftDowned = false;
	bool bLeftCombinated = false;

	SDL_Point iMouseStartPos{};

	m_spineClock.Restart();
	m_voiceClock.Restart();
	while (!bToBeQuit)
	{
		SDL_Event event;
		while (::SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				bToBeQuit = true;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode)
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
						m_sdlSpinePlayer->ToggleBlendModeAdoption();
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
					bToBeQuit = true;
					break;
				case SDL_SCANCODE_UP:
					iRet = 2;
					bToBeQuit = true;
					break;
				case SDL_SCANCODE_DOWN:
					iRet = 1;
					bToBeQuit = true;
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					::SDL_GetMouseState(&iMouseStartPos.x, &iMouseStartPos.y);
					bLeftDowned = true;
					break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					{
						if (bLeftCombinated)
						{
							bLeftCombinated = false;
							bLeftDowned = false;
							break;
						}

						Uint32 uiButtonState = ::SDL_GetMouseState(nullptr, nullptr);

						if (bOnWindowMove || (uiButtonState & SDL_BUTTON(SDL_BUTTON_RIGHT)))
						{
							bOnWindowMove ^= true;
							break;
						}

						SDL_Point iMouseEndPos{};
						::SDL_GetMouseState(&iMouseEndPos.x, &iMouseEndPos.y);

						int iX = iMouseStartPos.x - iMouseEndPos.x;
						int iY = iMouseStartPos.y - iMouseEndPos.y;

						if (m_sdlSpinePlayer.get() != nullptr)
						{
							if (iX == 0 && iY == 0)
							{
								m_sdlSpinePlayer->ShiftAnimation();
							}
						}

						bLeftDowned = false;
					}
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					ResetSpinePlayerScale();
				}
				break;
			case SDL_MOUSEMOTION:
			{
				SDL_Point iMouseMovePos{};
				Uint32 uiButtonState = ::SDL_GetMouseState(&iMouseMovePos.x, &iMouseMovePos.y);
				if (uiButtonState & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					if (bLeftDowned)
					{
						int iX = iMouseStartPos.x - iMouseMovePos.x;
						int iY = iMouseStartPos.y - iMouseMovePos.y;
						m_sdlSpinePlayer->MoveViewPoint(iX, iY);

						iMouseStartPos = iMouseMovePos;

						bLeftCombinated = true;
					}
				}
			}
			break;
			case SDL_MOUSEWHEEL:
			{
				Uint32 uiButtonState = ::SDL_GetMouseState(nullptr, nullptr);
				if (uiButtonState & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->RescaleTime(event.wheel.y < 0);
						bLeftCombinated = true;
					}
				}
				else if (uiButtonState & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					ShiftMessageText(event.wheel.y < 0);
				}
				else
				{
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->RescaleSkeleton(event.wheel.y < 0);

						int nKeyCount = 0;
						const Uint8* pKeyboardState = ::SDL_GetKeyboardState(&nKeyCount);
						if (nKeyCount > SDL_SCANCODE_LCTRL && pKeyboardState[SDL_SCANCODE_LCTRL] == 0)
						{
							m_sdlSpinePlayer->RescaleCanvas(event.wheel.y < 0);
							ResizeWindow();
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

		if (bOnWindowMove)
		{
			SDL_Point currentMousePos{};
			::SDL_GetGlobalMouseState(&currentMousePos.x, &currentMousePos.y);

			SDL_Rect windowRect{};
			::SDL_GetWindowPosition(m_window.get(), &windowRect.x, &windowRect.y);
			::SDL_GetWindowSize(m_window.get(), &windowRect.w, &windowRect.h);

			SDL_Point windowPosToBe{};
			windowPosToBe.x = currentMousePos.x - (windowRect.w / 2);
			windowPosToBe.y = currentMousePos.y - (windowRect.h / 2);

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
	const auto GetFilePathToSave = [this]()
		-> std::string
		{
			auto pBasePath = std::shared_ptr<char>
				(
					::SDL_GetBasePath(),
					[](char* pPath)
					{
						if (pPath != nullptr)::SDL_free(pPath);
					}
				);
			if (pBasePath.get() == nullptr)return std::string();

			const char* pzAnimationName = m_sdlSpinePlayer->GetCurrentAnimationName();
			if (pzAnimationName == nullptr)return std::string();
			float fTrackTime = 0.f;
			m_sdlSpinePlayer->GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);

			std::string strPath = pBasePath.get();
#ifdef _WIN32
			strPath += u8"\\";
#else
			strPath += u8"/";
#endif
			strPath += pzAnimationName + std::to_string(fTrackTime) + u8".png";
			return strPath;
		};

	std::string strFilePath = GetFilePathToSave();
	if (strFilePath.empty())return false;

	SDL_Rect windowRect{};
	::SDL_GetWindowSize(m_window.get(), &windowRect.w, &windowRect.h);

	auto pSurface = std::shared_ptr<SDL_Surface>
		(
			::SDL_CreateRGBSurfaceWithFormat(0, windowRect.w, windowRect.h, 32, SDL_PIXELFORMAT_ARGB8888),
			[](SDL_Surface* pSdlSurface)
			{
				::SDL_FreeSurface(pSdlSurface);
			}
		);
	if (pSurface.get() == nullptr)return false;

	int iRet = ::SDL_RenderReadPixels(m_renderer.get(), nullptr, pSurface->format->format, pSurface->pixels, pSurface->pitch);
	if (iRet < 0)return false;

	Uint32 uiPic = ::SDL_MapRGBA(pSurface->format, 0, 0, 0, 255);
	if (SDL_MUSTLOCK(pSurface.get()))::SDL_LockSurface(pSurface.get());
	Uint32* pPixels = static_cast<Uint32*>(pSurface->pixels);
	for (int i = 0; i < pSurface->w * pSurface->h; ++i)
	{
		if (pPixels[i] == uiPic)
		{
			pPixels[i] = 0;
		}
	}
	if (SDL_MUSTLOCK(pSurface.get()))::SDL_UnlockSurface(pSurface.get());

	iRet = ::IMG_SavePNG(pSurface.get(), strFilePath.c_str());

	return iRet == 0;
}

void CSdlMainWindow::ResetSpinePlayerScale()
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		m_sdlSpinePlayer->ResetScale();
		ResizeWindow();
		//m_sdlSpinePlayer->SetZoom(1.215f);
	}
}
/*表示文章移行*/
void CSdlMainWindow::ShiftMessageText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}

	std::wstring wstr = m_textData.at(m_nTextIndex).wstrVoicePath;
	if (!wstr.empty())
	{
		if (wstr.find(L"_loop") != std::wstring::npos)
		{
			m_pBgPlayer->Play(wstr.c_str());
		}
		else
		{
			m_voicePlayer.Play(wstr.c_str());
		}
	}
	m_voiceClock.Restart();
}
/*表示文章作成*/
std::wstring CSdlMainWindow::FormatMessageText()
{
	if (m_textData.empty() || m_nTextIndex > m_textData.size() - 1)return std::wstring();

	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText;
	if (!wstr.empty() && wstr.back() != '\n')wstr += '\n';
	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	return wstr;
}
/*文字色切り替え*/
void CSdlMainWindow::ToggleTextColour()
{
	m_bTextColourReversed ^= true;
}
void CSdlMainWindow::ToggleTextVisibility()
{
	m_bTextHidden ^= true;
}
/*文章描き出し*/
void CSdlMainWindow::RenderText(const std::wstring& wstr, int iPosX, int iPosY)
{
	if (m_fillFont.get() == nullptr || m_outlineFont.get() == nullptr)return;

	const SDL_Color kWhite = SDL_Color{ 0xff, 0xff, 0xff };
	const SDL_Color kBlack = SDL_Color{ 0x00, 0x00, 0x00 };

	std::vector<Uint16> textBuffer;
	textBuffer.reserve(wstr.size());
	for (const auto& c : wstr)
	{
		textBuffer.push_back(c);
	}
	textBuffer.push_back(L'\0');

	auto pFillSurface = std::shared_ptr<SDL_Surface>
		(
			::TTF_RenderUNICODE_Blended_Wrapped(m_fillFont.get(), textBuffer.data(), m_bTextColourReversed ? kWhite : kBlack, 0),
			[](SDL_Surface* surface)
			{
				::SDL_FreeSurface(surface);
			}
		);

	auto pOutlineSurface = std::shared_ptr<SDL_Surface>
		(
			::TTF_RenderUNICODE_Blended_Wrapped(m_outlineFont.get(), textBuffer.data(), m_bTextColourReversed ? kBlack : kWhite, 0),
			[](SDL_Surface* surface)
			{
				::SDL_FreeSurface(surface);
			}
		);

	if (pFillSurface.get() == nullptr || pOutlineSurface.get() == nullptr)return;

	auto pFilledTexture = std::shared_ptr<SDL_Texture>
		(
			::SDL_CreateTextureFromSurface(m_renderer.get(), pFillSurface.get()),
			[](SDL_Texture* texture)
			{
				::SDL_DestroyTexture(texture);
			}
		);

	auto pOutlinedTexture = std::shared_ptr<SDL_Texture>
		(
			::SDL_CreateTextureFromSurface(m_renderer.get(), pOutlineSurface.get()),
			[](SDL_Texture* texture)
			{
				::SDL_DestroyTexture(texture);
			}
		);

	if (pFilledTexture.get() == nullptr || pOutlinedTexture.get() == nullptr)return;

	SDL_Rect textRect{ iPosX + EFontSize::kOutLineSize, iPosY + EFontSize::kOutLineSize, pOutlineSurface->w, pOutlineSurface->h };
	::SDL_RenderCopy(m_renderer.get(), pOutlinedTexture.get(), nullptr, &textRect);
	::SDL_RenderCopy(m_renderer.get(), pFilledTexture.get(), nullptr, &textRect);
}

void CSdlMainWindow::CheckTimer()
{
	constexpr float fAutoPlayInterval = 3.f;
	float fSecond = m_voiceClock.GetElapsedTime();
	if (m_voicePlayer.IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
	}
}
