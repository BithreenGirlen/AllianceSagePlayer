#ifndef SDL_MAIN_WINDOW_H_
#define SDL_MAIN_WINDOW_H_

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "sdl3-spine-cpp/sdl_spine_player.h"
#include "sdl_clock.h"
#include "adv.h"

/* Microsoft Media Foundation is used because SDL_mixer does not support AAC format. */
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
	CSdlMainWindow(const char* windowName, EBackEnd eBackEnd = EBackEnd::kDirectX, bool transparent = false);
	~CSdlMainWindow();

	bool setSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skelFilePaths);

	void setSlotsToExclude(const std::vector<std::string>& slotNames);
	void setSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	bool setFont(const char* fontFilePath, bool bold = false, bool italic = false);
	void setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);

	int display();
private:
	enum EFontSize { kOutLineSize = 1, kFillSize = 32 };

	std::shared_ptr<SDL_Window> m_window;
	std::shared_ptr<SDL_Renderer> m_renderer;

	std::unique_ptr<CSdlSpinePlayer> m_sdlSpinePlayer;
	CSdlClock m_spineClock;

	void resizeWindow();

	bool saveCurrentFrameImage();

	void resetSpinePlayerScale();

	std::shared_ptr<TTF_Font> m_fillFont;
	std::shared_ptr<TTF_Font> m_outlineFont;
	bool m_isTextColourReversed = false;
	bool m_isTextHidden = false;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	std::vector<std::string> m_animationNames;
	size_t m_nLastAnimationIndex = 0;

	std::unique_ptr<CMfMediaPlayer> m_pBgPlayer;
	CMfMediaPlayer m_voicePlayer;
	CSdlClock m_textClock;

	void shiftMessageText(bool forward);
	void updateMessageText();
	std::string formatMessageText();

	void toggleTextColour();
	void toggleTextVisibility();

	void renderText(const std::string& str, int iPosX = 0, int iPosY = 0);

	void checkTimer();
};
#endif // !SDL_MAIN_WINDOW_H_
