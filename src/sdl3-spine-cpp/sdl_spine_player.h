#ifndef SDL_SPINE_PLAYER_H_
#define SDL_SPINE_PLAYER_H_

#include "spine_player.h"

class CSdlSpinePlayer : public CSpinePlayer
{
public:
	CSdlSpinePlayer(SDL_Window* pSdlWindow, SDL_Renderer* pSdlRenderer);
	virtual ~CSdlSpinePlayer();

	void Redraw();

	SDL_FRect GetCurrentBoundingOfSlot(const char* slotName, size_t nameLength) const;
private:
	void WorkOutDefaultScale() override;
	void WorkOutDefaultOffset() override;

	SDL_Window *m_pSdlWindow = nullptr;
	SDL_Renderer* m_pSdlRenderer = nullptr;
};
#endif // !SDL_SPINE_PLAYER_H_
