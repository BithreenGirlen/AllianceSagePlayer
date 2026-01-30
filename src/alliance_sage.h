#ifndef ALLIANCE_SAGE_H_
#define ALLIANCE_SAGE_H_

#include <string>
#include <vector>

#include "adv.h"

namespace alliance_sage
{
	bool LoadScenario(
		const std::wstring& wstrFilePath,
		std::vector<adv::TextDatum>& textData,
		std::vector<std::string>& spineFilePaths,
		std::vector<std::string>& animationNames);
}
#endif // !ALLIANCE_SAGE_H_

