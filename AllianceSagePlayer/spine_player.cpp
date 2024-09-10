
#include "spine_player.h"
#include "spine_loader.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{

}

/*ファイル取り込み*/
bool CSpinePlayer::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefualtSize();
	WorkOutDefaultScale();

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CSpinePlayer::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData.at(i);
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonData = skelData.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefualtSize();
	WorkOutDefaultScale();

	return SetupDrawer();
}
/*拡縮変更*/
void CSpinePlayer::RescaleSkeleton(bool bUpscale)
{
	constexpr float kfScalePortion = 0.025f;
	constexpr float kfMinScale = 0.15f;
	if (bUpscale)
	{
		m_fSkeletonScale += kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
	UpdateScaletonScale();
	ResizeWindow();
}
/*時間尺度変更*/
void CSpinePlayer::RescaleTime(bool bHasten)
{
	constexpr float kfTimeScalePortion = 0.05f;
	if (bHasten)
	{
		m_fTimeScale += kfTimeScalePortion;
	}
	else
	{
		m_fTimeScale -= kfTimeScalePortion;
	}
	if (m_fTimeScale < 0.f)m_fTimeScale = 0.f;

	UpdateTimeScale();
}
/*速度・尺度・視点初期化*/
void CSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultWindowScale;
	m_fOffset = m_fDefaultOffset;

	UpdateScaletonScale();
	UpdateTimeScale();
	MoveViewPoint(0, 0);
	ResizeWindow();
}
/*視点移動*/
void CSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.x += iX;
	m_fOffset.y += iY;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}
/*動作移行*/
void CSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex > m_animationNames.size() - 1)m_nAnimationIndex = 0;

	ClearAnimationTracks();

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(m_nAnimationIndex).c_str());
		if (animation != nullptr)
		{
			m_drawables.at(i).get()->animationState->setAnimation(0, animation->getName(), true);
		}
	}
}
/*装い移行*/
void CSpinePlayer::ShiftSkin()
{
	++m_nSkinIndex;
	if (m_nSkinIndex > m_skinNames.size() - 1)m_nSkinIndex = 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin* skin = m_skeletonData.at(i).get()->findSkin(m_skinNames.at(m_nSkinIndex).c_str());
		if (skin != nullptr)
		{
			m_drawables.at(i).get()->skeleton->setSkin(skin);
		}
		m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
	}
}
/*再描画*/
void CSpinePlayer::Redraw(float fDelta)
{
	if (!m_drawables.empty())
	{
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables.at(i).get()->Update(fDelta);
			m_drawables.at(i).get()->Draw();
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CSpinePlayer::SwitchPma()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->SwitchPma();
	}
}
/*槽溝指定合成方法採択可否*/
void CSpinePlayer::SwitchBlendModeAdoption()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->SwitchBlendModeAdoption();
	}
}
/*槽溝名称引き渡し*/
std::vector<std::string> CSpinePlayer::GetSlotList()
{
	std::vector<std::string> slotNames;
	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		auto& slots = m_skeletonData.at(i).get()->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const std::string& strName = slots[ii]->getName().buffer();
			const auto iter = std::find(slotNames.begin(), slotNames.end(), strName);
			if (iter == slotNames.cend())slotNames.push_back(strName);
		}
	}
	return slotNames;
}
/*装い名称引き渡し*/
std::vector<std::string> CSpinePlayer::GetSkinList() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
std::vector<std::string> CSpinePlayer::GetAnimationList() const
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

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->SetLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CSpinePlayer::MixSkins(const std::vector<std::string>& skinNames)
{
	const auto& currentSkinName = m_skinNames.at(m_nSkinIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin* skinToSet = m_skeletonData.at(i).get()->findSkin(currentSkinName.c_str());
		if (skinToSet != nullptr)
		{
			for (const auto& skinName : skinNames)
			{
				if (currentSkinName != skinName)
				{
					spine::Skin* skinToAdd = m_skeletonData.at(i).get()->findSkin(skinName.c_str());
					if (skinToAdd != nullptr)
					{
						skinToSet->addSkin(skinToAdd);
					}
				}
			}
			m_drawables.at(i).get()->skeleton->setSkin(skinToSet);
			m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
		}
	}
}
/*動作合成*/
void CSpinePlayer::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	const auto& currentAnimationName = m_animationNames.at(m_nAnimationIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		if (m_skeletonData.at(i).get()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					m_drawables.at(i).get()->animationState->addAnimation(iTrack, animation, false, 0.f);
					++iTrack;
				}
			}
		}
	}
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
	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<CSpineDrawable>(m_skeletonData.at(i).get()));

		CSpineDrawable* drawable = m_drawables.at(i).get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
		drawable->skeleton->updateWorldTransform();

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			const std::string& strAnimationName = animations[ii]->getName().buffer();
			const auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		auto& skins = m_skeletonData.at(i).get()->getSkins();
		for (size_t ii = 0; ii < skins.size(); ++ii)
		{
			const std::string& strName = skins[ii]->getName().buffer();
			const auto iter = std::find(m_skinNames.begin(), m_skinNames.end(), strName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(strName);
		}
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(0).c_str());
			if (animation != nullptr)
			{
				m_drawables.at(i).get()->animationState->setAnimation(0, animation->getName(), true);
			}
		}
	}

	ResetScale();

	return m_animationNames.size() > 0;
}
/*基準寸法・位置算出*/
void CSpinePlayer::WorkOutDefualtSize()
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
		spine::Attachment* pAttachment = pSkeletonData.get()->getDefaultSkin()->getAttachments().next()._attachment;
		if (pAttachment == nullptr)continue;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

			bool bRet = CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
			if (bRet)
			{
				m_fDefaultOffset.x = pRegionAttachment->getX() * 2.f;
				m_fDefaultOffset.y = -pRegionAttachment->getY() * 2.f;

				spine::SlotData *pSlotData = pSkeletonData->findSlot(pAttachment->getName());
				if (pSlotData == nullptr)continue;

				auto& bones = pSkeletonData->getBones();
				for (int i = 0; i < bones.size(); ++i)
				{
					if(pSlotData->getBoneData().getName() == bones[i]->getName())
					{
						if (pRegionAttachment->getY() * bones[i]->getY() < 0 && bones[i]->getX() == 0)
						{
							m_fDefaultOffset.y -= bones[i]->getY() * 2.f;
						}
						break;
					}
				}
			}
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

			spine::SlotData* pSlotData = pSkeletonData.get()->findSlot(pAttachment->getName());

			float fScaleX = pSlotData != nullptr ? pSlotData->getBoneData().getScaleX() : 1.f;
			float fScaleY = pSlotData != nullptr ? pSlotData->getBoneData().getScaleY() : 1.f;

			bool bRet = CompareDimention(pMeshAttachment->getWidth() * fScaleX, pMeshAttachment->getHeight() * fScaleY);
			if (bRet)
			{
				const auto WorkoutCentroid = [&pMeshAttachment]()
					-> Fpoint2
					{
						Fpoint2 fCentroid{};
						float fFilled = 0.f;

						const int iSize = pMeshAttachment->getHullLength();
						auto& vertices = pMeshAttachment->getVertices();

						if (vertices.size() < iSize)return Fpoint2{};

						Fpoint2 fFore
						{
							vertices[2 * (iSize - 1)],
							vertices[2 * (iSize - 1) + 1]
						};
						for (int i = 0; i < iSize; ++i)
						{
							Fpoint2 fNext
							{
								vertices[2 * i],
								vertices[2 * i + 1]
							};
							float fArea = fFore.x * fNext.y - fFore.y * fNext.x;
							fCentroid.x += (fFore.x + fNext.x) * fArea;
							fCentroid.y += (fFore.y + fNext.y) * fArea;
							fFilled += fArea;
							fFore = fNext;
						}
						fCentroid.x /= (6.f * fFilled * 0.5f);
						fCentroid.y /= (6.f * fFilled * 0.5f);

						return fCentroid;
					};

				Fpoint2 fCentroid = WorkoutCentroid();

				m_fDefaultOffset.x = fCentroid.x * 2.f;
				m_fDefaultOffset.y = -fCentroid.y * 2.f;
			}
		}
	}

	for (const auto& pSkeletonData : m_skeletonData)
	{
		CompareDimention(pSkeletonData.get()->getWidth(), pSkeletonData.get()->getHeight());
	}
}
/*尺度適用*/
void CSpinePlayer::UpdateScaletonScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setScaleX(m_fSkeletonScale);
		m_drawables.at(i).get()->skeleton->setScaleY(m_fSkeletonScale);
	}
}
/*速度適用*/
void CSpinePlayer::UpdateTimeScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->timeScale = m_fTimeScale;
	}
}
/*合成動作消去*/
void CSpinePlayer::ClearAnimationTracks()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		const auto& trackEntry = m_drawables.at(i).get()->animationState->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			m_drawables.at(i).get()->animationState->setEmptyAnimation(iTrack, 0.f);
		}
	}
}