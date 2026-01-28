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
	enum class EBackend
	{
		kDirectX = 0,
		kOpenGL,
		kVulkan
	};
	CSdlMainWindow(const char* windowName, EBackend eBackEnd = EBackend::kDirectX, bool transparent = false);
	~CSdlMainWindow();

	bool setSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skelFilePaths);

	void setSlotExclusionCallback(bool (*pFunc)(const char*, size_t));

	bool setFont(const char* fontFilePath, bool bold = false, bool italic = false);
	void setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);

	int display();
private:
	enum EFontSize { kOutLineSize = 1, kFillSize = 32 };
	static constexpr float kScaleDelta = 0.01f;

	std::unique_ptr<SDL_Window, decltype(&::SDL_DestroyWindow)> m_window{ nullptr, ::SDL_DestroyWindow};
	std::unique_ptr<SDL_Renderer, decltype(&::SDL_DestroyRenderer)> m_renderer{nullptr, ::SDL_DestroyRenderer };

	std::unique_ptr<CSdlSpinePlayer> m_sdlSpinePlayer;
	CSdlClock m_spineClock;

	std::unique_ptr<SDL_Texture, decltype(&::SDL_DestroyTexture)> m_spineTexture {nullptr, ::SDL_DestroyTexture };

	void resizeWindow();

	bool saveCurrentFrameImage();

	void resetSpinePlayerScale();
	void setSpinePlayerSize();

	std::unique_ptr<TTF_Font, decltype(&::TTF_CloseFont)> m_fillFont { nullptr, ::TTF_CloseFont };
	std::unique_ptr<TTF_Font, decltype(&::TTF_CloseFont)> m_outlineFont{ nullptr, ::TTF_CloseFont };
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
