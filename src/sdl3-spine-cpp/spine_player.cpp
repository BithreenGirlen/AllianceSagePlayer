
#include "spine_player.h"
#include "spine_loader.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{

}

/*ファイル取り込み*/
bool CSpinePlayer::LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CSpinePlayer::LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& texturePaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != texturePaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strTexturePath = texturePaths[i];
		const std::string& strSkeletonDatum = skelData[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strTexturePath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(strSkeletonDatum.data()), static_cast<int>(strSkeletonDatum.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonDatum.data(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawer();
}

size_t CSpinePlayer::GetNumberOfSpines() const
{
	return m_drawables.size();
}

bool CSpinePlayer::HasSpineBeenLoaded() const
{
	return !m_drawables.empty();
}
/*状態更新*/
void CSpinePlayer::Update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->Update(fDelta * m_fTimeScale);
	}
}
/*速度・尺度・視点初期化*/
void CSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;

	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;

	m_fOffset = m_fDefaultOffset;

	UpdatePosition();
}
/*視点移動*/
void CSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	UpdatePosition();
}

void CSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	RestartAnimation();
}

void CSpinePlayer::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	SetupSkin();
}

void CSpinePlayer::SetAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		RestartAnimation();
	}
}

void CSpinePlayer::SetAnimationByName(const char* animationName)
{
	if (animationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), animationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			RestartAnimation();
		}
	}
}
/*動作適用*/
void CSpinePlayer::RestartAnimation()
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* animationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton->getData()->findAnimation(animationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState->setAnimation(0, pAnimation->getName(), true);
		}
	}
}

void CSpinePlayer::SetSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		SetupSkin();
	}
}

void CSpinePlayer::SetSkinByName(const char* skinName)
{
	if (skinName != nullptr)
	{
		const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), skinName);
		if (iter != m_skinNames.cend())
		{
			m_nSkinIndex = std::distance(m_skinNames.begin(), iter);
			SetupSkin();
		}
	}
}

void CSpinePlayer::SetupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* skinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Skin* skin = pDrawable->skeleton->getData()->findSkin(skinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton->setSkin(skin);
			pDrawable->skeleton->setSlotsToSetupPose();
		}
	}
}

void CSpinePlayer::TogglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->PremultiplyAlpha(!pDrawable->IsAlphaPremultiplied());
	}
}

void CSpinePlayer::ToggleBlendMode()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->ForceBlendModeNormal(!pDrawable->IsBlendModeNormalForced());
	}
}

bool CSpinePlayer::PremultiplyAlpha(bool toBePremultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->PremultiplyAlpha(toBePremultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayer::IsAlphaPremultiplied(size_t nDrawableIndex) const
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->IsAlphaPremultiplied();
	}

	return false;
}

bool CSpinePlayer::ForceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->ForceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

bool CSpinePlayer::IsBlendModeNormalForced(size_t nDrawableIndex) const
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->IsBlendModeNormalForced();
	}

	return false;
}

void CSpinePlayer::SetDrawOrder(bool toBeReversed)
{
	m_isDrawOrderReversed = toBeReversed;
}

bool CSpinePlayer::IsDrawOrderReversed() const
{
	return m_isDrawOrderReversed;
}

const char* CSpinePlayer::GetCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
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

void CSpinePlayer::GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
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
/*槽溝名称引き渡し*/
std::vector<std::string> CSpinePlayer::GetSlotNames()
{
	std::vector<std::string> slotNames;
	for (const auto& skeletonDatum : m_skeletonData)
	{
		auto& slots = skeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto iter = std::find(slotNames.begin(), slotNames.end(), szName);
			if (iter == slotNames.cend())slotNames.push_back(szName);
		}
	}

	return slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::GetSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::GetAnimationNames() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayer::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CSpinePlayer::MixSkins(const std::vector<std::string>& skinNames)
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spine::Skin* skinToSet = pDrawble->skeleton->getData()->findSkin(currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spine::Skin* skinToAdd = pDrawble->skeleton->getData()->findSkin(skinName.c_str());
				if (skinToAdd != nullptr)
				{
					skinToSet->addSkin(skinToAdd);
				}
			}
		}
		pDrawble->skeleton->setSkin(skinToSet);
		pDrawble->skeleton->setSlotsToSetupPose();
	}
}
/*動作合成*/
void CSpinePlayer::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawable : m_drawables)
	{
		if (pDrawable->skeleton->getData()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState->addAnimation(iTrack, animation, false, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*描画除外是否関数登録*/
void CSpinePlayer::SetSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutCallback(pFunc);
	}
}
/*寸法受け渡し*/
FPoint2 CSpinePlayer::GetBaseSize() const
{
	return m_fBaseSize;
}

void CSpinePlayer::SetBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	WorkOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	ResetScale();
}

void CSpinePlayer::ResetBaseSize()
{
	WorkOutDefaultSize();
	WorkOutDefaultScale();

	m_fOffset = {};
	UpdatePosition();
	for (const auto& drawable : m_drawables)
	{
		drawable->animationState->setEmptyAnimations(0.f);
		drawable->Update(0.f);
	}

	WorkOutDefaultOffset();
	ResetScale();
	RestartAnimation();
}

FPoint2 CSpinePlayer::GetOffset() const
{
	return m_fOffset;
}

float CSpinePlayer::GetSkeletonScale() const
{
	return m_fSkeletonScale;
}

void CSpinePlayer::SetSkeletonScale(float fScale)
{
	m_fSkeletonScale = fScale;
}

float CSpinePlayer::GetCanvasScale() const
{
	return m_fCanvasScale;
}
void CSpinePlayer::SetCanvasScale(float fScale)
{
	m_fCanvasScale = fScale;
}

float CSpinePlayer::GetTimeScale() const
{
	return m_fTimeScale;
}

void CSpinePlayer::SetTimeScale(float fTimeScale)
{
	m_fTimeScale = fTimeScale;
}
/*消去*/
void CSpinePlayer::ClearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;
}
/*描画器設定*/
bool CSpinePlayer::SetupDrawer()
{
	WorkOutDefaultSize();
	WorkOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		auto pDrawable = std::make_shared<CSpineDrawable>(pSkeletonDatum.get());
		if (pDrawable.get() == nullptr)continue;

		pDrawable->skeleton->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
		pDrawable->Update(0.f);

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

	}

	WorkOutDefaultOffset();

	RestartAnimation();

	ResetScale();

	return m_animationNames.size() > 0;
}
/*基準寸法・位置算出*/
void CSpinePlayer::WorkOutDefaultSize()
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
void CSpinePlayer::UpdatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}
/*合成動作消去*/
void CSpinePlayer::ClearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState->setEmptyAnimation(iTrack, 0.f);
		}
	}
}