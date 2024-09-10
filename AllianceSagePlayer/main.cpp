
#include <winsdkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

/*SDL2*/
#ifdef _WIN64
#pragma comment(lib, "x64/SDL2.lib")
#pragma comment(lib, "x64/SDL2_image.lib")
#pragma comment(lib, "x64/SDL2_ttf.lib")
#else
#pragma comment(lib, "x86/SDL2.lib")
#pragma comment(lib, "x86/SDL2_image.lib")
#pragma comment(lib, "x86/SDL2_ttf.lib")
#endif

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "alliance_sage.h"
#include "sdl_main_window.h"

struct SSdlInit
{
	SSdlInit()
	{
		iInitialised = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		iInitialised |= ::TTF_Init();
		/*IMG_Init() is not necessary unless optional format be used.*/
	}
	~SSdlInit()
	{
		::TTF_Quit();
		::SDL_Quit();
	}
	int iInitialised = -1;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
	SSdlInit sSdlInit;
	if (sSdlInit.iInitialised < 0)return 0;

	std::wstring wstrPickedFile = win_dialogue::SelectOpenFile(L"Text files", L"*_3.txt;*_4.txt;7*.txt", nullptr, nullptr);
	if (wstrPickedFile.empty())return 0;

	std::vector<std::wstring> scriptFilePaths;
	size_t nFileIndex = 0;
	bool bRet = win_filesystem::GetFilePathListAndIndex(wstrPickedFile.c_str(), L"*_3.txt;*_4.txt;7*.txt", scriptFilePaths, &nFileIndex);
	if (!bRet)return 0;

	/*7068.txt, 7069.txtは健全。*/
	scriptFilePaths.erase(std::remove_if(scriptFilePaths.begin(), scriptFilePaths.end(),
		[](const std::wstring& wstr)
		{
			return wcsstr(wstr.c_str(), L"memory\\7068.txt") != nullptr || wcsstr(wstr.c_str(), L"memory\\7069.txt") != nullptr;
		}), scriptFilePaths.end());

	for (;;)
	{
		std::vector<adv::TextDatum> textData;
		std::vector<std::string> strSpineNames;
		bRet = alliance_sage::LoadScenario(scriptFilePaths.at(nFileIndex), textData, strSpineNames);
		if (!bRet) break;

		std::vector<std::string> atlasPaths;
		std::vector<std::string> skelPaths;

		for (const auto& strSpineName : strSpineNames)
		{
			atlasPaths.emplace_back(strSpineName + ".atlas.txt");
			skelPaths.emplace_back(strSpineName + ".txt");
		}

		CSdlMainWindow sSdlMainWindow("SDL spine player", CSdlMainWindow::EBackEnd::kVulkan);

		bRet = sSdlMainWindow.SetSpineFromFile(atlasPaths, skelPaths, false);
		if (!bRet)break;

		sSdlMainWindow.SetFont("C:\\Windows\\Fonts\\yumin.ttf", true, true);
		sSdlMainWindow.SetTexts(textData);

		int iRet = sSdlMainWindow.Display();
		if (iRet == 1)
		{
			++nFileIndex;
			if (nFileIndex > scriptFilePaths.size() - 1)nFileIndex = 0;
		}
		else if (iRet == 2)
		{
			--nFileIndex;
			if (nFileIndex > scriptFilePaths.size() - 1)nFileIndex = scriptFilePaths.size() - 1;
		}
		else break;
	}

    return 0;
}