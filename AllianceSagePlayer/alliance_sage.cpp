
#include <memory>

#include "alliance_sage.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "json_minimal.h"

namespace alliance_sage
{
	struct SScriptCommand
	{
		std::string strName;
		std::string strValue;
	};

	static std::wstring DeriveBasePathFromScriptFilePath(const std::wstring wstrFilePath)
	{
		size_t nPos = wstrFilePath.find(L"event_ja");
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrFilePath.substr(0, nPos);
	}

	static void ReplaceAll(std::string& src, const std::string& strOld, const std::string& strNew)
	{
		if (strOld == strNew)return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::string::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}

	static void ReplaceAll(std::wstring& src, const std::wstring& strOld, const std::wstring& strNew)
	{
		if (strOld == strNew)return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::wstring::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}
}

bool alliance_sage::LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& spineFilePaths, std::vector<std::string>& animationNames)
{
	std::wstring wstrBaseFolderPath = DeriveBasePathFromScriptFilePath(wstrFilePath);
	if (wstrBaseFolderPath.empty())return false;

	std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	ReplaceAll(strFile, "\t", "");
	ReplaceAll(strFile, "\r", "");
	ReplaceAll(strFile, "\n", "");

	char* p = &strFile[0];

	std::vector<std::string> jsonObjects;
	for (;;)
	{
		auto pp = std::make_unique<char*>();
		bool bRet = json_minimal::ExtractJsonObject(&p, nullptr, &*pp);
		if (!bRet)break;
		jsonObjects.push_back(*pp);
	}

	std::vector<std::vector<SScriptCommand>> scriptCommandSets;
	{
		std::vector<char> nameBuffer(512);
		std::vector<char> valueBuffer(512);

		for (auto& jsonObject : jsonObjects)
		{
			std::vector<SScriptCommand> scriptCommandSetBuffer;
			p = &jsonObject[0];
			for (;;)
			{
				bool bRet = json_minimal::ReadNextKey(&p, nameBuffer.data(), nameBuffer.size(), valueBuffer.data(), valueBuffer.size());
				if (!bRet)break;
				SScriptCommand s;
				s.strName = nameBuffer.data();
				s.strValue = valueBuffer.data();

				scriptCommandSetBuffer.emplace_back(std::move(s));
			}
			scriptCommandSets.push_back(std::move(scriptCommandSetBuffer));
		}
	}

	for (const auto& scriptCommandSet : scriptCommandSets)
	{
		std::string nameBuffer;
		std::string msgBuffer;
		std::string cvBuffer;

		std::string commandBuffer;

		for (const auto& scriptCommand : scriptCommandSet)
		{
			if (scriptCommand.strName == "name")
			{
				nameBuffer = scriptCommand.strValue;
			}
			else if (scriptCommand.strName == "message")
			{
				msgBuffer = scriptCommand.strValue;
			}
			else if (scriptCommand.strName == "cv")
			{
				cvBuffer = scriptCommand.strValue;
			}
			else if (scriptCommand.strName == "command")
			{
				commandBuffer = scriptCommand.strValue;
			}
			else if (scriptCommand.strName == "param")
			{
				if (commandBuffer == "spine on")
				{
					spineFilePaths.push_back(win_text::NarrowUtf8(wstrBaseFolderPath) + scriptCommand.strValue);
				}
				else if (commandBuffer == "spine animation")
				{
					animationNames.push_back(scriptCommand.strValue);
				}
			}
		}

		if (!msgBuffer.empty())
		{
			adv::TextDatum t;
			if (!nameBuffer.empty())
			{
				t.wstrText = win_text::WidenUtf8(nameBuffer);
				t.wstrText += L": ";
			}

			t.wstrText = win_text::WidenUtf8(msgBuffer);
			ReplaceAll(t.wstrText, L"<name>", L"俺");
			ReplaceAll(t.wstrText, L"\\r", L"");
			ReplaceAll(t.wstrText, L"\\n", L"\n");
			if (!cvBuffer.empty())
			{
				t.wstrVoicePath = wstrBaseFolderPath + L"cv\\" + win_text::WidenUtf8(cvBuffer) + L".m4a";
			}

			t.nAnimationIndex = animationNames.empty() ? 0 : animationNames.size() - 1;

			textData.push_back(std::move(t));
		}
	}

	return textData.size() > 0 && spineFilePaths.size() > 0;
}
