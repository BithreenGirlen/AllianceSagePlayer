#ifndef SDL_MAIN_WINDOW_H_
#define SDL_MAIN_WINDOW_H_

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "sdl_spine_player.h"
#include "sdl_clock.h"
#include "adv.h"

/*Microsoft Media Foundation is used because SDL_mixer does not support AAC format.*/
#include "mf_media_player.h"

class CSdlMainWindow
{
public:
	enum class EBackEnd
	{
		kDirectX = 0,
		kOpenGL,
		kVulkan
	};
	CSdlMainWindow(const char* pzWindowName, EBackEnd eBackEnd = EBackEnd::kDirectX);
	~CSdlMainWindow();

	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetFont(const char* szFontFilePath, bool bBold = false, bool bItalic = false);
	void SetTexts(const std::vector<adv::TextDatum>& textData);

	int Display();
private:
	enum EFontSize{kOutLineSize = 1, kFillSize = 32};

	std::shared_ptr<SDL_Window> m_window;
	std::shared_ptr<SDL_Renderer> m_renderer;

	std::shared_ptr<TTF_Font> m_fillFont;
	std::shared_ptr<TTF_Font> m_outlineFont;
	bool bTextColourReversed = false;
	bool bTextHidden = false;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	std::unique_ptr<CSdlSpinePlayer> m_sdlSpinePlayer;
	CSdlClock m_spineClock;

	CMfMediaPlayer m_mfBgPlayer;
	CMfMediaPlayer m_mfVoicePlayer;
	CSdlClock m_voiceClock;

	void ShiftMessageText(bool bForward);
	std::wstring FormatMessageText();

	void SwitchTextColour();

	void RenderText(const std::wstring& wstr, int iPosX = 0, int iPosY = 0);

	void CheckTimer();
};
#endif // !SDL_MAIN_WINDOW_H_
