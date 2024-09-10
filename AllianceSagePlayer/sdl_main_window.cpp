
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
			::SDL_CreateWindow(pzWindowName, 0, 0, 200, 200, SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | iBackEnd),
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

	m_sdlSpinePlayer = std::make_unique<CSdlSpinePlayer>(m_window.get(), m_renderer.get());
}

CSdlMainWindow::~CSdlMainWindow()
{

}

bool CSdlMainWindow::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (m_sdlSpinePlayer.get() != nullptr)
	{
		return m_sdlSpinePlayer.get()->SetSpineFromFile(atlasPaths, skelPaths, bIsBinary);
	}

	return false;
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

	m_mfBgPlayer.SwitchLoop();
}

int CSdlMainWindow::Display()
{
	::SDL_ShowWindow(m_window.get());

	int iRet = 0;
	bool bToBeQuit = false;
	bool bOnWindowMove = false;
	bool bSpeedHavingChanged = false;

	SDL_Point iMouseStartPos{};
	
	::SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 255, 255);
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
						m_sdlSpinePlayer->SwitchPma();
					}
					break;
				case SDL_SCANCODE_B:
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->SwitchBlendModeAdoption();
					}
					break;
				case SDL_SCANCODE_C:
					SwitchTextColour();
					break;
				case SDL_SCANCODE_S:

					break;
				case SDL_SCANCODE_T:
					bTextHidden ^= true;
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
					break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					{
						if (bSpeedHavingChanged)
						{
							bSpeedHavingChanged = false;
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
							else
							{
								m_sdlSpinePlayer->MoveViewPoint(iX, iY);
							}
						}
					}
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					if (m_sdlSpinePlayer.get() != nullptr)
					{
						m_sdlSpinePlayer->ResetScale();
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
						bSpeedHavingChanged = true;
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
					}
				}
			}
			break;
			default:

				break;
			}
		}

		float deltaTime = m_spineClock.GetElapsedTime();
		m_spineClock.Restart();

		::SDL_RenderClear(m_renderer.get());

		if (m_sdlSpinePlayer.get() != nullptr)
		{
			m_sdlSpinePlayer->Redraw(deltaTime);
		}

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
			m_mfBgPlayer.Play(wstr.c_str());
		}
		else
		{
			m_mfVoicePlayer.Play(wstr.c_str());
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
void CSdlMainWindow::SwitchTextColour()
{
	bTextColourReversed ^= true;
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
			::TTF_RenderUNICODE_Blended_Wrapped(m_fillFont.get(), textBuffer.data(), bTextColourReversed ? kWhite : kBlack, 0),
			[](SDL_Surface* surface)
			{
				::SDL_FreeSurface(surface);
			}
		);

	auto pOutlineSurface = std::shared_ptr<SDL_Surface>
		(
			::TTF_RenderUNICODE_Blended_Wrapped(m_outlineFont.get(), textBuffer.data(), bTextColourReversed ? kBlack : kWhite, 0),
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
	if (m_mfVoicePlayer.IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
	}
}
