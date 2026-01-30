
#include "spine_player.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{

}

/*ファイル取り込み*/
bool CSpinePlayer::loadSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skeletonFilePaths)
{
	if (atlasFilePaths.size() != skeletonFilePaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasFilePaths.size(); ++i)
	{
		const std::string& atlasFilePath = atlasFilePaths[i];
		const std::string& skeletonFilePath = skeletonFilePaths[i];

		auto atlas = spine_loader::ReadAtlasFromFile(atlasFilePath.c_str(), &m_textureLoader);
		if (atlas == nullptr)continue;

		auto skeletonData = spine_loader::ReadSkeletonFromFile(skeletonFilePath.c_str(), atlas.get());
		if (skeletonData == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawer();
}
/*メモリ取り込み*/
bool CSpinePlayer::loadSpineFromMemory(const std::vector<std::string>& atlasFileData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skeletonFileData)
{
	if (atlasFileData.size() != skeletonFileData.size() || atlasFileData.size() != textureDirectories.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasFileData.size(); ++i)
	{
		const std::string& atlasFileDatum = atlasFileData[i];
		const std::string& textureDirectory = textureDirectories[i];
		const std::string& skeletonFileDatum = skeletonFileData[i];

		auto atlas = spine_loader::ReadAtlasFromMemory(atlasFileDatum.c_str(), atlasFileDatum.size(), textureDirectory.c_str(),  & m_textureLoader);
		if (atlas.get() == nullptr)continue;

		auto skeletonData = spine_loader::ReadSkeletonFromMemory(reinterpret_cast<const unsigned char*>(skeletonFileDatum.data()),skeletonFileDatum.size(), atlas.get());
		if (skeletonData == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawer();
}

size_t CSpinePlayer::getNumberOfSpines() const
{
	return m_drawables.size();
}

bool CSpinePlayer::hasSpineBeenLoaded() const
{
	return !m_drawables.empty();
}
/*状態更新*/
void CSpinePlayer::update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->update(fDelta * m_fTimeScale);
	}
}
/*速度・尺度・視点初期化*/
void CSpinePlayer::resetScale()
{
	m_fTimeScale = 1.0f;

	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;

	m_fOffset = m_fDefaultOffset;

	updatePosition();
}
/*視点移動*/
void CSpinePlayer::addOffset(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	updatePosition();
}

void CSpinePlayer::shiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	clearAnimationTracks();
	restartAnimation();
}

void CSpinePlayer::shiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	setupSkin();
}

void CSpinePlayer::setAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		restartAnimation();
	}
}

void CSpinePlayer::setAnimationByName(const char* animationName)
{
	if (animationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), animationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			restartAnimation();
		}
	}
}
/*動作適用*/
void CSpinePlayer::restartAnimation()
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* animationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState()->setAnimation(0, pAnimation->getName(), true);
		}
	}
}

void CSpinePlayer::setSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		setupSkin();
	}
}

void CSpinePlayer::setSkinByName(const char* skinName)
{
	if (skinName != nullptr)
	{
		const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), skinName);
		if (iter != m_skinNames.cend())
		{
			m_nSkinIndex = std::distance(m_skinNames.begin(), iter);
			setupSkin();
		}
	}
}

void CSpinePlayer::setupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* skinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Skin* skin = pDrawable->skeleton()->getData()->findSkin(skinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton()->setSkin(skin);
			pDrawable->skeleton()->setSlotsToSetupPose();
		}
	}
}

void CSpinePlayer::togglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->premultiplyAlpha(!pDrawable->isAlphaPremultiplied());
	}
}

void CSpinePlayer::toggleBlendMode()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->forceBlendModeNormal(!pDrawable->isBlendModeNormalForced());
	}
}

bool CSpinePlayer::premultiplyAlpha(bool toBePremultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->premultiplyAlpha(toBePremultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayer::isAlphaPremultiplied(size_t nDrawableIndex) const
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied();
	}

	return false;
}

bool CSpinePlayer::forceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->forceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

bool CSpinePlayer::isBlendModeNormalForced(size_t nDrawableIndex) const
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isBlendModeNormalForced();
	}

	return false;
}

void CSpinePlayer::setDrawOrder(bool toBeReversed)
{
	m_isDrawOrderReversed = toBeReversed;
}

bool CSpinePlayer::isDrawOrderReversed() const
{
	return m_isDrawOrderReversed;
}

const char* CSpinePlayer::getCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				return pAnimation->getName().buffer();
			}
		}
	}

	return nullptr;
}

void CSpinePlayer::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				if (fTrack != nullptr)*fTrack = tracks[i]->getTrackTime();
				if (fLast != nullptr)*fLast = tracks[i]->getAnimationLast();
				if (fStart != nullptr)*fStart = tracks[i]->getAnimationStart();
				if (fEnd != nullptr)*fEnd = tracks[i]->getAnimationEnd();

				return;
			}
		}
	}
}

float CSpinePlayer::getAnimationDuration(const char* animationName)
{
	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
		if (pAnimation != nullptr)
		{
			return pAnimation->getDuration();
		}
	}

	return 0.f;
}
/*槽溝名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::getSlotNames() const
{
	return m_slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::getSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::GetAnimationNames() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayer::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CSpinePlayer::mixSkins(const std::vector<std::string>& skinNames)
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spine::Skin* skinToSet = pDrawble->skeleton()->getData()->findSkin(currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spine::Skin* skinToAdd = pDrawble->skeleton()->getData()->findSkin(skinName.c_str());
				if (skinToAdd != nullptr)
				{
					skinToSet->addSkin(skinToAdd);
				}
			}
		}
		pDrawble->skeleton()->setSkin(skinToSet);
		pDrawble->skeleton()->setSlotsToSetupPose();
	}
}
/*動作予約*/
void CSpinePlayer::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	clearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawable : m_drawables)
	{
		if (pDrawable->skeleton()->getData()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton()->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState()->addAnimation(iTrack, animation, loop, 0.f);
					++iTrack;
				}
			}
		}
	}
}

void CSpinePlayer::mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime)
{
	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* fadeOutAnimation = pDrawable->skeleton()->getData()->findAnimation(fadeOutAnimationName);
		spine::Animation* fadeInAnimation = pDrawable->skeleton()->getData()->findAnimation(fadeInAnimationName);
		if (fadeOutAnimation != nullptr && fadeInAnimation != nullptr)
		{
			const auto& animationStateData = pDrawable->animationState()->getData();
			animationStateData->setMix(fadeOutAnimation, fadeInAnimation, mixTime);
		}
	}
}
void CSpinePlayer::clearMixedAnimation()
{
#ifdef SPINE_4_1_OR_LATER
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->animationState()->getData()->clear();
	}
#endif
}
/*描画除外是否関数登録*/
void CSpinePlayer::setSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutCallback(pFunc);
	}
}
/*寸法受け渡し*/
FPoint2 CSpinePlayer::getBaseSize() const
{
	return m_fBaseSize;
}

void CSpinePlayer::setBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	workOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	resetScale();
}

void CSpinePlayer::resetBaseSize()
{
	workOutDefaultSize();
	workOutDefaultScale();

	m_fOffset = {};
	updatePosition();
	for (const auto& drawable : m_drawables)
	{
		drawable->animationState()->setEmptyAnimations(0.f);
		drawable->update(0.f);
	}

	workOutDefaultOffset();
	resetScale();
	restartAnimation();
}

FPoint2 CSpinePlayer::getOffset() const
{
	return m_fOffset;
}

void CSpinePlayer::setOffset(float fWidth, float fHeight)
{
	m_fOffset.x = fWidth;
	m_fOffset.y = fHeight;
}

float CSpinePlayer::getSkeletonScale() const
{
	return m_fSkeletonScale;
}

void CSpinePlayer::setSkeletonScale(float fScale)
{
	m_fSkeletonScale = fScale;
}

float CSpinePlayer::getCanvasScale() const
{
	return m_fCanvasScale;
}
void CSpinePlayer::setCanvasScale(float fScale)
{
	m_fCanvasScale = fScale;
}

float CSpinePlayer::getTimeScale() const
{
	return m_fTimeScale;
}

void CSpinePlayer::setTimeScale(float fTimeScale)
{
	m_fTimeScale = fTimeScale;
}
/*消去*/
void CSpinePlayer::clearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;

	m_slotNames.clear();
}
/*描画器設定*/
bool CSpinePlayer::setupDrawer()
{
	workOutDefaultSize();
	workOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		auto pDrawable = std::make_unique<CSpineDrawable>(pSkeletonDatum.get());
		if (pDrawable.get() == nullptr)continue;

		pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
		pDrawable->update(0.f);

		m_drawables.push_back(std::move(pDrawable));

		auto& animations = pSkeletonDatum->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			const char* animationName = animations[i]->getName().buffer();
			if (animationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), animationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(animationName);
		}

		auto& skins = pSkeletonDatum->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			const char* szSkinName = skins[i]->getName().buffer();
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

		auto& slots = pSkeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto& iter = std::find(m_slotNames.begin(), m_slotNames.end(), szName);
			if (iter == m_slotNames.cend())m_slotNames.push_back(szName);
		}
	}

	workOutDefaultOffset();

	restartAnimation();

	resetScale();

	return m_animationNames.size() > 0;
}
/*基準寸法・位置算出*/
void CSpinePlayer::workOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> bool
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.x = fWidth;
				m_fBaseSize.y = fHeight;
				fMaxSize = fWidth * fHeight;
				return true;
			}

			return false;
		};

	for (const auto& pSkeletonData : m_skeletonData)
	{
		if (pSkeletonData.get()->getWidth() > 0 && pSkeletonData.get()->getHeight())
		{
			CompareDimention(pSkeletonData.get()->getWidth(), pSkeletonData.get()->getHeight());
		}
		else
		{
			/*Why spine::Skin lacks searching methods based on its own spine::Vector<spine::Attachment*>?*/
			const auto FindDefaultSkinAttachment = [&pSkeletonData]()
				-> spine::Attachment*
				{
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonData.get()->getDefaultSkin()->getAttachments();
					for (; attachmentMapEntries.hasNext();)
					{
						spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();
						if (attachmentMapEntry._slotIndex == 0)
						{
							return attachmentMapEntry._attachment;
						}
					}
					return nullptr;
				};

			spine::Attachment* pAttachment = FindDefaultSkinAttachment();
			if (pAttachment == nullptr)continue;

			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

				CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

				spine::SlotData* pSlotData = pSkeletonData.get()->findSlot(pAttachment->getName());

				float fScaleX = pSlotData != nullptr ? pSlotData->getBoneData().getScaleX() : 1.f;
				float fScaleY = pSlotData != nullptr ? pSlotData->getBoneData().getScaleY() : 1.f;

				CompareDimention(pMeshAttachment->getWidth() * fScaleX, pMeshAttachment->getHeight() * fScaleY);
			}
		}
	}
}
/*位置適用*/
void CSpinePlayer::updatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}
/*合成動作消去*/
void CSpinePlayer::clearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState()->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState()->setEmptyAnimation(iTrack, 0.f);
		}
	}
}