

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

	static std::wstring DeriveBasePathFromScriptFilePath(const std::wstring& wstrFilePath)
	{
		size_t nPos = wstrFilePath.find(L"event_ja");
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrFilePath.substr(0, nPos);
	}

	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const std::basic_string<CharType>& strOld, const std::basic_string<CharType>& strNew)
	{
		if (strOld.empty() || strOld == strNew) return;

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nPos += strNew.size();
		}
	}
	template <typename CharType, size_t sizeOld, size_t sizeNew>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType(&strOld)[sizeOld], const CharType(&strNew)[sizeNew])
	{
		constexpr size_t lenOld = sizeOld - 1;
		constexpr size_t lenNew = sizeNew - 1;

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos, lenOld);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, lenOld, strNew, lenNew);
			nPos += lenNew;
		}
	}
#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || (defined(__cplusplus) && __cplusplus >= 202002L)
	template <size_t sizeOld, size_t sizeNew>
	void ReplaceAll(std::string& src, const char8_t(&strOld)[sizeOld], const char8_t(&strNew)[sizeNew])
	{
		constexpr size_t lenOld = sizeOld - 1;
		constexpr size_t lenNew = sizeNew - 1;

		const char* pOld = reinterpret_cast<const char*>(strOld);
		const char* pNew = reinterpret_cast<const char*>(strNew);

		for (size_t nPos = 0;;)
		{
			nPos = src.find(pOld, nPos, lenOld);
			if (nPos == std::string::npos) break;

			src.replace(nPos, lenOld, pNew, lenNew);
			nPos += lenNew;
		}
	}
#endif
}

bool alliance_sage::LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& spineFilePaths, std::vector<std::string>& animationNames)
{
	std::wstring wstrBaseFolderPath = DeriveBasePathFromScriptFilePath(wstrFilePath);
	if (wstrBaseFolderPath.empty())return false;

	const std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	const char* p = &strFile[0];
	const char* pStart = nullptr, * pEnd = nullptr;

	std::vector<std::string> jsonObjects;
	for (;;)
	{
		bool bRet = json_minimal::FindNextObject(&p, nullptr, &pStart, &pEnd);
		if (!bRet)break;
		jsonObjects.emplace_back(pStart, pEnd);
	}

	std::vector<std::vector<SScriptCommand>> scriptCommandsList;
	for (const auto& jsonObject : jsonObjects)
	{
		p = &jsonObject[0];
		std::vector<SScriptCommand> scriptCommands;
		for (;;)
		{
			const char* pKeyStart = nullptr, * pKeyEnd = nullptr;
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bool bRet = json_minimal::util::ReadNextKeyInObject(&p, &pKeyStart, &pKeyEnd, &pValueStart, &pValueEnd);
			if (!bRet)break;

			scriptCommands.emplace_back(SScriptCommand{ std::string(pKeyStart, pKeyEnd), std::string(pValueStart, pValueEnd) });
		}

		scriptCommandsList.push_back(std::move(scriptCommands));
	}


	for (const auto& scriptCommands : scriptCommandsList)
	{
		std::string nameBuffer;
		std::string msgBuffer;
		std::string cvBuffer;

		std::string commandBuffer;

		for (const auto& scriptCommand : scriptCommands)
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
					spineFilePaths.emplace_back(win_text::NarrowUtf8(wstrBaseFolderPath).append(scriptCommand.strValue));
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
				t.strText = nameBuffer;
				t.strText += ": \n";
			}

			t.strText += msgBuffer;
			ReplaceAll(t.strText, u8"<name>", u8"俺");
			ReplaceAll(t.strText, "\\r", "");
			ReplaceAll(t.strText, "\\n", "\n");
			if (!cvBuffer.empty())
			{
				t.wstrVoicePath = std::wstring(wstrBaseFolderPath).append(L"cv\\").append(win_text::WidenUtf8(cvBuffer)).append(L".m4a");
			}

			t.nAnimationIndex = animationNames.empty() ? 0 : animationNames.size() - 1;

			textData.push_back(std::move(t));
		}
	}

	return textData.size() > 0 && spineFilePaths.size() > 0;
}
