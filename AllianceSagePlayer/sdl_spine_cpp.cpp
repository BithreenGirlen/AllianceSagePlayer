

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#include "sdl_spine_cpp.h"

#ifdef _WIN32
#include "win_text.h"
#endif

spine::SpineExtension* spine::getDefaultExtension()
{
	return new DefaultSpineExtension();
}

CSdlSpineDrawer::CSdlSpineDrawer(spine::SkeletonData *pSkeletonData, spine::AnimationStateData *pAnimationStateData)
{
	if (pSkeletonData == nullptr)return;

	spine::Bone::setYDown(true);
	m_sdlVertices.ensureCapacity(pSkeletonData->getBones().size() * sizeof(SDL_Vertex) / sizeof(float));

	skeleton = new(__FILE__, __LINE__) spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new(__FILE__, __LINE__) spine::AnimationStateData(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}
	animationState = new(__FILE__, __LINE__) spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);

	m_SdlBlendModeScreen = ::SDL_ComposeCustomBlendMode
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);

	m_SdlBlendModeNormalPma = ::SDL_ComposeCustomBlendMode
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);

	m_SdlBlendModeAdditivePma = ::SDL_ComposeCustomBlendMode
	(
		SDL_BlendFactor::SDL_BLENDFACTOR_SRC_ALPHA,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendFactor::SDL_BLENDFACTOR_ONE,
		SDL_BlendOperation::SDL_BLENDOPERATION_ADD
	);
}

CSdlSpineDrawer::~CSdlSpineDrawer()
{
	if (animationState != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
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

void CSdlSpineDrawer::Update(float fDelta)
{
	if (skeleton != nullptr && animationState != nullptr)
	{
		skeleton->update(fDelta);
		animationState->update(fDelta * timeScale);
		animationState->apply(*skeleton);
		skeleton->updateWorldTransform();
	}
}

void CSdlSpineDrawer::Draw(SDL_Renderer *pSdlRenderer)
{
	if (pSdlRenderer == nullptr || skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->getColor().a == 0) return;

	for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
	{
		spine::Slot &slot = *skeleton->getDrawOrder()[i];
		spine::Attachment *pAttachment = slot.getAttachment();
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

		if (IsToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float> *pVertices = &m_worldVertices;
		int verticesCount = 0;
		spine::Vector<float> *pAttachmentUvs = nullptr;

		spine::Vector<unsigned short> *pIndices = nullptr;
		int indicesCount = 0;

		spine::Color *pAttachmentColor = nullptr;

		SDL_Texture* pSdlTexture = nullptr;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment *pRegionAttachment = (spine::RegionAttachment *) pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			pSdlTexture = (SDL_Texture*)((spine::AtlasRegion*)pRegionAttachment->getRendererObject())->page->getRendererObject();

			m_worldVertices.setSize(8, 0);
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
			verticesCount = 4;
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;
			indicesCount = 6;
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment *pMeshAttachment = (spine::MeshAttachment *) pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			pSdlTexture = (SDL_Texture*)((spine::AtlasRegion*)pMeshAttachment->getRendererObject())->page->getRendererObject();

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices.buffer(), 0, 2);
			verticesCount = static_cast<int>(pMeshAttachment->getWorldVerticesLength() / 2);
			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();
			indicesCount = static_cast<int>(pIndices->size());

		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment *clip = (spine::ClippingAttachment *) slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else
		{
			continue;
		}

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			pVertices = &m_clipper.getClippedVertices();
			verticesCount = static_cast<int>(m_clipper.getClippedVertices().size() / 2);
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
			indicesCount = static_cast<int>(m_clipper.getClippedTriangles().size());
		}

		spine::Color tint
		{
			skeleton->getColor().r * slot.getColor().r * pAttachmentColor->r,
			skeleton->getColor().g * slot.getColor().g * pAttachmentColor->g,
			skeleton->getColor().b * slot.getColor().b * pAttachmentColor->b,
			skeleton->getColor().a * slot.getColor().a * pAttachmentColor->a,
		};

		m_sdlVertices.clear();
		for (int ii = 0; ii < verticesCount * 2; ii += 2)
		{
			SDL_Vertex sdlVertex{};
			sdlVertex.color.r = static_cast<Uint8>(tint.r * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sdlVertex.color.g = static_cast<Uint8>(tint.g * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sdlVertex.color.b = static_cast<Uint8>(tint.b * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sdlVertex.color.a = static_cast<Uint8>(tint.a * 255.f);
			sdlVertex.position.x = (*pVertices)[ii];
			sdlVertex.position.y = (*pVertices)[ii + 1LL];
			sdlVertex.tex_coord.x = (*pAttachmentUvs)[ii];
			sdlVertex.tex_coord.y = (*pAttachmentUvs)[ii + 1LL];
			m_sdlVertices.add(sdlVertex);
		}

		m_sdlIndices.clear();
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			m_sdlIndices.add((*pIndices)[ii]);
		}

		if (m_bForceBlendModeNormal)
		{
			::SDL_SetTextureBlendMode(pSdlTexture, m_bAlphaPremultiplied ? m_SdlBlendModeNormalPma : SDL_BLENDMODE_BLEND);
		}
		else
		{
			switch (slot.getData().getBlendMode())
			{
			case spine::BlendMode_Additive:
				::SDL_SetTextureBlendMode(pSdlTexture, m_bAlphaPremultiplied ? m_SdlBlendModeAdditivePma : SDL_BLENDMODE_ADD);
				break;
			case spine::BlendMode_Multiply:
				::SDL_SetTextureBlendMode(pSdlTexture, SDL_BLENDMODE_MUL);
				break;
			case spine::BlendMode_Screen:
				::SDL_SetTextureBlendMode(pSdlTexture, m_SdlBlendModeScreen);
				break;
			default:
				::SDL_SetTextureBlendMode(pSdlTexture, m_bAlphaPremultiplied ? m_SdlBlendModeNormalPma : SDL_BLENDMODE_BLEND);
				break;
			}
		}

		::SDL_RenderGeometry
		(
			pSdlRenderer,
			pSdlTexture,
			m_sdlVertices.buffer(),
			static_cast<int>(m_sdlVertices.size()),
			m_sdlIndices.buffer(),
			indicesCount
		);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSdlSpineDrawer::SetLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clear();
	for (size_t i = 0; i < list.size(); ++i)
	{
		m_leaveOutList.add(list[i].buffer());
	}
}

bool CSdlSpineDrawer::IsToBeLeftOut(const spine::String& slotName)
{
	for (size_t i = 0; i < m_leaveOutList.size(); ++i)
	{
		if (strcmp(slotName.buffer(), m_leaveOutList[i].buffer()) == 0)return true;
	}
	return false;
}

void CSdlTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& path)
{
	if (m_pSdlRenderer == nullptr)return;

	/*
	* Spine's DefaultSpineExtension::_readFile() calls fopen() without encoding flag that
	* the string should be ANSI code-page, but SDL assumes UTF-8 for file path.
	* Here Win32API is used because ::SDL_iconv() seems not to support SHIFT-JIS.
	* It might be better to define SpineExtension in which _readFile() calls fopen with ccs=UTF-8.
	*/
#ifdef _WIN32
	std::wstring wstr = win_text::WidenANSI(path.buffer());
	std::string str = win_text::NarrowUtf8(wstr);
	SDL_Texture* pSdlTexture = ::IMG_LoadTexture(m_pSdlRenderer, str.c_str());
#else
	SDL_Texture* pSdlTexture = ::IMG_LoadTexture(m_pSdlRenderer, path.buffer());
#endif
	if (pSdlTexture == nullptr)
	{
		m_SdlErrorMassage = ::SDL_GetError();
		return;
	}

	switch (atlasPage.magFilter)
	{
	case spine::TextureFilter_Nearest:
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_ScaleModeNearest);
		break;
	case spine::TextureFilter_Linear:
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_ScaleModeLinear);
		break;
	default:
		::SDL_SetTextureScaleMode(pSdlTexture, SDL_ScaleModeBest);
	}

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
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

	atlasPage.setRendererObject(pSdlTexture);
}

void CSdlTextureLoader::unload(void* texture)
{
	::SDL_DestroyTexture(static_cast<SDL_Texture*>(texture));
}
