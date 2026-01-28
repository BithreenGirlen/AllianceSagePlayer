
#include <winsdkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

/* SDL3 */
#ifdef _WIN64
#pragma comment(lib, "x64/SDL3.lib")
#pragma comment(lib, "x64/SDL3_image.lib")
#pragma comment(lib, "x64/SDL3_ttf.lib")
#else
#pragma comment(lib, "x86/SDL3.lib")
#pragma comment(lib, "x86/SDL3_image.lib")
#pragma comment(lib, "x86/SDL3_ttf.lib")
#endif /* _WIN64 */

/* Spine C++ runtime */
#ifdef _DEBUG
#pragma comment(lib, "spine-cpp-d.lib")
#else
#pragma comment(lib, "spine-cpp.lib")
#endif /* _DEBUG*/

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "alliance_sage.h"
#include "sdl_main_window.h"

struct SSdlInit
{
	SSdlInit()
	{
		isInitialised = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		isInitialised |= ::TTF_Init();
		/*IMG_Init() is not necessary unless optional format be used.*/
	}
	~SSdlInit()
	{
		::TTF_Quit();
		::SDL_Quit();
	}
	bool isInitialised = false;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	SSdlInit sdlInit;
	if (!sdlInit.isInitialised)return 0;

	constexpr const wchar_t fileFilters[] = L"*_3.json;*_4.json;7*.json";
	std::wstring wstrPickedFile = win_dialogue::SelectOpenFile(L"Scenario files", fileFilters, nullptr, nullptr);
	if (wstrPickedFile.empty())return 0;

	std::vector<std::wstring> scriptFilePaths;
	size_t nFileIndex = 0;
	bool bRet = win_filesystem::GetFilePathListAndIndex(wstrPickedFile.c_str(), fileFilters, scriptFilePaths, &nFileIndex);
	if (!bRet)return 0;

	/*7068.txt, 7069.txtは健全。*/
	scriptFilePaths.erase(std::remove_if(scriptFilePaths.begin(), scriptFilePaths.end(),
		[](const std::wstring& wstr)
		{
			return wcsstr(wstr.c_str(), L"memory\\7068.json") != nullptr || wcsstr(wstr.c_str(), L"memory\\7069.json") != nullptr;
		}), scriptFilePaths.end());

	CSdlMainWindow mainWindow("SDL spine player", CSdlMainWindow::EBackend::kDirectX);
	mainWindow.setFont("C:\\Windows\\Fonts\\yumin.ttf", true, true);

	for (;;)
	{
		std::vector<adv::TextDatum> textData;
		std::vector<std::string> spineFilePaths;
		std::vector<std::string> animationNames;
		bRet = alliance_sage::LoadScenario(scriptFilePaths[nFileIndex], textData, spineFilePaths, animationNames);
		if (!bRet)
		{
			::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load scenario", nullptr);
			break;
		}

		std::vector<std::string> atlasPaths;
		std::vector<std::string> skelPaths;

		for (const auto& spineFilePath : spineFilePaths)
		{
			atlasPaths.emplace_back(spineFilePath + ".atlas");
			skelPaths.emplace_back(spineFilePath + ".json");
		}

		bRet = mainWindow.setSpineFromFile(atlasPaths, skelPaths);
		if (!bRet)
		{
			std::string strError = "Failed to load Spine file";
			strError.append("\natlas:").append(atlasPaths[0]).append("\nskel: ").append(skelPaths[0]);

			::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", strError.c_str(), nullptr);
			break;
		}

		mainWindow.setScenarioData(textData, animationNames);

		int iRet = mainWindow.display();
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