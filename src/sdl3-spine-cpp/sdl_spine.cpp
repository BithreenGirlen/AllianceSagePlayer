

/* To calculate bounding box */
#include <float.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "sdl_spine.h"

spine::SpineExtension* spine::getDefaultExtension()
{
	return new DefaultSpineExtension();
}

/* Taken from SDL_render.c */
#define SDL_COMPOSE_BLENDMODE(srcColorFactor, dstColorFactor, colorOperation, \
							  srcAlphaFactor, dstAlphaFactor, alphaOperation) \
	(SDL_BlendMode)(((Uint32)(colorOperation) << 0) |                         \
					((Uint32)(srcColorFactor) << 4) |                         \
					((Uint32)(dstColorFactor) << 8) |                         \
					((Uint32)(alphaOperation) << 16) |                        \
					((Uint32)(srcAlphaFactor) << 20) |                        \
					((Uint32)(dstAlphaFactor) << 24))

struct SdlSpineBlendMode
{
	/* SDL2 does not have SDL_BLENDMODE_BLEND_PREMULTIPLIED */
	static constexpr SDL_BlendMode NormalPma = SDL_COMPOSE_BLENDMODE
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
	/* To be used instead of pre-defined SDL_BLENDMODE_ADD which discards source alpha. */
	static constexpr SDL_BlendMode Add = SDL_COMPOSE_BLENDMODE
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_SRC_ALPHA,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
	static constexpr SDL_BlendMode AddPma = SDL_COMPOSE_BLENDMODE
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
	/* To be used instead of pre-defined SDL_BLENDMODE_MUL which discards source alpha. */
	static constexpr SDL_BlendMode Multiply = SDL_COMPOSE_BLENDMODE
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_DST_COLOR,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
	static constexpr SDL_BlendMode Screen = SDL_COMPOSE_BLENDMODE
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
};

CSdlSpineDrawable::CSdlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	if (pSkeletonData == nullptr)return;

	spine::Bone::setYDown(true);
	m_sdlVertices.ensureCapacity(128);

	skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}
	animationState = new spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CSdlSpineDrawable::~CSdlSpineDrawable()
{
	if (animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			delete animationState->getData();
		}

		delete animationState;
	}
	if (skeleton != nullptr)
	{
		delete skeleton;
	}
}

void CSdlSpineDrawable::PremultiplyAlpha(bool toPremultiply)
{
	m_isAlphaPremultiplied = toPremultiply;
}

bool CSdlSpineDrawable::IsAlphaPremultiplied() const
{
	return m_isAlphaPremultiplied;
}

void CSdlSpineDrawable::ForceBlendModeNormal(bool toForce)
{
	m_toForceBlendModeNormal = toForce;
}

bool CSdlSpineDrawable::IsBlendModeNormalForced() const
{
	return m_toForceBlendModeNormal;
}

void CSdlSpineDrawable::Update(float fDelta)
{
	if (skeleton != nullptr && animationState != nullptr)
	{
#ifndef SPINE_4_1_OR_LATER
		skeleton->update(fDelta);
#endif
		animationState->update(fDelta);
		animationState->apply(*skeleton);
#ifdef SPINE_4_2_OR_LATER
		skeleton->update(fDelta);
		skeleton->updateWorldTransform(spine::Physics::Physics_Update);
#else
		skeleton->updateWorldTransform();
#endif
	}
}

void CSdlSpineDrawable::Draw(float fOffsetX, float fOffsetY)
{
	if (skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->getColor().a == 0) return;

	for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();
		if (!pAttachment)
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (IsSlotToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		spine::Vector<float>* pAttachmentUvs = nullptr;
		spine::Vector<unsigned short>* pIndices = nullptr;

		spine::Color* pAttachmentColor = nullptr;

		SDL_Texture* pSdlTexture = nullptr;
		SDL_Renderer* pSdlRenderer = nullptr;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
			pRegionAttachment->computeWorldVertices(slot, m_worldVertices, 0, 2);
#else
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
#endif
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSdlTexture = reinterpret_cast<SDL_Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSdlTexture = reinterpret_cast<SDL_Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);
			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSdlTexture = reinterpret_cast<SDL_Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSdlTexture = reinterpret_cast<SDL_Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (pSdlTexture == nullptr)continue;
		pSdlRenderer = ::SDL_GetRendererFromTexture(pSdlTexture);
		if (pSdlRenderer == nullptr)continue;

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			if (m_clipper.getClippedTriangles().size() == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}
			pVertices = &m_clipper.getClippedVertices();
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
		}

		const spine::Color tint
		{
			skeleton->getColor().r * slot.getColor().r * pAttachmentColor->r,
			skeleton->getColor().g * slot.getColor().g * pAttachmentColor->g,
			skeleton->getColor().b * slot.getColor().b * pAttachmentColor->b,
			skeleton->getColor().a * slot.getColor().a * pAttachmentColor->a,
		};

		m_sdlVertices.clear();
		for (int ii = 0; ii < pVertices->size(); ii += 2)
		{
			SDL_Vertex sdlVertex{};

			sdlVertex.position.x = (*pVertices)[ii] + fOffsetX;
			sdlVertex.position.y = (*pVertices)[ii + 1LL] + fOffsetY;

			sdlVertex.color.r = tint.r * (m_isAlphaPremultiplied ? tint.a : 1.f);
			sdlVertex.color.g = tint.g * (m_isAlphaPremultiplied ? tint.a : 1.f);
			sdlVertex.color.b = tint.b * (m_isAlphaPremultiplied ? tint.a : 1.f);
			sdlVertex.color.a = tint.a;

			sdlVertex.tex_coord.x = (*pAttachmentUvs)[ii];
			sdlVertex.tex_coord.y = (*pAttachmentUvs)[ii + 1LL];
			m_sdlVertices.add(sdlVertex);
		}

		m_sdlIndices.setSize(pIndices->size(), 0);
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			m_sdlIndices[ii] = static_cast<int>((*pIndices)[ii]);
		}

		spine::BlendMode spineBlendMode = m_toForceBlendModeNormal ? spine::BlendMode::BlendMode_Normal : slot.getData().getBlendMode();
		switch (slot.getData().getBlendMode())
		{
		case spine::BlendMode_Additive:
			::SDL_SetTextureBlendMode(pSdlTexture, m_isAlphaPremultiplied ? SdlSpineBlendMode::AddPma : SdlSpineBlendMode::Add);
			break;
		case spine::BlendMode_Multiply:
			::SDL_SetTextureBlendMode(pSdlTexture, SdlSpineBlendMode::Multiply);
			break;
		case spine::BlendMode_Screen:
			::SDL_SetTextureBlendMode(pSdlTexture, SdlSpineBlendMode::Screen);
			break;
		default:
			::SDL_SetTextureBlendMode(pSdlTexture, m_isAlphaPremultiplied ? SdlSpineBlendMode::NormalPma : SDL_BLENDMODE_BLEND);
			break;
		}

		::SDL_RenderGeometry
		(
			pSdlRenderer,
			pSdlTexture,
			m_sdlVertices.buffer(),
			static_cast<int>(m_sdlVertices.size()),
			m_sdlIndices.buffer(),
			static_cast<int>(m_sdlIndices.size())
		);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSdlSpineDrawable::SetLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clearAndAddAll(list);
}

SDL_FRect CSdlSpineDrawable::GetBoundingBox() const
{
	SDL_FRect boundingBox{};

	if (skeleton != nullptr)
	{
		spine::Vector<float> tempVertices;
		skeleton->getBounds(boundingBox.x, boundingBox.y, boundingBox.w, boundingBox.h, tempVertices);
	}

	return boundingBox;
}

SDL_FRect CSdlSpineDrawable::GetBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	if (skeleton != nullptr)
	{
		for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
		{
			spine::Slot& slot = *skeleton->getDrawOrder()[i];
			const spine::String& slotDataName = slot.getData().getName();
			if (nameLength != slotDataName.length())continue;

			if (::memcmp(slotDataName.buffer(), slotName, slotDataName.length()) == 0)
			{
				spine::Attachment* pAttachment = slot.getAttachment();
				if (pAttachment != nullptr)
				{
					spine::Vector<float> tempVertices;
					if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
					{
						spine::RegionAttachment* pRegionAttachment = static_cast<spine::RegionAttachment*>(pAttachment);

						tempVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
						pRegionAttachment->computeWorldVertices(slot, tempVertices, 0, 2);
#else
						pRegionAttachment->computeWorldVertices(slot.getBone(), tempVertices, 0, 2);
#endif
					}
					else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
					{
						spine::MeshAttachment* pMeshAttachment = static_cast<spine::MeshAttachment*>(pAttachment);
						tempVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
						pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), tempVertices, 0, 2);
					}
					else
					{
						continue;
					}

					for (size_t i = 0; i < tempVertices.size(); i += 2)
					{
						float fX = tempVertices[i];
						float fY = tempVertices[i + 1LL];

						fMinX = fMinX < fX ? fMinX : fX;
						fMinY = fMinY < fY ? fMinY : fY;
						fMaxX = fMaxX > fX ? fMaxX : fX;
						fMaxY = fMaxY > fY ? fMaxY : fY;
					}

					if (found != nullptr)*found = true;
					break;
				}
			}
		}
	}

	return SDL_FRect{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

bool CSdlSpineDrawable::IsSlotToBeLeftOut(const spine::String& slotName)
{
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		return m_leaveOutList.contains(slotName);
	}

	return false;
}


void CSdlTextureLoader::SetRenderer(SDL_Renderer* pSdlRenderer)
{
	m_pSdlRenderer = pSdlRenderer;
}

void CSdlTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& path)
{
	if (m_pSdlRenderer == nullptr)return;

	SDL_Texture* pSdlTexture = ::IMG_LoadTexture(m_pSdlRenderer, path.buffer());
	if (pSdlTexture == nullptr)
	{
#ifdef _DEBUG
		m_SdlErrorMassage = ::SDL_GetError();
#endif
		return;
	}

	switch (atlasPage.magFilter)
	{
	case spine::TextureFilter_Nearest:
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_SCALEMODE_NEAREST);
		break;
	case spine::TextureFilter_Linear:
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_SCALEMODE_LINEAR);
		break;
	default:
		/* SDL_ScaleModeBest has been removed in SDL3 */
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_SCALEMODE_LINEAR);
		break;
	}

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
#if 0
	if (atlasPage.width == 0 || atlasPage.height == 0)
	{
		int iWidth = 0;
		int iHeight = 0;
		int iRet = ::SDL_QueryTexture(pSdlTexture, nullptr, nullptr, &iWidth, &iHeight);
		if (iRet == 0)
		{
			atlasPage.width = iWidth;
			atlasPage.height = iHeight;
		}
	}
#endif

#ifdef SPINE_4_1_OR_LATER
	atlasPage.texture = pSdlTexture;
#else
	atlasPage.setRendererObject(pSdlTexture);
#endif
}

void CSdlTextureLoader::unload(void* texture)
{
	::SDL_DestroyTexture(static_cast<SDL_Texture*>(texture));
}
